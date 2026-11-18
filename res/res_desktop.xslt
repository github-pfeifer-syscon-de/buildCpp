<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="text" omit-xml-declaration="yes"/>

  <xsl:template match="/">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="proj">
    <xsl:apply-templates select="res">
      <xsl:with-param name="proj_name">
        <xsl:value-of select="@name"/>
      </xsl:with-param>
      <xsl:with-param name="proj_name_lower">
        <xsl:value-of select="@name_lower"/>
      </xsl:with-param>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="res">
    <xsl:param name="proj_name_lower"/>
    <xsl:text>[Desktop Entry]&#10;</xsl:text>
    <xsl:text>Type=Application&#10;</xsl:text>
    <xsl:text>Encoding=UTF-8&#10;</xsl:text>
    <xsl:text>Version=0.1&#10;</xsl:text>
    <xsl:text>Name=Build&#10;</xsl:text>
    <xsl:text>Comment=Build utility&#10;</xsl:text>
    <xsl:text>Categories=Application;Utility;&#10;</xsl:text>
    <xsl:text>Exec=</xsl:text><xsl:value-of select="$proj_name_lower"/><xsl:text>&#10;</xsl:text>
    <xsl:text>Icon=</xsl:text><xsl:value-of select="$proj_name_lower"/><xsl:text>&#10;</xsl:text>
  </xsl:template>
</xsl:stylesheet>