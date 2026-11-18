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
#include <format>
#include <libxml/tree.h>
#include <libxslt/transform.h>

#include "Template.hpp"


namespace psc::xml
{

#undef PscXmlDebug

PtrXmlNode
XmlElement::addElement(const std::string& element)
{
    xmlNode* node = xmlNewNode(nullptr, reinterpret_cast<const xmlChar*>(element.c_str()));
    xmlAddChild(m_element, node);
    return std::make_shared<XmlElement>(node);
}

xmlAttr*
XmlElement::addAttribute(const std::string& name, const std::string& value)
{
    return xmlNewProp(m_element
            , reinterpret_cast<const xmlChar*>(name.c_str())
            , reinterpret_cast<const xmlChar*>(value.c_str()));
}

void
XmlElement::addText(const std::string& text)
{
    auto ret = xmlNodeAddContent(m_element, reinterpret_cast<const xmlChar*>(text.c_str()));
    if (ret != 0) {
        throw XmlException(std::format("Error {} addText {}", ret, text));
    }
}


XmlDocument::XmlDocument(const std::string& documentElement, int majorVersion, int minorVersion)
{
    auto version = std::format("{}.{}", majorVersion, minorVersion);
    m_document = xmlNewDoc(reinterpret_cast<const xmlChar*>(version.c_str()));
    xmlNode* docNode = xmlNewDocNode(m_document, nullptr, reinterpret_cast<const xmlChar*>(documentElement.c_str()), nullptr);
    m_documentNode = std::make_shared<XmlElement>(docNode);
    xmlDocSetRootElement(m_document, docNode);
#   ifdef PscXmlDebug
    std::cout << "XmlDocument::XmlDocument " << static_cast<void*>(m_document)
              << " elem " << documentElement << std::endl;
#   endif
}


XmlDocument::XmlDocument(const std::string &documentContent)
: m_document{xmlReadMemory(documentContent.c_str(), documentContent.size(), nullptr, "", 0)}
{
#   ifdef PscXmlDebug
    std::cout << "XmlDocument::XmlDocument " << static_cast<void*>(m_document) << std::endl;
#   endif
}

XmlDocument::XmlDocument(const xmlDocPtr docPtr)
: m_document{docPtr}
{
#   ifdef PscXmlDebug
    std::cout << "XmlDocument::XmlDocument " << static_cast<void*>(m_document) << std::endl;
#   endif
}

XmlDocument::~XmlDocument()
{
    if (m_freeOnDestruct && m_document != nullptr) {
#       ifdef PscXmlDebug
        std::cout << "XmlDocument::~XmlDocument " << static_cast<void*>(m_document) << std::endl;
#       endif
        xmlFreeDoc(m_document);
    }
}


std::string
XmlDocument::toString()
{
    xmlChar* outputData;
    int doc_text_len;
    xmlDocDumpMemory(m_document, &outputData, &doc_text_len);
    return std::string(reinterpret_cast<const char*>(outputData), doc_text_len);
}

XsltDocument::XsltDocument(const PtrXmlDocument &document)
: m_stylesheet{xsltParseStylesheetDoc(document->getXmlDocument())}
{
#   ifdef PscXmlDebug
    std::cout << "XsltDocument::XsltDocument (doc) " << static_cast<void*>(m_stylesheet) << std::endl;
#   endif
    document->setFreeOnDestruct(false); // this "eats" the reference
}

XsltDocument::~XsltDocument()
{
#   ifdef PscXmlDebug
    std::cout << "XsltDocument::~XsltDocument " << static_cast<void*>(m_stylesheet) << std::endl;
#   endif
    if (m_stylesheet != nullptr) {
        xsltFreeStylesheet(m_stylesheet);
    }
}



PtrXmlDocument
XsltDocument::apply(const PtrXmlDocument& document)
{
    int nbparams{1};
    const char *params[nbparams] {nullptr};
    auto result = xsltApplyStylesheet(m_stylesheet, document->getXmlDocument(), params);

    return std::make_shared<XmlDocument>(result);
}

} // namespace psc::xml

uint32_t crc32::table[256];

