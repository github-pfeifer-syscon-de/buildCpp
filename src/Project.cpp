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
#include <psc_Files.hpp>
#include <cstring>

#include "Project.hpp"
#include "BuildCppWin.hpp"

std::vector<Glib::RefPtr<Gio::File>>
ProjectCmd::findPackages(Project* project)
{
    std::vector<Glib::RefPtr<Gio::File>> packages;
    packages. reserve(2);
    auto buildDir = project->getBuildDir();
    auto iter = buildDir->enumerate_children(project->getCancel());
    while (true) {
        auto fileInfo = iter->next_file();
        if (!fileInfo) {
            break;
        }
        if (fileInfo->get_file_type() == Gio::FileType::FILE_TYPE_REGULAR
         && StringUtils::endsWith(fileInfo->get_name(), Project::PACK_SUFFIX)) {
            auto file = iter->get_child(fileInfo);
            packages.push_back(file);
         }
    }
    return packages;
}

bool
PrepareCmd::run() {
    auto packages = findPackages(m_project);
    for (auto& package : packages) {
        package->remove(m_project->getCancel());
    }
    return linkPkgbuild(m_project);
};


bool
PrepareCmd::linkPkgbuild(Project* project)
{
    auto pkgBuild = project->getSourcePkgbuild();
    if (pkgBuild->query_exists()) {
        auto buildDir = project->getBuildDir();
        auto pkgBuildTarget = buildDir->get_child(Project::PKGBUILD_NAME);
        if (!pkgBuildTarget->query_exists()) {
            //pkgBuildTarget->query_file_type() != Gio::FileType::FILE_TYPE_SYMBOLIC_LINK) {
            return pkgBuildTarget->make_symbolic_link(pkgBuild->get_path());
        }
        return true;
    }
    return false;
}


void
Repoadd::prepare()
{
    auto packDb = m_param[1];
    auto packDbFile = Gio::File::create_for_path(packDb);
    auto packDir = packDbFile->get_parent();
    setWorkDir(packDir);
    auto packs = findPackages(m_project);
    for (auto& pack : packs) {
        auto baseName = pack->get_basename();
        //std::cout << "Repoadd::prepare " << baseName << std::endl;
        auto movedPack = packDir->get_child(baseName);
        pack->move(movedPack, Gio::FILE_COPY_OVERWRITE);
        addParam(baseName);
    }
}

AsyncAskCmd::~AsyncAskCmd()
{
    if (m_runEnv != nullptr){
        g_strfreev(m_runEnv);
    }
}

void
AsyncAskCmd::addAskpass(const std::string& askpass) {
    m_askpass = askpass;
    auto askpassEnv = Glib::getenv(getAskPassEnv());
    if (askpassEnv.empty()) {
        if (m_askpass.empty()) {
            std::cout << "A existing environment var " << getAskPassEnv() << " was not found, "
                          "and the setting ~/.config/buildcpp.conf Askpass=path_askpass_executable was missing as well, "
                          "this might work if the user can use sudo/ssh directly (not recommended)" << std::endl;
        }
        else {
            m_setaskPass = true;
        }
    }
}


char**
AsyncAskCmd::getRunEnv()
{
    if (m_setaskPass) {
        if (m_runEnv == nullptr) {
            auto askpassEnv = getAskPassEnv();
            m_runEnv = g_get_environ();
            [[maybe_unused]] auto env = g_environ_setenv(m_runEnv, askpassEnv.c_str(), m_askpass.c_str(), TRUE);
        }
        return m_runEnv;
    }
    return AsyncCmd::getRunEnv();
}

void
PacmanCmd::addAskpass(const std::string& askpass)
{
    AsyncAskCmd::addAskpass(askpass);
    addParam("--askpass");
}


void
PacmanCmd::prepare() {
    auto packs = findPackages(m_project);
    for (auto pack : packs) {
        addParam(pack->get_path());
    }
}

Glib::ustring
Project::getPath()
{
    return (m_file->get_basename());
}

