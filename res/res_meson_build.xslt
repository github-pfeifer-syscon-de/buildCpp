<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="text" omit-xml-declaration="yes"/>

  <xsl:template match="/">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="proj">
    <xsl:apply-templates select="res"/>
  </xsl:template>

  <xsl:template match="res">
    <xsl:text># see https://mesonbuild.com/Gnome-module.html&#10;</xsl:text>
    <xsl:text>  gnome = import('gnome')&#10;</xsl:text>
    <xsl:text>  app_resources = gnome.compile_resources(&#10;</xsl:text>
    <xsl:text>  'resources', 'gresources.xml'&#10;</xsl:text>
    <xsl:text>  , source_dir: meson.current_source_dir()&#10;</xsl:text>
    <xsl:text>  )&#10;</xsl:text>
    <xsl:text>  &#10;</xsl:text>
    <xsl:text>  # icons + desktop&#10;</xsl:text>
    <xsl:text>  dataDir = join_paths(get_option('prefix'), get_option('datadir'))&#10;</xsl:text>
    <xsl:text>  iconDir = join_paths(dataDir, 'icons/hicolor')&#10;</xsl:text>
    <xsl:text>  install_data(&#10;</xsl:text>
    <xsl:text>  meson.project_name() + '.png'&#10;</xsl:text>
    <xsl:text>  , install_dir: join_paths(iconDir, '64x64/apps'))&#10;</xsl:text>
    <xsl:text>  install_data(&#10;</xsl:text>
    <xsl:text>  meson.project_name() + '.svg'&#10;</xsl:text>
    <xsl:text>  , install_dir: join_paths(iconDir, 'scalable/apps'))&#10;</xsl:text>
    <xsl:text>  install_data(&#10;</xsl:text>
    <xsl:text>  meson.project_name() + '.desktop'&#10;</xsl:text>
    <xsl:text>  , install_dir: join_paths(dataDir, 'applications'))&#10;</xsl:text>
    <xsl:text>  &#10;</xsl:text>
    <xsl:text>  win_res_file = []&#10;</xsl:text>
    <xsl:text>  if target_machine.system() == 'windows'     # expects resource compiler, disable if you don't have one (part of msys2 but not claimed by any package... )&#10;</xsl:text>
    <xsl:text>  windows = import('windows')&#10;</xsl:text>
    <xsl:text>  win_res_file = windows.compile_resources('win.rc'&#10;</xsl:text>
    <xsl:text>  ,args: []&#10;</xsl:text>
    <xsl:text>  ,depend_files: [meson.project_name() + '.ico']&#10;</xsl:text>
    <xsl:text>)&#10;</xsl:text>
  </xsl:template>
</xsl:stylesheet>