// SPDX-License-Identifier: CC-BY-SA-4.0
// Copyright 2018-2019 Peter A. Bigot

/* Display the Rocksoft^TM standard polynomial description
 * along with the CRC table. */

#include <cstdio>
#include <cinttypes>

#include <pabigot/crc.hpp>

#if 0
static const char* const crc_tag = "CRC-8/DOW"; // CRC-MAXIM
using crc_type = pabigot::crc::crc<8, 0x31, true, true, 0, 0>;
#define FMT "%02" PRIx8
#elif 1
static const char* const crc_tag = "CRC-8/SMBUS"; // CRC-8
using crc_type = pabigot::crc::crc<8, 0x7, false, false, 0, 0>;
#define FMT "%02" PRIx8
#elif 0
static const char* const crc_tag = "CRC-16/DNP";
using crc_type = pabigot::crc::crc<16, 0x3d65, true, true, 0, -1>;
#define FMT "%04" PRIx16
#elif 0
static const char* const crc_tag = "CRC-16/EN-13757";
using crc_type = pabigot::crc::crc<16, 0x3d65, false, false, 0, -1>;
#define FMT "%04" PRIx16
#elif 0
static const char* const crc_tag = "XMODEM";
using crc_type = pabigot::crc::crc<16, 0x1021, false, false, 0, 0>;
#define FMT "%04" PRIx16
#elif 0
static const char* const crc_tag = "CRC-24";
using crc_type = pabigot::crc::crc<24, 0x864CFB, false, false, 0xB704CE, 0>;
#define FMT "%06" PRIx32
#elif 0
static const char* const crc_tag = "CRC-24/BLE";
using crc_type = pabigot::crc::crc<24, 0x00065B, true, true, 0x555555, 0>;
#define FMT "%06" PRIx32
#elif 0
static const char* const crc_tag = "CRC-32/BZIP2";
using crc_type = pabigot::crc::crc<32, 0x04C11DB7, false, false, -1, -1>;
#define FMT "%08" PRIx32
#elif 0
static const char* const crc_tag = "CRC-32/POSIX";
using crc_type = pabigot::crc::crc<32, 0x04C11DB7, false, false, 0, -1>;
#define FMT "%08" PRIx32
#elif 1
static const char* const crc_tag = "CRC-32";
using crc_type = pabigot::crc::crc<32, 0x04C11DB7, true, true, -1, -1>;
#define FMT "%08" PRIx32
#else
static const char* const crc_tag = "CRC-64";
using crc_type = pabigot::crc::crc<64, 0x42f0e1eba9ea3693, false, false, 0, 0>;
#define FMT "%016" PRIx64
#endif

constexpr auto crc = crc_type::instantiate_tabler();

int
main ()
{
  static const char check[] = "123456789";
  static const char* const bool_str[] = {"false", "true"};
  printf("%s"
         " width=%u"
         " poly=0x" FMT
         " init=0x" FMT "\n"
         " refin=%s"
         " refout=%s"
         " xorout=0x" FMT "\n"
         " check=0x" FMT
         " residue=0x" FMT
         "\n",
         crc_tag,
         crc_type::width,
         static_cast<crc_type::least_type>(crc_type::poly),
         static_cast<crc_type::least_type>(crc_type::init),
         bool_str[crc_type::refin],
         bool_str[crc_type::refout],
         static_cast<crc_type::least_type>(crc_type::xorout),
         crc.finalize(crc.append(check, check + sizeof(check) - 1)),
         static_cast<crc_type::least_type>(crc_type::xorout ^ crc_type::residue()));

  unsigned int idx_mask = 7;
  if (32 <= crc_type::width) {
    idx_mask = 3;
  }
  int idx = 0;
  while (idx <= 256) {
    if ((0 < idx) && (0 == (idx & idx_mask))) {
      printf("  /* %3u .. %u */\n", (idx - idx_mask - 1), idx - 1);
    }
    if (idx < 256) {
      printf(" 0x" FMT ",", crc.table[idx]);
    }
    ++idx;
  }

  return EXIT_SUCCESS;
}
