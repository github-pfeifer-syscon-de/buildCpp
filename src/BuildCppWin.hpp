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

#pragma once

#include <gtkmm.h>
#include <memory>
#include <KeyConfig.hpp>

#include "Project.hpp"

class ProjListColumns
: public Gtk::TreeModelColumnRecord
{
public:
    Gtk::TreeModelColumn<Glib::ustring> path;
    Gtk::TreeModelColumn<Glib::ustring> last;
    Gtk::TreeModelColumn<Glib::ustring> diff;
    Gtk::TreeModelColumn<std::shared_ptr<Project>> proj;
    ProjListColumns()
    {
        add(path);
        add(last);
        add(diff);
        add(proj);
    }
    virtual ~ProjListColumns() = default;
};

class BuildCppApp;

/*
 * Window that is the central visible piece of application,
 *   loaded from resource, uses subclassed components,
 *   uses settings to store and application state.
 */
class BuildCppWin
: public Gtk::ApplicationWindow
, public ReaderListener {
public:
    BuildCppWin(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder, BuildCppApp *appl);
    virtual ~BuildCppWin() = default;

    void on_hide() override;
    void show_error(const Glib::ustring& msg, Gtk::MessageType type = Gtk::MessageType::MESSAGE_WARNING);
    void apply_font(bool defaultFont);
    BuildCppApp *getApplication();
    std::shared_ptr<KeyConfig> getSettings();

    static constexpr auto MAIN_GRP{"Main"};
    static constexpr auto PROJ_ROOT_KEY{"ProjRoot"};
    static constexpr auto PACMAN_DB_KEY{"PacmanDb"};
    static constexpr auto ASKPASS_KEY{"Askpass"};
protected:
    void done(bool stat, int exit) override;
    void notify(bool err, const std::string& str) override;
    void runNext();

    ProjListColumns m_listColumns;

    Glib::RefPtr<Gtk::ListStore> findPaths();
    void build();
    void load_config();
    void save_config();
    void activate_actions();

    BuildCppApp *m_application;
    Gtk::TextView* m_textView{};
    Gtk::TreeView* m_treeView{};
    std::shared_ptr<KeyConfig> m_settings;
    Glib::RefPtr<Gio::Cancellable> m_cancel;

    std::shared_ptr<Launcher> m_launcher;
    std::list<PtrProjectCmd> m_commands;
    Glib::RefPtr<Gtk::TextBuffer::Tag> m_tagErr;
    Glib::RefPtr<Gtk::TextBuffer::Tag> m_tagOk;
    Glib::RefPtr<Gtk::TextBuffer::Tag> m_tagFail;
};