void
crc32::generate_table()
{
    if (table[1] == 0) {
        uint32_t polynomial = 0xEDB88320;
        for (uint32_t i = 0; i < 256; i++)
        {
            uint32_t c = i;
            for (size_t j = 0; j < 8; j++)
            {
                if (c & 1) {
                    c = polynomial ^ (c >> 1);
                }
                else {
                    c >>= 1;
                }
            }
            table[i] = c;
        }
    }
}

uint32_t
crc32::update(uint32_t initial, const void* buf, size_t len)
{
    generate_table();
    uint32_t c = initial ^ 0xFFFFFFFF;
    const uint8_t* u = static_cast<const uint8_t*>(buf);
    for (size_t i = 0; i < len; ++i)
    {
        c = table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFF;
}

Template::Template()
{
}

Template::~Template()
{
    //xsltCleanupGlobals();
    //xmlCleanupParser();
}

void
Template::buildProj(const BuildData& buildData)
{
    if (!buildData.fBuild->query_exists()) {
        buildData.fBuild->make_directory();
    }
    std::string mesonXslt = loadTemplate(buildData.execPath, "meson_build.xslt");
    auto fMesonBuild = buildData.fBuild->get_child("meson.build");
    transform(mesonXslt, buildData.doc, fMesonBuild);
    buildRes(buildData);
}

void
Template::buildRes(const BuildData& buildData)
{
    auto fRes = buildData.fBuild->get_child("res");
    if (!fRes->query_exists()) {
        fRes->make_directory();
    }
    std::string mesonXslt = loadTemplate(buildData.execPath, "res_meson_build.xslt");
    auto fMesonRes = fRes->get_child("meson.build");
    transform(mesonXslt, buildData.doc, fMesonRes);
    std::string gresXslt = loadTemplate(buildData.execPath, "res_gresources.xslt");
    auto fGresXml = fRes->get_child("gresources.xml");
    transform(gresXslt, buildData.doc, fGresXml);
    std::string desktopXslt = loadTemplate(buildData.execPath, "res_desktop.xslt");
    auto fDesktop = fRes->get_child(buildData.name + ".desktop");
    transform(desktopXslt, buildData.doc, fDesktop);
}


void
Template::transform(
      const std::string& styleSheet
    , const psc::xml::PtrXmlDocument& doc
    , const Glib::RefPtr<Gio::File>& file)
{
    xmlSubstituteEntitiesDefault(1);    // convert entities
    xmlLoadExtDtdDefaultValue = 1;
    auto xsltDoc = std::make_shared<psc::xml::XmlDocument>(styleSheet);
    auto xslt = std::make_shared<psc::xml::XsltDocument>(xsltDoc);
    auto result = xslt->apply(doc);
    auto content = result->toString();
    auto tag = std::format("W/{}-{:08X}", content.length(), crc32::update(0u, reinterpret_cast<const void*>(content.data()), content.length()));
    file->replace_contents(content.c_str(), content.size(), "",  tag);
}


// the intention is to get res for testing run
Glib::RefPtr<Gio::File>
Template::getResSrcPath(Glib::StdStringView execPath)
{
    auto resConfig = Gio::File::create_for_path(psc::util::Files::getSrcRelativeDir(execPath, PACKAGE_SRC_DIR));
    return resConfig;
}

Glib::RefPtr<Gio::File>
Template::getGlobalDataPath()
{
   auto globalConfig = Gio::File::create_for_path(PACKAGE_DATA_DIR);
   return globalConfig;
}

std::string
Template::loadTemplate(Glib::StdStringView execPath, const std::string& templateName)
{
    // this effort is done to run from source dir (without installed package data)
    auto resPath = getResSrcPath(execPath);
    auto templateFile = resPath->get_child(templateName);
    if (!templateFile->query_exists()) {
        auto globalPath = getGlobalDataPath();
         templateFile = globalPath->get_child(templateName);
        if (!templateFile->query_exists()) {
            std::cout << "Template::loadTemplate the file "<< templateName << " was not found in locations"
                      << " global " << globalPath->get_path()
                      << " res " << resPath->get_path()
                      << " maybe the used prefix for configure was wrong?" << std::endl;
            return "";
        }
    }
    char* content;
    gsize content_len;
    templateFile->load_contents(content, content_len);
    return std::string(content, content_len);
}
