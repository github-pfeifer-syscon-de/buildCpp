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

#include <glibmm.h>
#include <gtkmm.h>

class ProjListColumns;

class Project {
public:
    Project(const Glib::RefPtr<Gio::File>& file
        , const Glib::RefPtr<Gio::Cancellable>& cancel)
    : m_cancel{cancel}
    , m_file{file}
    {
    }
    virtual ~Project() = default;
    static constexpr auto MESON_BUILD{"meson.build"};
    static constexpr auto DEFAULT_BUILD_ROOT{"csrc.git"};
    static constexpr auto SRC_DIR{"src"};
    static constexpr auto BUILD_DIR{"build"};
    static constexpr auto USEC_MINUTES{60l * G_TIME_SPAN_SECOND};
    static constexpr auto USEC_HOURS{60l * USEC_MINUTES};
    static constexpr auto USEC_DAYS{24l * USEC_HOURS};

    bool isUsable() {
        auto meson = m_file->get_child(Project::MESON_BUILD);
        return meson->query_exists();
    }
    Glib::ustring getPath();
    Glib::ustring getLast();
    Glib::ustring getDiff();
    static void findPaths(const Glib::RefPtr<Gio::File>& fBuildRoot
            , const Glib::RefPtr<Gtk::ListStore>& list
            , ProjListColumns& listColumns
            , const Glib::RefPtr<Gio::Cancellable>& cancel);

private:
    Glib::RefPtr<Gio::Cancellable> m_cancel;
    Glib::RefPtr<Gio::File> m_file;
    Glib::ustring m_last;
    Glib::ustring m_diff;
};
