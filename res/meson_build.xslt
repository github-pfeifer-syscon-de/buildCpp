<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="text" omit-xml-declaration="yes"/>

  <xsl:template match="/">
      <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="proj">
    <xsl:text>project('</xsl:text><xsl:value-of select="@name"/><xsl:text>', 'c', 'cpp'&#10;</xsl:text>
    <xsl:text>  , default_options: ['c_std=c17'&#10;</xsl:text>
    <xsl:text>  , 'cpp_std=c++20'&#10;</xsl:text>
    <xsl:text>  , 'buildtype=debugoptimized'&#10;</xsl:text>
    <xsl:text>  , 'warning_level=3']&#10;</xsl:text>
    <xsl:text>  , license : 'GPL-3.0'&#10;</xsl:text>
    <xsl:text>  , meson_version: '&gt;=1.3.0'&#10;</xsl:text>
    <xsl:text>  , version: '0.1')&#10;</xsl:text>
    <xsl:text>  &#10;</xsl:text>
    <xsl:text>  fs = import('fs')&#10;</xsl:text>
    <xsl:text>  &#10;</xsl:text>
    <xsl:text>  pkgdatadir = join_paths(get_option('prefix'), get_option('datadir'), meson.project_name())&#10;</xsl:text>
    <xsl:text>  pkglocaledir = join_paths(get_option('prefix'), get_option('datadir'), 'locale')&#10;</xsl:text>
    <xsl:text>  pkgsrcdir = fs.relative_to( meson.global_source_root(), meson.current_build_dir()) + '/src'&#10;</xsl:text>
    <xsl:text>  add_project_arguments('-DPACKAGE_DATA_DIR="' + pkgdatadir + '"', language: ['cpp','c'])&#10;</xsl:text>
    <xsl:text>  add_project_arguments('-DPACKAGE="' + meson.project_name() + '"', language: ['cpp','c'])&#10;</xsl:text>
    <xsl:text>  # this is to recreate the conventions from autotools&#10;</xsl:text>
    <xsl:text>  add_project_arguments('-DPACKAGE_SRC_DIR="' + pkgsrcdir + '"', language: ['cpp','c'])&#10;</xsl:text>
    <xsl:text>  # add_project_arguments('-DGETTEXT_PACKAGE="' + meson.project_name() + '"', language: ['c'])&#10;</xsl:text>
    <xsl:text>  # add_project_arguments('-DPACKAGE_LOCALE_DIR="' + pkglocaledir + '"', language: ['cpp','c'])&#10;</xsl:text>
    <xsl:text>  &#10;</xsl:text>
    <xsl:text>  &#10;</xsl:text>
    <xsl:text>  compile_flags = [&#10;</xsl:text>
    <xsl:text>  '-Wno-unused-parameter'         # limit the compiler noisiness&#10;</xsl:text>
    <xsl:text>  ,'-Wno-overlength-strings'&#10;</xsl:text>
    <xsl:text>  ,'-Wconversion'                 # these can be helpful&#10;</xsl:text>
    <xsl:text>  ]&#10;</xsl:text>
    <xsl:text>  cc = meson.get_compiler('c')&#10;</xsl:text>
    <xsl:text>  foreach cflag : compile_flags&#10;</xsl:text>
    <xsl:text>  if cc.has_argument(cflag)&#10;</xsl:text>
    <xsl:text>  add_project_arguments(cflag, language: ['c', 'cpp'])&#10;</xsl:text>
    <xsl:text>  endif&#10;</xsl:text>
    <xsl:text>  endforeach&#10;</xsl:text>
    <xsl:text>  &#10;</xsl:text>
    <xsl:text>  conf = configuration_data()&#10;</xsl:text>
    <xsl:text>  conf.set_quoted('BUILD_VERSION', meson.project_version())     # Surround the version in quotes to make it a C string&#10;</xsl:text>
    <xsl:text>  conf.set_quoted('BUILD_PACKAGE', meson.project_name())     # Surround the version in quotes to make it a C string&#10;</xsl:text>
    <xsl:text>  conf_file = configure_file(output : meson.project_name() + '_config.h'&#10;</xsl:text>
    <xsl:text>  , configuration : conf)&#10;</xsl:text>
    <xsl:text>  &#10;</xsl:text>
    <xsl:text>  &#10;</xsl:text>
    <xsl:text>  gtkmm3_deps     = dependency('gtkmm-3.0')&#10;</xsl:text>
    <xsl:text>  glibmm2_deps    = dependency('glibmm-2.4 giomm-2.4')&#10;</xsl:text>
    <xsl:text>  thread_deps     = dependency('threads')&#10;</xsl:text>
    <xsl:text>  genericimg_deps = dependency('genericimg', version :'&gt;= 0.4.13')&#10;</xsl:text>
    <xsl:text>  deps = [gtkmm3_deps&#10;</xsl:text>
    <xsl:text>  , glibmm2_deps&#10;</xsl:text>
    <xsl:text>  , thread_deps&#10;</xsl:text>
    <xsl:text>  , genericimg_deps&#10;</xsl:text>
    <xsl:text>  ]&#10;</xsl:text>
    <xsl:text>  subdir('src')&#10;</xsl:text>
    <xsl:text>  subdir('res')&#10;</xsl:text>
    <xsl:text>  sources += app_resources&#10;</xsl:text>
    <xsl:text>  sources += win_res_file&#10;</xsl:text>
    <xsl:text>  &#10;</xsl:text>
    <xsl:text>  project_targt = executable(meson.project_name()&#10;</xsl:text>
    <xsl:text>  , sources&#10;</xsl:text>
    <xsl:text>  , dependencies: deps&#10;</xsl:text>
    <xsl:text>  , install: true&#10;</xsl:text>
    <xsl:text>  , link_with: []&#10;</xsl:text>
    <xsl:text>  , win_subsystem: 'windows'  # disable this to see console output for windows&#10;</xsl:text>
    <xsl:text>  )&#10;</xsl:text>
  </xsl:template>
</xsl:stylesheet>