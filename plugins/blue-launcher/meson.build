project('blue-launcher', 'cpp',
        version: '0.1.0',
        default_options: ['cpp_std=c++17'])

# Znajdź zależności
wayfire_dep = dependency('wayfire', required: true)
wf_config_dep = dependency('wf-config', required: true)
cairo_dep = dependency('cairo', required: true)
glib_dep = dependency('glib-2.0', required: true)
gio_dep = dependency('gio-2.0', required: true)
json_dep = dependency('nlohmann_json', required: true)

# Źródła pluginu
blue_launcher_src = files(
  'blue_launcher.cpp',
  'blue_launcher_utils.cpp'
)

# Katalogi nagłówkowe
inc_dirs = include_directories('.')

# Buduj współdzieloną bibliotekę
shared_library(
  'blue_launcher',
  blue_launcher_src,
  include_directories: inc_dirs,
  dependencies: [wayfire_dep, wf_config_dep, cairo_dep, glib_dep, gio_dep, json_dep],
  install: true,
  install_dir: join_paths(get_option('prefix'), 'lib', 'wayfire')
)
