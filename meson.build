# SPDX-License-Identifier: CC0-1.0
# Written in 2018-2019 by Peter A. Bigot

project('pabigot', ['cpp'],
        version: '0.1.0',
        default_options: [
          'cpp_std=c++17',
          'werror=true',
        ])
cdata = configuration_data()
cdata.set('PROJECT', meson.project_name())
cdata.set('PROJECT_UC', meson.project_name().to_upper())
version_split = meson.project_version().split('-')
cdata.set('VERSION', meson.project_version())
version_components = version_split[0].split('.')
cdata.set('VERSION_MAJOR', version_components[0])
cdata.set('VERSION_MINOR', version_components[1])
cdata.set('VERSION_PATCH', version_components[2])
if version_split.length() == 1
  cdata.set('VERSION_RELEASE', 'RELEASE')
  cdata.set('VERSION_DEVEL', 1)
else
  version_dev = version_split[1].split('.')
  if version_dev[0] != 'dev'
    error('In the project version attribute: @0@ should be dev'.format(version_dev[0]))
  endif
  cdata.set('VERSION_RELEASE', 'DEVEL')
  cdata.set('VERSION_DEVEL', version_dev[1])
  version_components += version_dev[1]
endif

cdata.set('OPTION_FULLCPP', 0)

if get_option('support')
  doxygen = find_program('doxygen', required: false)
  if find_program('dot', required: false).found()
    cdata.set('HAVE_DOT', 'YES')
  else
    cdata.set('HAVE_DOT', 'NO')
  endif
else
  doxygen = disabler()
endif

if get_option('googletest')
  gtest_sp = subproject('gtest')
  gtest_dep = gtest_sp.get_variable('gtest_main_dep')
  gmock_dep = gtest_sp.get_variable('gmock_dep')
else
  gtest_dep = disabler()
  gmock_dep = disabler()
endif

if doxygen.found()
  subdir('doc')
endif

pkg = import('pkgconfig')

subdir('include')
subdir('src')
if get_option('support')
  subdir('examples')
endif
subdir('tests')

pkg.generate(libraries: pabigot_lib,
             version: meson.project_version(),
             name: 'libpabigot',
             filebase: 'pabigot',
             description: 'Collected C++ utilities for me')
