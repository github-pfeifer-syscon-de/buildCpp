project('buildcpp', 'c', 'cpp'
    , default_options: ['c_std=c17'
                       , 'cpp_std=c++20'
                       , 'buildtype=debugoptimized'
                       , 'warning_level=3']
    , license : 'GPL-3.0'
    , meson_version: '>=1.3.0'
    , version: '0.1')

fs = import('fs')

pkgdatadir = join_paths(get_option('prefix'), get_option('datadir'), meson.project_name())
pkglocaledir = join_paths(get_option('prefix'), get_option('datadir'), 'locale')
pkgsrcdir = fs.relative_to( meson.global_source_root(), meson.current_build_dir()) + '/src'
add_project_arguments('-DPACKAGE_DATA_DIR="' + pkgdatadir + '"', language: ['cpp','c'])
add_project_arguments('-DPACKAGE="' + meson.project_name() + '"', language: ['cpp','c'])
# this is to recreate the conventions from autotools
add_project_arguments('-DPACKAGE_SRC_DIR="' + pkgsrcdir + '"', language: ['cpp','c'])
# add_project_arguments('-DGETTEXT_PACKAGE="' + meson.project_name() + '"', language: ['c'])
# add_project_arguments('-DPACKAGE_LOCALE_DIR="' + pkglocaledir + '"', language: ['cpp','c'])


compile_flags = [
    '-Wno-unused-parameter'         # limit the compiler noisiness
    ,'-Wno-overlength-strings'
    ,'-Wconversion'                 # these can be helpful
]
cc = meson.get_compiler('c')
foreach cflag : compile_flags
    if cc.has_argument(cflag)
        add_project_arguments(cflag, language: ['c', 'cpp'])
    endif
endforeach

conf = configuration_data()
conf.set_quoted('BUILD_VERSION', meson.project_version())     # Surround the version in quotes to make it a C string
conf.set_quoted('BUILD_PACKAGE', meson.project_name())
conf_file = configure_file(output : meson.project_name() + '_config.h'
               , configuration : conf)


gtkmm3_deps     = dependency('gtkmm-3.0')
glibmm2_deps    = dependency('glibmm-2.4 giomm-2.4')
thread_deps     = dependency('threads')
genericimg_deps = dependency('genericimg', version :'>= 0.4.13')
#openssl_deps    = dependency('openssl')
deps = [gtkmm3_deps
    , glibmm2_deps
    , thread_deps
    , genericimg_deps
    ]
subdir('src')
subdir('res')
sources += app_resources
sources += win_res_file

project_targt = executable(meson.project_name()
    , sources
    , dependencies: deps
    , install: true
    , link_with: []
    , win_subsystem: 'windows'  # disable this to see console output for windows
    )
