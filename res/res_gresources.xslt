<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="xml" encoding="UTF-8" indent="yes" />

  <xsl:template match="/">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="proj">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="res">
  <gresources>
      <gresource>
        <xsl:attribute name="prefix">
          <xsl:value-of select="@prefix"/>
        </xsl:attribute>
        <!-- file preprocess="xml-stripblanks">app-menu.ui</file -->
        <file preprocess="xml-stripblanks">build-win.ui</file>
        <file>buildcpp.png</file>
      </gresource>
    </gresources>
  </xsl:template>
</xsl:stylesheet>