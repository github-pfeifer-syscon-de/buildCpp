/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2020 rpf
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
#include <StringUtils.hpp>
#include <psc_Files.hpp>
#include <Log.hpp>  // formatter Glib::Error
#include <cstring>


#include "BuildCppWin.hpp"
#include "BuildCppApp.hpp"
#include "buildcpp_config.h"

BuildCppWin::BuildCppWin(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, BuildCppApp *application)
: Gtk::ApplicationWindow(cobject)       //Calls the base class constructor
, m_application{application}
, m_cancel{Gio::Cancellable::create()}
{
    set_title("BuildCpp");
    auto pix = Gdk::Pixbuf::create_from_resource(m_application->get_resource_base_path() + "/buildcpp.png");
    set_icon(pix);
    builder->get_widget("text_view", m_textView);
    builder->get_widget("tree_view", m_treeView);
    Gtk::Button* btn_build{};
    builder->get_widget("btn_build", btn_build);

    load_config();
    activate_actions();

    m_treeView->append_column("Path", m_listColumns.path);
    m_treeView->append_column("Last", m_listColumns.last);
    m_treeView->append_column("Diff", m_listColumns.diff);
    m_treeView->set_model(findPaths());


    btn_build->signal_clicked().connect(sigc::mem_fun(*this, &BuildCppWin::build));

    // not working see CalcTextView
    //set_events(Gdk::KEY_PRESS_MASK);
    //m_textView->signal_key_press_event().connect(sigc::mem_fun(*this, &BuildCppWin::key_pressed));

    show_all_children();
}

void
BuildCppWin::activate_actions()
{

}

void
BuildCppWin::build()
{
    auto iter = m_treeView->get_selection()->get_selected();
    if (iter) {
        auto row = *iter;
        auto proj = row.get_value(m_listColumns.proj);
        auto cmds = proj->getBuildCmds(m_settings);
        m_commands.splice(m_commands.end(), cmds);
        runNext();
    }
    else {
        show_error("Please select project.");
    }
}

Glib::RefPtr<Gtk::ListStore>
BuildCppWin::findPaths()
{
    auto buildRoot = m_settings->getString(MAIN_GRP, PROJ_ROOT_KEY);
    Glib::RefPtr<Gio::File> fBuildRoot;
    bool saveConfig{};
    if (buildRoot.empty()) {
        auto home = Glib::get_home_dir();
        auto fHome = Gio::File::create_for_path(home);
        fBuildRoot = fHome->get_child(Project::DEFAULT_BUILD_ROOT);
        m_settings->setString(MAIN_GRP, PROJ_ROOT_KEY, fBuildRoot->get_path());
        saveConfig = true;
    }
    else {
        fBuildRoot = Gio::File::create_for_path(buildRoot);
    }
    auto packDb = m_settings->getString(MAIN_GRP, PACMAN_DB_KEY);
    if (packDb.empty()) {
        m_settings->setString(MAIN_GRP, PACMAN_DB_KEY, "/var/local/pacman/custom.db.tar.gz");
        saveConfig = true;
    }
    if (saveConfig) {
        save_config();
    }
    auto list = Gtk::ListStore::create(m_listColumns);
    Project::findPaths(fBuildRoot, list,m_listColumns, m_cancel);
    return list;
}

void
BuildCppWin::apply_font(bool defaultFont )
{
    //Glib::ustring font = m_settings->get_string(CONFIG_FONT);
    //Glib::RefPtr<Gtk::TextBuffer> buffer = m_textView->get_buffer();

  //   if (!defaultFont) {
		// if (!m_fontTag) {
		// 		m_fontTag = buffer->create_tag();
		// 		m_settings->bind(CONFIG_FONT, m_fontTag.get(), "font");
		// 		//std::cout << "Bind font " << font << std::endl;
		// }
		// buffer->apply_tag(m_fontTag, start_iter, end_iter);
		// 	//std::cout << "   apply font " << font
		// 	//          << " start " <<  start_iter.get_line()
		// 	//          << " end " <<  end_iter.get_line()
		// 	//          << std::endl;
  //   }
  //   else {
  //       if (m_fontTag) {
		// 	buffer->remove_tag(m_fontTag, start_iter, end_iter);
		// 	m_fontTag.clear();
  //           //std::cout << "   clear font " << font << std::endl;
  //       }
  //   }
}

BuildCppApp *
BuildCppWin::getApplication()
{
    return m_application;
}

void
BuildCppWin::load_config()
{
    m_settings = std::make_shared<KeyConfig>(BUILD_PACKAGE ".conf");

}

std::shared_ptr<KeyConfig>
BuildCppWin::getSettings()
{
    return m_settings;
}

void
BuildCppWin::save_config()
{
    if (m_settings) {
        m_settings->saveConfig();
    }
}


void
BuildCppWin::on_hide()
{
    save_config();
    Gtk::ApplicationWindow::on_hide();
}

void
BuildCppWin::show_error(const Glib::ustring& msg, Gtk::MessageType type)
{
    // this shoud automatically give some context
    g_warning("show_error %s", msg.c_str());
    Gtk::MessageDialog messagedialog(*this, msg, FALSE, type);
    messagedialog.run();
    messagedialog.hide();
}

void
BuildCppWin::runNext()
{
    PtrProjectCmd projectCmd;
    while (true) {  // work on sync commands
        if (m_commands.empty()) {
            return;
        }
        projectCmd = m_commands.front();
        m_commands.pop_front();
        auto syncCmd = std::dynamic_pointer_cast<SyncCmd>(projectCmd);
        if (syncCmd) {
            if (!syncCmd->run()) {
                show_error("Error running command!");
                m_commands.clear(); // break exec
            }
            continue;
        }
        else {
            break;
        }
    }
    if (!m_launcher) {
        m_launcher = std::make_shared<Launcher>(this);
    }
    auto asyncCmd = std::dynamic_pointer_cast<AsyncCmd>(projectCmd);
    asyncCmd->prepare();
    m_launcher->run(asyncCmd);
}

void
BuildCppWin::notify(bool err, const std::string& str)
{
    //std::cout << "Project::notify " << (err ? "err" : "std") << " " << string << std::endl;
    Glib::ustring msg;
    msg.reserve(str.length() + 5);
    if (err) {
        msg = "Err ";
    }
    msg += str;
    auto buf= m_textView->get_buffer();
    auto ins = buf->get_insert();
    auto end = buf->insert(ins->get_iter(), msg);
    m_textView->scroll_to(end);
}

void
BuildCppWin::done(bool stat, int exit)
{
    if (exit == 0) {
        m_textView->get_buffer()->insert_at_cursor(
            "Completed successfully\n");
    }
    else {
         m_textView->get_buffer()->insert_at_cursor(
         Glib::ustring::sprintf("Failed %d\n", exit));
        m_commands.clear();     // stop executing
    }
    runNext();
}

