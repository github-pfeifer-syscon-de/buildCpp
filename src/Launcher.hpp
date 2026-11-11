/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2026 RPf
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
#pragma once

#include <giomm.h>
#include <memory>
#include <queue>
#include <atomic>
#include <mutex>

class RpcLaunch;

class ReaderListener
{
public:
    ReaderListener() = default;
    virtual ~ReaderListener() = default;

    virtual void notify(bool err, const std::string& str) = 0;
    virtual void done(bool stat, int exit) = 0;
};


// this is a glib implementation for line reader (may work 4windows?)
class LineReaderChannel
{
public:
    LineReaderChannel(gint out, bool err, ReaderListener* readerListener);
    explicit LineReaderChannel(const LineReaderChannel& orig) = delete;
    virtual ~LineReaderChannel();

    void done();
    void ready();
    void drain();
protected:
    void reads();

    gint m_out;
    bool m_err;
    GIOChannel* m_channel{};
    ReaderListener* m_readerListener;
    guint m_source;
    bool m_done{};
};

// this is a glibmm implementation for line reader
class LineReaderData
{
public:
    LineReaderData(gint out, bool err, ReaderListener* readerListener);
    explicit LineReaderData(const LineReaderData& orig) = delete;
    virtual ~LineReaderData();

    void drain();
    void asyncReady(Glib::RefPtr<Gio::AsyncResult> result);

protected:
    void reads();

    gint m_out;
    bool m_err;
    Glib::RefPtr<Gio::UnixInputStream> m_channel;
    Glib::RefPtr<Gio::DataInputStream> m_dataStrm;
    ReaderListener* m_readerListener;
    guint m_source;
    bool m_done{};
    Glib::RefPtr<Gio::Cancellable> m_cancellable;
};

class LaunchCmd
{
public:
    LaunchCmd() = default;
    explicit LaunchCmd(const LaunchCmd& orig) = delete;
    virtual ~LaunchCmd() = default;
    void addParam(const std::string& cmd) {
        m_param.push_back(cmd);
    }
    virtual std::vector<std::string>& getParam() {
        return m_param;
    }
    void setWorkDir(const Glib::RefPtr<Gio::File>& projDir) {
        m_projDir = projDir;
    }
    Glib::RefPtr<Gio::File> getWorkDir() {
        return m_projDir;
    }
    Glib::ustring getName() {
        Glib::ustring name;
        for (auto& param : m_param) {
            if (!name.empty()) {
                name += " ";
            }
            name += param;
        }
        return name;
    }
    virtual char** getRunEnv() {
        return nullptr;
    };
protected:
    std::vector<std::string> m_param;
    Glib::RefPtr<Gio::File> m_projDir;
};

using PtrLaunchCmd = std::shared_ptr<LaunchCmd>;

class Launcher
{
public:
    Launcher(ReaderListener* readerListener)
    : m_readerListener{readerListener}
    {
    }
    explicit Launcher(const RpcLaunch& orig) = delete;
    virtual ~Launcher() = default;

    void run(const PtrLaunchCmd& launchCmd);
    void communicate(const std::string& message);
    void childExited(bool stat, int exit);
    GPid getChildPid() {
        return child_pid;
    }
protected:
    void doCommunicate(const std::string& message);

private:
    ReaderListener* m_readerListener;
    std::shared_ptr<LineReaderData> m_readerStd;
    std::shared_ptr<LineReaderData> m_readerErr;
    Glib::RefPtr<Gio::UnixOutputStream> m_strmStdin;
    gint child_stdout, child_stderr, child_stdin;
    GPid child_pid;
    std::vector<Glib::ustring> m_args;
};
