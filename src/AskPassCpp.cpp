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

#include <iostream>

#include "AskPassCpp.hpp"

AskPassCpp::AskPassCpp(int argc, char **argv)
: Gtk::Application(argc, argv
            , "de.pfeifer_syscon.askpasscpp"
            , Gio::APPLICATION_HANDLES_COMMAND_LINE)
{
}

int
AskPassCpp::on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine>& cmd)
{
    int argc{};
    char** argv = cmd->get_arguments(argc);
    //std::cout << "AskPassCpp::on_command_line " << argc << std::endl;
    if (argc > 1) {
        m_hint = argv[1];
        //std::cout << "AskPassCpp::on_command_line argv[1] " << m_hint << std::endl;
    }
    else {
        std::string askpass = Glib::getenv("ASKPASS");
        if (!askpass.empty()) {
            //std::cout << "AskPassCpp::on_command_line ASKPASS " << askpass << std::endl;
            m_hint = askpass;
        }
        else {
            m_hint = "Please enter password:";
        }
    }
    return TRUE;
}

int
AskPassCpp::run()
{
    int ret = Gtk::Application::run();
    auto builder = Gtk::Builder::create();
    try {
        builder->add_from_resource(get_resource_base_path() + "/askpass-dlg.ui");
        builder->get_widget("askpassDlg", m_askPassDialog);
        builder->get_widget("label", m_label);
        builder->get_widget("entry", m_entry);
        //add_window(*m_askPassDialog);
        m_label->set_text(m_hint);
        ret = m_askPassDialog->run();
        if (ret == Gtk::RESPONSE_OK) {
            std::cout << m_entry->get_text() << std::endl;
            ret = 0;
        }
        else {
            ret = 1;
        }

    }
    catch (const Glib::Error &ex) {
        std::cerr << "Unable to load askpass-dlg: " << ex.what() << std::endl;
        ret = 1;
    }
    return ret;
}

//void
//AskPassCpp::on_activate()
//{
//    std::cout << "AskPassCpp::on_activate" << std::endl;
//}
//void
//AskPassCpp::on_open(const type_vec_files& files, const Glib::ustring& hint)
//{
//    std::cout << "AskPassCpp::on_open" << std::endl;
//}
//
// void
// AskPassCpp::on_action_quit()
// {
//     m_askPassDialog->hide();
//
//     // and we shoud delete appWindow if we were not going exit anyway
//     // Not really necessary, when Gtk::Widget::hide() is called, unless
//     // Gio::Application::hold() has been called without a corresponding call
//     // to Gio::Application::release().
//     quit();
// }
//void
//AskPassCpp::on_startup()
//{
    //std::cout << "AskPassCpp::on_startup" << std::endl;
//    Gtk::Application::on_startup();
//}

int main(int argc, char** argv)
{
    char* loc = std::setlocale(LC_ALL, "");
#   ifndef __MINGW32__
    // sync c++ (if we are not on windows)
    std::locale::global(std::locale(loc));
#   endif
    //}
    //bindtextdomain(PACKAGE, PACKAGE_LOCALE_DIR);
    //textdomain(PACKAGE);
    Glib::init();

    AskPassCpp app(argc, argv);
    return app.run();
}
