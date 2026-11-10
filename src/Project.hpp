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
#include <memory>
#include <StringUtils.hpp>

#include "Launcher.hpp"

class ProjListColumns;
class ProjectCmd;

class Project
{
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
    static constexpr auto PACK_SUFFIX{".tar.zst"};
    static constexpr auto PKGBUILD_NAME{"PKGBUILD"};

    std::list<std::shared_ptr<ProjectCmd>> getBuildCmds(const std::shared_ptr<KeyConfig>& config);
    bool isUsable() {
        auto meson = m_file->get_child(Project::MESON_BUILD);
        return meson->query_exists();
    }
    Glib::ustring getName();
    Glib::ustring getPath();
    Glib::ustring getLast();
    Glib::ustring getDiff();
    static void findPaths(const Glib::RefPtr<Gio::File>& fBuildRoot
            , const Glib::RefPtr<Gtk::ListStore>& list
            , ProjListColumns& listColumns
            , const Glib::RefPtr<Gio::Cancellable>& cancel);
    Glib::RefPtr<Gio::File> getProjectDir() {
        return m_file;
    }
    Glib::RefPtr<Gio::Cancellable> getCancel() {
        return m_cancel;
    }
    Glib::RefPtr<Gio::File> getBuildDir();

protected:

private:
    Glib::RefPtr<Gio::Cancellable> m_cancel;
    Glib::RefPtr<Gio::File> m_file;
    Glib::ustring m_last;
    Glib::ustring m_diff;
    Glib::ustring m_name;
};


class ProjectCmd
{
public:
    ProjectCmd() = default;
    virtual ~ProjectCmd() = default;

    virtual void prepare() {
    }

    std::vector<Glib::RefPtr<Gio::File>> findPackages(Project* project);
};

using PtrProjectCmd = std::shared_ptr<ProjectCmd>;

class SyncCmd
: public ProjectCmd
{
public:
    SyncCmd(Project* project)
    : m_project{project}
    {
    }
    virtual ~SyncCmd() = default;


    virtual bool run() = 0;
protected:
    Project* m_project;
};

class AsyncCmd
: public ProjectCmd
, public LaunchCmd
{
public:
    AsyncCmd() = default;
    virtual ~AsyncCmd() = default;

};

class PrepareCmd
: public SyncCmd
{
public:
    PrepareCmd(Project* project)
    : SyncCmd(project)
    {
    }
    virtual ~PrepareCmd() = default;

    bool run() override;
    bool linkPkgbuild(Project* project);
};

class Repoadd
: public AsyncCmd
{
public:
    Repoadd(Project* project)
    : m_project{project}
    {
    }
    explicit Repoadd(const Repoadd& other) = delete;
    virtual ~Repoadd() = default;

    void prepare() override;

private:
    Project* m_project;
};

class PacmanCmd
: public AsyncCmd
{
public:
    PacmanCmd(Project* project)
    : m_project{project}
    {
    }

    explicit PacmanCmd(const PacmanCmd& other) = delete;
    virtual ~PacmanCmd() = default;
    static constexpr auto ASKPASS_ENV{"SUDO_ASKPASS"};
    void addAskpass(const std::string& askpass);
    void prepare() override;


private:
    Project* m_project;
    bool m_setaskPass{};
    std::string m_askpass;
};
