# SPDX-License-Identifier: CC0-1.0
# Written in 2018 by Peter A. Bigot

common_hpp = configure_file(input: 'common.hpp.in',
                            output: 'common.hpp',
                            configuration: cdata)

headers = [
  'ble.hpp',
  'byteorder.hpp',
  common_hpp,
  'crc.hpp',
  'container.hpp',
]

subdir('external')
subdir('ble')

install_headers(headers,
                subdir: 'pabigot')
