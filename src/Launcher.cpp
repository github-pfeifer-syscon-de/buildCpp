/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2025 RPf
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <iostream>
#include <glibmm.h>
#include <chrono>
#include <vector>
#include <psc_format.hpp>
#include <unistd.h>     // getpid
#include <sys/wait.h>
#include <signal.h>
#include <Log.hpp>
#include <cstring>

#include "Launcher.hpp"

#include "../../../../../usr/include/glib-2.0/glib/gmain.h"

static void
child_watch_cb(GPid     pid,
               gint     status,
               gpointer user_data)
{
    // Free any resources associated with the child here, such as I/O channels
    // on its stdout and stderr FDs. If you have no code to put in the
    // child_watch_cb() callback, you can remove it and the g_child_watch_add()
    // call, but you must also remove the G_SPAWN_DO_NOT_REAP_CHILD flag,
    // otherwise the child process will stay around as a zombie until this
    // process exits.
    g_autoptr(GError) error{nullptr};
    auto stat = g_spawn_check_wait_status(status, &error);
    if (user_data) {
        int exit{0};
        if (error) {
            exit = error->code;
        }
        std::cout <<  psc::fmt::format("wait_stat {} exit {}", stat, exit) << std::endl;
        auto launcher = static_cast<Launcher*>(user_data);
        launcher->childExited(stat, exit);
    }
    else {
        std::cout << ("child_watch_cb no user_data") << std::endl;
    }
    g_spawn_close_pid(pid);
}


static gboolean
cb_out_watch(GIOChannel *channel, GIOCondition cond, gpointer user_data)
{
    LineReaderChannel*  reader{};
    if (user_data != nullptr) {
         reader = static_cast<LineReaderChannel*>(user_data);
    }
    else {
        std::cout << ("cb_out_watch output no user_data!") << std::endl;
        return FALSE;
    }
    if (cond == G_IO_HUP) {
        //std::cout << "cb_out_watch shut " << static_cast<void*>(channel) << std::endl;
        g_io_channel_shutdown(channel, FALSE, nullptr);
        reader->done();
        return FALSE;
    }
    reader->ready();
    return TRUE;
}


LineReaderChannel::LineReaderChannel(gint out, bool err, ReaderListener* readerListener)
: m_out{out}
, m_err{err}
, m_readerListener{readerListener}
{
    m_channel = g_io_channel_unix_new(m_out);   // channels offer some convenient way to read stuff
    g_autoptr(GError) error{nullptr};
    g_io_channel_set_encoding(m_channel, nullptr, &error);
    if (error) {
        psc::log::Log::logAdd(psc::log::Level::Error,
                      [&] {
                          return psc::fmt::format("error {} set enconding", error->message);
                      });
    }
    /* Add watches to channels (read/hup callback) */
    m_source = g_io_add_watch(m_channel, static_cast<GIOCondition>(G_IO_IN | G_IO_HUP), cb_out_watch, this);   // (GIOFunc)
}

LineReaderChannel::~LineReaderChannel()
{
    if (m_channel != nullptr) {
        g_io_channel_unref(m_channel);
    }
    close(m_out);
}

void
LineReaderChannel::done()
{
    m_done = true;
}

void
LineReaderChannel::ready()
{
    while (true) {
        g_autoptr(GError) error{};
        gchar *string{};
        gsize size{0};
        g_io_channel_read_line(m_channel, &string, &size, nullptr, &error);
        if (error) {
            std::cout <<  psc::fmt::format("error {} reading", error->message) << std::endl;
            break;
        }
        if (string != nullptr) {
            //std::cout << "LineReader::ready read " << static_cast<void*>(m_channel)
            //          << " str \"" << string << "\"" << std::endl;
            m_readerListener->notify(m_err, string);
            g_free(string);
        }
        auto cond = g_io_channel_get_buffer_condition(m_channel);
        //std::cout << "LineReader::ready read " << static_cast<void*>(m_channel)
        //          << " cond " << cond << std::endl;
        if ((cond & static_cast<GIOCondition>(G_IO_IN)) == 0) { // break if nothoing more to read
            break;
        }
    }
}

