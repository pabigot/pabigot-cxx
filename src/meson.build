# SPDX-License-Identifier: CC0-1.0
# Written in 2018 by Peter A. Bigot

pabigot_dependencies = []

pabigot_src = [
  'ble.cc',
  'ble-gap.cc',
]

if get_option('fullcpp')
  pabigot_lib = library('pabigot', pabigot_src,
                        include_directories: inc,
                        implicit_include_directories: false,
                        dependencies: pabigot_dependencies,
                        install: true,
                        soversion: '.'.join(version_components),
                       )
else
  pabigot_lib = static_library('pabigot', pabigot_src,
                               include_directories: inc,
                               dependencies: pabigot_dependencies,
                               implicit_include_directories: false,
                               install: true,
                               pic: false,
                              )
endif

pabigot_dep = declare_dependency(link_with: pabigot_lib,
                                 include_directories: inc,
                                 dependencies: pabigot_dependencies,
                                )
