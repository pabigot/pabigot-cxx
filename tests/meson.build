# SPDX-License-Identifier: CC0-1.0
# Written in 2018 by Peter A. Bigot

test_names = [
  'ble',
  'ble-gap',
  'byteorder',
  'container',
  'crc',
]

mock_names = [
]

foreach base: test_names
  gt = 'gt_' + base
  test(base, executable(gt, gt + '.cc',
                        dependencies: [
                          gtest_dep,
                          pabigot_dep,
                        ]))
endforeach
foreach base: mock_names
  gt = 'gt_' + base
  test(base, executable(gt, gt + '.cc',
                        dependencies: [
                          gtest_dep,
                          gmock_dep,
                          pabigot_dep,
                        ]))
endforeach