Glib::ustring
Project::getName()
{
    if (m_name.empty()) {   // should scan PKBUILD for adjusted name...
        m_name = (m_file->get_basename());
    }
    return m_name;
}

Glib::ustring
Project::getLast()
{
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
            auto diff = buildMod.difference(srcMod);
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
            m_diff += Glib::ustring::sprintf("%ld:%ld:%ld", days, hours, minutes);
        }
    }
    return m_diff;
}

std::list<PtrProjectCmd>
Project::getBuildCmds(const std::shared_ptr<KeyConfig>& config)
{
    std::list<PtrProjectCmd> commands;
    auto syncCmd = std::make_shared<PrepareCmd>(this);
    commands.emplace_back(std::move(syncCmd));
    auto asyncCmd = std::make_shared<AsyncCmd>();
    asyncCmd->addParam("makepkg");
    asyncCmd->addParam("--syncdeps");
    asyncCmd->addParam("--force");
    asyncCmd->setWorkDir(getBuildDir());
    commands.emplace_back(std::move(asyncCmd));
    asyncCmd = std::make_shared<Repoadd>(this);
    asyncCmd->addParam("repo-add");
    auto packDb = config->getString(BuildCppWin::MAIN_GRP, BuildCppWin::PACMAN_DB_KEY);
    asyncCmd->addParam(packDb);
    commands.emplace_back(std::move(asyncCmd));
    auto pacmanCmd = std::make_shared<PacmanCmd>(this);
    pacmanCmd->addParam("sudo");
    pacmanCmd->addAskpass(config->getString(BuildCppWin::MAIN_GRP, BuildCppWin::ASKPASS_KEY));
    pacmanCmd->addParam("--shell");
    pacmanCmd->addParam("pacman");
    pacmanCmd->addParam("--noconfirm");
    pacmanCmd->addParam("--sync");
    pacmanCmd->addParam("--refresh");
    pacmanCmd->addParam(getPackName());
    pacmanCmd->addParam(getPackName() + "-debug");
    pacmanCmd->setWorkDir(getBuildDir());
    commands.emplace_back(std::move(pacmanCmd));

    return commands;
}

std::list<PtrProjectCmd>
Project::getGitCmds(const std::shared_ptr<KeyConfig>& config)
{
    std::list<PtrProjectCmd> commands;
    auto asyncCmd = std::make_shared<GitCmd>(this);
    asyncCmd->addAskpass(config->getString(BuildCppWin::MAIN_GRP, BuildCppWin::ASKPASS_KEY));
    asyncCmd->addParam("setsid");   // use separate session see ssh/SSH_ASKPASS https://unix.stackexchange.com/questions/83986/tell-ssh-to-use-a-graphical-prompt-for-key-passphrase
    asyncCmd->addParam("git");
    asyncCmd->addParam("pull");
    asyncCmd->setWorkDir(getBuildDir());
    commands.emplace_back(std::move(asyncCmd));
    return commands;
}


Glib::RefPtr<Gio::File>
Project::getSourcePkgbuild()
{
    auto buildDir = getBuildDir();
    auto buildRoot = buildDir->get_parent()->get_parent();
    auto pkgBuild = buildRoot->get_child(getProjectDir()->get_basename() + Project::PKGBUILD_NAME);
    return pkgBuild;
}

std::string
Project::getPackName()
{
    auto sourcePgkbuid = getSourcePkgbuild();
    if (m_packName.empty()) {
        if (sourcePgkbuid->query_exists()) {
            auto stream = sourcePgkbuid->read();
            auto buffered = Gio::DataInputStream::create(stream);
            while (!buffered->is_closed()) {
                std::string line;
                if (!buffered->read_line(line)) {
                    break;
                }
                if (line.starts_with("pkgname=")) {
                    auto pos = line.find('=');
                    ++pos;
                    m_packName = line.substr(pos);
                    break;
                }
            }
        }
        else {
            m_packName = getName();
        }
    }
    return m_packName;
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

Glib::RefPtr<Gio::File>
Project::getBuildDir()
{
    auto build = m_file->get_child(BUILD_DIR);
    return build;
}