void
LineReaderChannel::drain()
{
    if (m_done) {   // have already shut this
        return;
    }
    //std::cout << "LineReader::drain " << static_cast<void*>(m_channel) << std::endl;
    //if (!g_source_is_destroyed(m_source)) {   // there are different sources for gio&gtk
    g_source_remove(m_source);   // stop messaging
    //}
    g_autoptr(GError) error{nullptr};
    gchar *string{nullptr};
    gsize size{};
    g_io_channel_read_to_end(m_channel, &string, &size, &error);
    if (error) {
         std::cout << psc::fmt::format("error {} reading", error->message) << std::endl;
    }
    else if (string != nullptr && std::strlen(string) > 0) {
        //std::cout << "LineReader::drain " << static_cast<void*>(m_channel)
        //          << " str \"" << string << "\"" << std::endl;
        m_readerListener->notify(m_err, string);
        g_free(string);
    }
}

// kept for reference, this is the classic "select/read" method (needs to be called on a separate thread)
void
LineReaderChannel::reads()
{
    std::array<char, 8192> outArr;
    fd_set fdset;
    FD_ZERO(&fdset); /* clear the set */
    FD_SET(m_out, &fdset); /* add our file descriptor to the set */

    fd_set fderr;

    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 50000000ul;

    sigset_t sigset;
    sigfillset(&sigset);    // any signal

    while (true) { // !strm->is_closed()
        //std::cout << "Readers::read fd " << m_out << std::endl;
        FD_ZERO(&fderr); /* clear the set */
        int rv = pselect(1, &fdset, NULL, &fderr, &timeout, NULL);
        if (rv == -1) {
            std::cout << "error select " <<  m_out << std::endl; /* an error accured */
            break;
        }
        else if (rv == 0) {
            //std::cout << "Reader::read timeout " << m_out << std::endl; /* a timeout occured */
        }
        else {
            if (FD_ISSET(m_out, &fderr)) {
                std::cout << "err set!" << m_out << std::endl;
            }
            auto sizeRead = read(m_out, outArr.data(), outArr.size()); /* there was data to read */
            //std::cout << "Readers::read read " << sizeRead << std::endl;
            //gsize sizeStd = strm->read(outArr.data(), outArr.size());
            if (sizeRead > 0) {
                std::cout << "Reader::read got " << sizeRead << std::endl;
                std::string data(outArr.data(), sizeRead);    // use std::string as this may contain utf-8
                //m_str += data;
            }
            else {
                if (sizeRead < 0) {
                    break;
                }
                //using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
                //std::this_thread::sleep_for(50ms);
            }
        }
    }
    //m_done = true;
    std::cout << "Readers::reads done " << m_out << std::endl;
}


LineReaderData::LineReaderData(gint out, bool err, ReaderListener* readerListener)
: m_out{out}
, m_err{err}
, m_channel{Gio::UnixInputStream::create(m_out,false)}
, m_dataStrm{Gio::DataInputStream::create(m_channel)}
, m_readerListener{readerListener}
, m_cancellable{Gio::Cancellable::create()}
{
    // channels offer some convenient way to read stuff
    //g_autoptr(GError) error{nullptr};
    //g_io_channel_set_encoding(m_channel, nullptr, &error);
    //if (error) {
    //    psc::log::Log::logAdd(psc::log::Level::Error,
    //                  [&] {
    //                      return psc::fmt::format("error {} set enconding", error->message);
    //                  });
    //}
    /* Add watches to channels (read/hup callback) */
    //m_source = g_io_add_watch(m_channel, static_cast<GIOCondition>(G_IO_IN | G_IO_HUP), cb_out_watch, this);   // (GIOFunc)
    m_dataStrm->read_line_async(sigc::mem_fun(*this, &LineReaderData::asyncReady), m_cancellable);
}

LineReaderData::~LineReaderData()
{
    if (m_channel) {
        if (!m_done) {
            m_channel->close();
        }
        //g_io_channel_unref(m_channel);
    }
    close(m_out);
}

void
LineReaderData::asyncReady(Glib::RefPtr<Gio::AsyncResult> result)
{
    try {
        std::string string;
        m_dataStrm->read_line_finish(result, string);
        string += '\n';
        m_readerListener->notify(m_err, string);
        if (!m_cancellable->is_cancelled()) {   // keep asking for more
            m_dataStrm->read_line_async(sigc::mem_fun(*this, &LineReaderData::asyncReady), m_cancellable);
        }
    }
    catch (const Glib::Error& error) {  // catch canceled
        if (!m_cancellable->is_cancelled()) {   // ignore on cancel
            std::cout << "LineReader::asyncReady " << error.what() << std::endl;
        }
    }
}

