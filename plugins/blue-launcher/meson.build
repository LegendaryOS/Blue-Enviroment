project('blue-launcher', 'cpp')

# Znajdź zależności
wayfire_dep = dependency('wayfire', required: true)
wf_config_dep = dependency('wf-config', required: true)
cairo_dep = dependency('cairo', required: true)
glib_dep = dependency('glib-2.0', required: true)
gio_dep = dependency('gio-2.0', required: true)

# Źródła pluginu
blue_launcher_src = files('blue-launcher.cpp')

# Buduj współdzieloną bibliotekę
shared_library(
  'blue_launcher',
  blue_launcher_src,
  include_directories: include_directories('..'),
  dependencies: [wayfire_dep, wf_config_dep, cairo_dep, glib_dep, gio_dep],
  install: true,
  install_dir: join_paths(get_option('prefix'), 'lib', 'wayfire')
)
