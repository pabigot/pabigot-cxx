# SPDX-License-Identifier: CC0-1.0
# Written in 2018 by Peter A. Bigot

ex_names = [
  'crc',
]
foreach base: ex_names
  ex = 'ex_' + base
  executable(base, base + '.cc',
             dependencies: [
               pabigot_dep,
             ])
endforeach
