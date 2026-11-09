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
#include <string_view>
#include <psc_i18n.hpp>
#include <locale.h>

#include "buildcpp_config.h"
#include "BuildCppApp.hpp"

BuildCppApp::BuildCppApp(int argc, char **argv)
: Gtk::Application(argc, argv, "de.pfeifer_syscon.buildcpp")
, m_exec{argv[0]}
{
#   ifdef DEBUG_BUILDCPPAPP
    std::cout << "BuildCppApp::BuildCppApp" << std::endl;
#   endif
}

void
BuildCppApp::on_activate()
{
#   ifdef DEBUG_BUILDCPPAPP
    std::cout << "BuildCppApp::on_activate" << std::endl;
#   endif
    auto builder = Gtk::Builder::create();
    try {
        builder->add_from_resource(get_resource_base_path() + "/build-win.ui");
        builder->get_widget_derived("BuildWin", m_buildAppWindow, this);
        add_window(*m_buildAppWindow);
        m_buildAppWindow->show();
    }
    catch (const Glib::Error &ex) {
        std::cerr << "Unable to load build-win: " << ex.what() << std::endl;
    }
}


void
BuildCppApp::on_action_quit()
{
    m_buildAppWindow->hide();

    // and we shoud delete appWindow if we were not going exit anyway
    // Not really necessary, when Gtk::Widget::hide() is called, unless
    // Gio::Application::hold() has been called without a corresponding call
    // to Gio::Application::release().
    quit();
}

template<typename T> void
BuildCppApp::build(const std::string& resName
                , const std::string& compName
                , std::function<void(const Glib::RefPtr<T>& dialog, const Glib::RefPtr<Gtk::Builder>& builder)> function)
{
    try {
#       ifdef DEBUG_BUILDCPPAPP
        std::cout << "BuildCppApp::build" << std::endl;
#       endif
        auto builder = Gtk::Builder::create();
        builder->add_from_resource(get_resource_base_path() + "/" + resName);
        auto dlgObj = builder->get_object(compName);
        auto dialog = Glib::RefPtr<T>::cast_dynamic(dlgObj);
        function(dialog, builder);
        dialog->set_transient_for(*m_buildAppWindow);
        dialog->show_all();
        dialog->run();
        dialog->hide();
    }
    catch (const Glib::Error &ex) {
        std::cerr << "Unable to load " << resName << " error " << ex.what() << std::endl;
    }
}

void
BuildCppApp::on_action_about() {
    build<Gtk::AboutDialog>("abt-dlg.ui", "abt-dlg", [this] (const Glib::RefPtr<Gtk::AboutDialog>& dialog, const Glib::RefPtr<Gtk::Builder>& builder)
    {
        auto icon = Gdk::Pixbuf::create_from_resource(get_resource_base_path() + "/calcpp.png");
        dialog->set_logo(icon);
    });
}




void
BuildCppApp::on_action_help()
{
    build<Gtk::Dialog>("help-dlg.ui", "help-dlg", [this] (const Glib::RefPtr<Gtk::Dialog>& dialog, const Glib::RefPtr<Gtk::Builder>& builder)
    {
        auto textObj = builder->get_object("text");
        auto text = Glib::RefPtr<Gtk::TextView>::cast_dynamic(textObj);
        //text->get_buffer()->set_text(getReadmeText());
    });
}

Glib::RefPtr<Gtk::Builder>
BuildCppApp::get_menu_builder()
{
    return m_builder;
}

Glib::StdStringView
BuildCppApp::get_exec_path()
{
    return m_exec;
}

void
BuildCppApp::on_startup()
{
#ifdef DEBUG_BUILDCPPAPP
    std::cout << "BuildCppApp::on_startup" << std::endl;
#endif
    Gtk::Application::on_startup();

    add_action("quit", sigc::mem_fun(*this, &BuildCppApp::on_action_quit));
    add_action("about", sigc::mem_fun(*this, &BuildCppApp::on_action_about));
    add_action("help", sigc::mem_fun(*this, &BuildCppApp::on_action_help));

    m_builder = Gtk::Builder::create();
    try {
        m_builder->add_from_resource(get_resource_base_path() + "/app-menu.ui");
        auto menuObj = m_builder->get_object("menubar");
        auto menuBar = Glib::RefPtr<Gio::Menu>::cast_dynamic(menuObj);
        if (menuBar)
            set_menubar(menuBar);
        else
            std::cerr << "Cound not find/cast menubar!" << std::endl;
    }
    catch (const Glib::FileError& ex) {
        std::cerr << "Unable to load menubar: " << ex.what() << std::endl;
    }
}

int main(int argc, char** argv)
{
#   ifdef DEBUG_BUILDCPPAPP
    std::cout << "main" << std::endl;
#   endif
    char* loc = std::setlocale(LC_ALL, "");
#   ifdef DEBUG_BUILDCPPAPP
    if (loc == nullptr) {
        std::cout << "error setlocale " << std::endl;
    }
    else {
        std::cout << "setlocale " << loc << std::endl;
    }
#   endif
#   ifndef __MINGW32__
    // sync c++ (if we are not on windows)
    std::locale::global(std::locale(loc));
#   endif
    //}
    //bindtextdomain(PACKAGE, PACKAGE_LOCALE_DIR);
    //textdomain(PACKAGE);
    Glib::init();

    BuildCppApp app(argc, argv);
    return app.run();
}
