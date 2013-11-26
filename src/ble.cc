// SPDX-License-Identifier: Apache-2.0
// Copyright 2018 Peter A. Bigot

#include <cctype>
#include <vector>

#include <pabigot/ble.hpp>

namespace {

/** Convert little-endian binary to big-endian hexadecimal text.
 *
 * @param sp pointer to the start of the little-endian sequence
 *
 * @param spe pointer to the end of the little-endian sequence
 *
 * @param dp pointer to a `char[]` region with at least `2 * (spe - sp)` octets
 * storage.
 *
 * @return pointer to the end of the decoded text representation. */
char*
append_hex (const uint8_t* sp,
            const uint8_t* spe,
            char* dp) noexcept
{
  // Implicitly reverse the order since we store little-endian but text
  // representation is big-endian.
  while (spe-- != sp) {
    unsigned int v = (*spe >> 4);
    *dp++ = static_cast<char>((v > 9) ? (v + 'a' - 10) : (v + '0'));
    v = (*spe & 0x0F);
    *dp++ = static_cast<char>((v > 9) ? (v + 'a' - 10) : (v + '0'));
  }
  return dp;
}

} // anonymous

namespace pabigot {
namespace ble {

const uuid128_type uuid128_type::BLUETOOTH_BASE{{
  0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00,
  0x00, 0x80,
  0x00, 0x10,
  0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
}};

uuid16_type::integer_type
uuid16_type::as_integer () const noexcept
{
  return details::to_integer<integer_type>(*this);
}

std::string
uuid16_type::as_string () const noexcept
{
  char buf[2 * byte_length + 1];
  auto dp = append_hex(data(), data() + byte_length, buf);
  *dp++ = 0;
  return {buf};
}

std::string
uuid32_type::as_string () const noexcept
{
  char buf[2 * byte_length + 1];
  auto dp = append_hex(data(), data() + byte_length, buf);
  *dp++ = 0;
  return {buf};
}

uuid128_type
uuid128_type::from_uuid16 (const uuid16_type& uuid16) const noexcept
{
  auto rv = *this;
  /* Bits 96..111 starting 12 octets into the little-endian representation. */
  std::copy(uuid16.begin(), uuid16.end(), rv.begin() + 12);
  return rv;
}

uuid16_type::integer_type
uuid128_type::uuid16 () const noexcept
{
  auto sp = begin() + 12;
  uuid16_type uuid16;
  std::copy(sp, sp + uuid16_type::byte_length, uuid16.begin());
  return uuid16.as_integer();
}

uuid128_type
uuid128_type::swap_endian () const noexcept
{
  auto rv = *this;
  // @todo for C++120 this can be constexpr and should move to header.
  std::reverse(rv.begin(), rv.end());
  return rv;
}

std::string
uuid128_type::as_string () const noexcept
{
  const uint8_t* sp = data();
  char buf[2 * byte_length + 4 + 1];
  auto dp = append_hex(sp + 12, sp + 16, buf);
  *dp++ = '-';
  dp = append_hex(sp + 10, sp + 12, dp);
  *dp++ = '-';
  dp = append_hex(sp + 8, sp + 10, dp);
  *dp++ = '-';
  dp = append_hex(sp + 6, sp + 8, dp);
  *dp++ = '-';
  dp = append_hex(sp + 0, sp + 6, dp);
  *dp++ = 0;
  return {buf};
}

bool
uuid128_type::base_match (const uuid128_type& other) const noexcept
{
  /* Match if the first 12 and last two octets match. */
  return (std::equal(begin(), begin() + 12, other.begin())
          && std::equal(begin() + 14, end(), other.begin() + 14));
}

} // ns ble
} // ns pabigot
