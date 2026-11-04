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

#include "Project.hpp"
#include "BuildCppWin.hpp"


Glib::ustring
Project::getPath()
{

    return (m_file->get_basename());
}

Glib::ustring
Project::getLast() {
    if (m_last.empty()) {
        auto lastTime = Glib::DateTime::create_now_utc(0);
        auto iter = m_file->enumerate_children(m_cancel);
        while (true) {
            auto fileInfo = iter->next_file();
            if (!fileInfo) {
                break;
            }
            auto modify = fileInfo->get_modification_date_time();
            if (modify.compare(lastTime) > 0) {
                lastTime = modify;
            }
        }
        m_last = lastTime.format_iso8601();
    }
    return m_last;
}

Glib::ustring
Project::getDiff() {
    if (m_diff.empty()) {
        auto src = m_file->get_child(SRC_DIR);
        auto build = m_file->get_child(BUILD_DIR);
        if (src && src->query_exists()
         && build && build->query_exists()) {
            auto srcInfo = src->query_info();
            auto buildInfo = build->query_info();
            auto srcMod = srcInfo->get_modification_date_time();
            auto buildMod = buildInfo->get_modification_date_time();
            auto diff = srcMod.difference(buildMod);
            bool neg{};
            if (diff < 0) {
                diff = -diff;
                m_diff = "-";
            }
            auto days = diff / USEC_DAYS;
            diff -= days * USEC_DAYS;
            auto hours = (diff / USEC_HOURS);
            diff -= hours * USEC_HOURS;
            auto minutes = (diff / USEC_MINUTES);
            diff -= minutes * USEC_MINUTES;
            m_diff += Glib::ustring::format("%ld:%ld:%ld", days, hours, minutes);
        }
    }
    return m_diff;
}

void
Project::findPaths(const Glib::RefPtr<Gio::File>& fBuildRoot
        , const Glib::RefPtr<Gtk::ListStore>& list
        , ProjListColumns& listColumns
        , const Glib::RefPtr<Gio::Cancellable>& cancel)
{
    auto iter = fBuildRoot->enumerate_children(cancel);
    while (true) {
        auto fileInfo = iter->next_file();
        if (!fileInfo) {
            break;
        }
        if (fileInfo->get_file_type() == Gio::FileType::FILE_TYPE_DIRECTORY) {
            auto chld = iter->get_child(fileInfo);
            auto proj = std::make_shared<Project>(chld, cancel);
            if (proj->isUsable()) {
                auto iter = list->append();
                auto row = *iter;
                row.set_value(listColumns.path, proj->getPath());
                row.set_value(listColumns.last, proj->getLast());
                row.set_value(listColumns.diff, proj->getDiff());
                row.set_value(listColumns.proj, proj);
            }
        }
    }
}

