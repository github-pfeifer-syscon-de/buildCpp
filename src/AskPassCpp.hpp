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

#include <gtkmm.h>

class AskPassCpp
: public Gtk::Application
{
public:
    AskPassCpp(int argc, char **argv);
    explicit AskPassCpp(const AskPassCpp& other) = delete;
    virtual ~AskPassCpp() = default;

    int on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) override;
    int run();

    //void on_activate() override;
    //void on_open(const type_vec_files& files, const Glib::ustring& hint) override;
    //void on_startup() override;
private:
    Gtk::Dialog* m_askPassDialog;
    Gtk::Label* m_label;
    Gtk::Entry* m_entry;
    Glib::ustring m_hint;
};