void
LineReaderData::drain()
{
    if (m_done) {   // have already shut this
        return;
    }
    //std::cout << "LineReader::drain " << static_cast<void*>(m_channel) << std::endl;
    //if (!g_source_is_destroyed(m_source)) {   // there are different sources for gio&gtk
    //g_source_remove(m_source);   // stop messaging
    //}
    std::vector<std::string> lines;
    lines.reserve(32);
    if (m_dataStrm->is_closed()) {
        return;
    }
    m_cancellable->cancel();    // finish async
}

// kept for reference, this is the classic "select/read" method (needs to be called on a separate thread)
void
LineReaderData::reads()
{
    std::array<char, 8192> outArr;
    fd_set fdset;
    FD_ZERO(&fdset); /* clear the set */
    FD_SET(m_out, &fdset); /* add our file descriptor to the set */

    fd_set fderr;

    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 50000000ul;

    sigset_t sigset;
    sigfillset(&sigset);    // any signal

    while (true) { // !strm->is_closed()
        //std::cout << "Readers::read fd " << m_out << std::endl;
        FD_ZERO(&fderr); /* clear the set */
        int rv = pselect(1, &fdset, NULL, &fderr, &timeout, NULL);
        if (rv == -1) {
            std::cout << "error select " <<  m_out << std::endl; /* an error accured */
            break;
        }
        else if (rv == 0) {
            //std::cout << "Reader::read timeout " << m_out << std::endl; /* a timeout occured */
        }
        else {
            if (FD_ISSET(m_out, &fderr)) {
                std::cout << "err set!" << m_out << std::endl;
            }
            auto sizeRead = read(m_out, outArr.data(), outArr.size()); /* there was data to read */
            //std::cout << "Readers::read read " << sizeRead << std::endl;
            //gsize sizeStd = strm->read(outArr.data(), outArr.size());
            if (sizeRead > 0) {
                std::cout << "Reader::read got " << sizeRead << std::endl;
                std::string data(outArr.data(), sizeRead);    // use std::string as this may contain utf-8
                //m_str += data;
            }
            else {
                if (sizeRead < 0) {
                    break;
                }
                //using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
                //std::this_thread::sleep_for(50ms);
            }
        }
    }
    //m_done = true;
    std::cout << "Readers::reads done " << m_out << std::endl;
}


void
Launcher::run(const PtrLaunchCmd& launchCmd)
{
    std::vector<gchar *> argv;
    auto& args = launchCmd->getParam();
    argv.reserve(args.size() + 1);
    for (uint32_t i = 0; i <= args.size(); ++i) {
        if (i < args.size()) {
            //std::cout << "Launch::Launch arg[" << i << "] " << args[i] << std::endl;
            argv.push_back(const_cast<gchar *>(args[i].c_str()));
        }
        else {
            argv.push_back(nullptr);
        }
    }

    g_autoptr(GError) error{nullptr};
    auto workDir = launchCmd->getWorkDir()->get_path();
    // Spawn child process.
    g_spawn_async_with_pipes(workDir.c_str() // workingdir
                            , &argv[0]  // arguments
                            , launchCmd->getRunEnv()   // env
                            , static_cast<GSpawnFlags>(G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH)
                            , nullptr   // child_setup
                            , this      // user data
                            , &child_pid
                            , &child_stdin, &child_stdout, &child_stderr
                            , &error);
    if (error) {
        std::cout <<   psc::fmt::format("error {} spawning child failed", error->message) << std::endl;
        return;
    }
    //for (size_t i = 0; i < argv.size(); ++i) {
    //    std::cout << "cmd[" << i << "] " << (argv[i] != nullptr ? argv[i] : "null") << std::endl;
    //}
    std::cout <<  psc::fmt::format("spawning child {} in {} out {} err {}", child_pid, child_stdin, child_stdout, child_stderr) << std::endl;
    // Add a child watch function which will be called when the child process
    // exits.
    g_child_watch_add(child_pid, child_watch_cb, this);
    // You could watch for output on @child_stdout and @child_stderr using
    // #GUnixInputStream or #GIOChannel here.
    //using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
    m_readerStd = std::make_shared<LineReaderData>(child_stdout, false, m_readerListener);
    m_readerErr = std::make_shared<LineReaderData>(child_stderr, true, m_readerListener);
    m_strmStdin = Gio::UnixOutputStream::create(child_stdin, true);
}

void
Launcher::communicate(const std::string& message)
{
    [[maybe_unused]] auto size = m_strmStdin->write(message);
}

void
Launcher::childExited(bool stat, int exit)
{
    if (stat) { //  && exit == 0
        m_readerStd->drain();   // transfer remaining messages before starting new
        m_readerErr->drain();
    }
    m_readerListener->done(stat, exit);
}

