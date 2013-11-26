# SPDX-License-Identifier: CC0-1.0
# Written in 2018 by Peter A. Bigot

cdata.set('TOP_SRCDIR', meson.source_root())
cdata.set('TOP_BUILDDIR', meson.build_root())

datadir = join_paths(get_option('datadir'), 'doc')
cdata.set('DOCDIR', 'doc')

if doxygen.found() and get_option('support')
  doxycfg = configure_file(input: 'doxygen.cfg.in',
                           output: 'doxygen.cfg',
                           configuration: cdata,
                           install: false)

  html_target = custom_target('docs',
                              input: doxycfg,
                              output: 'html',
                              command: [doxygen, doxycfg],
                              install: true,
                              install_dir: datadir)
endif
