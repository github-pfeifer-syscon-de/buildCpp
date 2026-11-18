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

#include <glibmm-2.4/glibmm.h>
#include <giomm-2.4/giomm.h>
#include <libxml/parser.h> //xmlDoc
#include <libxslt/xsltutils.h>
#include <memory>
#include <string>
#include <stdint.h>
#include <format>
#include <StringUtils.hpp>

namespace psc::xml
{

class XmlException
: public std::invalid_argument
{
public:
    XmlException(const std::string& message)
    : std::invalid_argument{message} {
    }
    XmlException(const XmlException& exception) = default;
    virtual ~XmlException() = default;
};


class XmlElement
{
public:
    XmlElement(xmlNodePtr node)
    : m_element{node}
    {
    }
    explicit XmlElement(const XmlElement& other) = delete;
    virtual ~XmlElement() = default;
    std::shared_ptr<XmlElement> addElement(const std::string& element);
    xmlNodePtr getNode() {
        return m_element;
    }
    xmlAttr* addAttribute(const std::string& name, const std::string& value);
    void addText(const std::string& text);

protected:
    xmlNodePtr m_element;
};

using PtrXmlNode = std::shared_ptr<XmlElement>;

class XmlDocument
{
public:
    XmlDocument(const std::string& documentElement, int majorVersion, int minorVersion = 0);
    XmlDocument(const std::string &document);
    XmlDocument(const xmlDocPtr docPtr);
    explicit XmlDocument(const XmlDocument& other) = delete;
    virtual ~XmlDocument();
    xmlDocPtr getXmlDocument() {
        return m_document;
    }
    PtrXmlNode getDocumentNode() {
        return m_documentNode;
    }
    // as references may get "swallod" avoid free on destruct
    void setFreeOnDestruct(bool freeOnDestruct) {
        m_freeOnDestruct = freeOnDestruct;
    }
    std::string toString();
private:
    xmlDocPtr m_document{};
    bool m_freeOnDestruct{true};
    PtrXmlNode m_documentNode{nullptr};
};

using PtrXmlDocument = std::shared_ptr<XmlDocument>;

class XsltDocument
{
public:
    XsltDocument(const PtrXmlDocument& document);
    explicit XsltDocument(const XsltDocument& other) = delete;
    virtual ~XsltDocument();
    PtrXmlDocument apply(const PtrXmlDocument& document);
private:
    xsltStylesheetPtr m_stylesheet{};
};

} // namespace psc::xml


// see https://gist.github.com/timepp/1f678e200d9e0f2a043a9ec6b3690635
struct crc32
{
    static uint32_t table[256];
    static void generate_table();
    static uint32_t update(uint32_t initial, const void* buf, size_t len);

};


// usage: the following code generates crc for 2 pieces of data
// uint32_t table[256];
// crc32::generate_table(table);
// uint32_t crc = crc32::update(table, 0, data_piece1, len1);
// crc = crc32::update(table, crc, data_piece2, len2);
// output(crc);

struct BuildData
{
    BuildData(
        const Glib::RefPtr<Gio::File>& fBuild
        , Glib::StdStringView execPath
        , const std::string& name)
    : fBuild{fBuild}
    , execPath{execPath.c_str()}
    , name{name}
    {
        const auto lower_name = StringUtils::lowerStd(name);
        doc = std::make_shared<psc::xml::XmlDocument>("proj", 1);
        auto proj = doc->getDocumentNode();
        proj->addAttribute("name", name);
        proj->addAttribute("name_lower", lower_name);
        auto res = proj->addElement("res");
        auto prefix = "/de/pfeifer_syscon/" + lower_name;
        res->addAttribute("prefix", prefix);
        std::cout << "Document\n" << doc->toString() << std::endl;
        //std::string projXml = std::format("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        //            "<proj name=\"{}\" name_lower=\"{}\">\n"
        //            "   <res prefix=\"/de/pfeifer_syscon/{}\"/>\n"
        //            "</proj>", name, lower_name, );
        //doc = std::make_shared<psc::xml::XmlDocument>(projXml);
    }
    virtual ~BuildData() = default;

    Glib::RefPtr<Gio::File> fBuild;
    std::string execPath;
    std::string name;
    psc::xml::PtrXmlDocument doc;
};

class Template
{
public:
    Template();
    explicit Template(const Template& other) = delete;
    virtual ~Template();

    void buildProj(const BuildData& buildData);
    void buildRes(const BuildData& buildData);
    void transform(
          const std::string& styleSheet
        , const psc::xml::PtrXmlDocument& doc
        , const Glib::RefPtr<Gio::File>& file);

    std::string loadTemplate(Glib::StdStringView execPath, const std::string& templateName);

protected:
    // the intention is to get res for testing run
    Glib::RefPtr<Gio::File> getResSrcPath(Glib::StdStringView execPath);
    Glib::RefPtr<Gio::File> getGlobalDataPath();
};
