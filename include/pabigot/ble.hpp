/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2017-2019 Peter A. Bigot */

/** Infrastructure supporting Bluetooth Low Energy
 *
 * @file */
#ifndef PABIGOT_BLE_HPP
#define PABIGOT_BLE_HPP
#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <string>

#include <pabigot/byteorder.hpp>

namespace pabigot {

/** Various helpers for Bluetooth Low Energy activities.
 *
 * @note This material in this namespace supports both both Nordic Soft Device
 * APIs and Bluez.  Without @link PABIGOT_OPTION_FULLCPP full C++
 * support@endlink some features are absent. */
namespace ble {

namespace details {

/** Template for durations corresponding to bits in the 3.2 kHz
 * Bluetooth native clock. */
template <unsigned int clock_bit>
using clk_type = std::chrono::duration<uint32_t, std::ratio<(1U << clock_bit), 3200>>;

/** Base class that captures 2, 4, or 16-byte UUIDs.
 *
 * The endianness of the captured value is not specified by this type.
 * However, the standard representation over-the-air is little-endian, and
 * that's what's used when UUIDs are stored as uuid16_type, uuid32_type, or
 * uuid128_type values. */
template <std::size_t nb>
class uuid_type : public std::array<uint8_t, nb>
{
public:
  using super_type = std::array<uint8_t, nb>;

  /** The length of the UUID in bytes. */
  static constexpr std::size_t byte_length = nb;

  /** The length of the UUID in bits. */
  static constexpr std::size_t bit_length = 8 * byte_length;

  // We don't inherit the base class constructors because gcc through 7.3
  // doesn't handle them correctly.  Something about the default constructor
  // not working.
  //
  // See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=83921

  constexpr uuid_type () :
    super_type{}
  { }

  constexpr uuid_type (const super_type& arr) :
    super_type{arr}
  { }

  constexpr uuid_type (super_type& arr) :
    super_type{arr}
  { }

  constexpr uuid_type (const uint8_t (&arr)[byte_length]) :
    super_type{}
  {
    std::copy(arr, byte_length + arr, this->begin());
  }

  constexpr uuid_type (uint8_t (&arr)[byte_length]) :
    super_type{}
  {
    std::copy(arr, byte_length + arr, this->begin());
  }
};

/** Convert a small UUID to its native integer type.
 *
 * This handles any necessary host endian conversion from the little-endian
 * standard storage. */
template <typename Int>
constexpr Int
to_integer (const uuid_type<sizeof(Int)>& uuid) noexcept
{
  using integer_type = Int;
  union { // @todo c++20 use std::byte
    integer_type integer;
    uint8_t u8;
  } u{};
  std::copy(uuid.begin(), uuid.end(), &u.u8);
  return byteorder::host_x_le(u.integer);
}
} // namespace details

/** A duration type that measures in 625 us ticks.
 *
 * This is the duration of the Bluetooth piconet physical channel time
 * slot, or CLK_1 of the 3.2 kHz Bluetooth clock.
 *
 * Intervals such as the minimum and maximum advertising interval in
 * the HCI `LE_Set_Advertising_Parameters` command are measured in
 * this duration. */
using clk1_type = details::clk_type<1>;

/** A duration type that measures in 1.25 ms ticks.
 *
 * This is the duration of CLK_2 of the 3.2 kHz Bluetooth clock.
 *
 * Intervals such as the minimum and maximum connection interval in
 * the GAP Peripheral Preferred Connection Parameters (PPCP)
 * characteristic are measured in this duration. */
using clk2_type = details::clk_type<2>;

/** A duration type that measures in 10 ms ticks.
 *
 * This is the duration of CLK_5 of the 3.2 kHz Bluetooth clock.
 *
 * Intervals such as the connection superviser timeout in the GAP
 * Peripheral Preferred Connection Parameters (PPCP) characteristic
 * are measured in this duration. */
using clk5_type = details::clk_type<5>;

/** 16-bit UUID type stored as a little-endian byte sequence. */
class uuid16_type : public details::uuid_type<2>
{
  using super_type = details::uuid_type<2>;

public:
  /** Native integer type for 16-bit UUID. */
  using integer_type = uint16_t;

  // Expose base class constructors
  using super_type::super_type;

  /** Construct an instance from a native integral UUID. */
  explicit constexpr uuid16_type (integer_type src) :
    super_type{}
  {
    union { // @todo c++20 use std::byte
      integer_type integer;
      uint8_t u8;
    } u{byteorder::host_x_le(src)};
    std::copy(&u.u8, super_type::byte_length + &u.u8, begin());
  }

  /** Convert the UUID to its host integral representation. */
  integer_type as_integer () const noexcept;

  /** Convert the UUID to its standard text representation.
   *
   * @note Standard representation is 4 lower-case xdigits in big-endian byte
   * order. */
  std::string as_string () const noexcept;
};

/** 32-bit UUID type stored as a little-endian byte sequence. */
class uuid32_type : public details::uuid_type<4>
{
  using super_type = details::uuid_type<4>;

public:
  /** Native integer type for 32-bit UUID. */
  using integer_type = uint32_t;

  // Expose base class constructors
  using super_type::super_type;

  /** Construct an instance from a native integral UUID. */
  explicit constexpr uuid32_type (integer_type src) :
    super_type{}
  {
    union { // @todo c++20 use std::byte
      integer_type integer;
      uint8_t u8;
    } u{byteorder::host_x_le(src)};
    std::copy(&u.u8, super_type::byte_length + &u.u8, begin());
  }

  /** Convert the UUID to its host integral representation. */
  integer_type as_integer () const noexcept
  {
    return details::to_integer<integer_type>(*this);
  }

  /** Convert the UUID to its standard text representation.
   *
   * @note Standard representation is 8 lower-case xdigits in big-endian byte
   * order. */
  std::string as_string () const noexcept;
};

/** Basic holder for 128-bit UUIDs stored as a little-endian byte sequence.
 *
 * To generate an initializer for a 128-bit UUID use:
 *
 *     UUID=${UUID:-$(uuidgen)}
 *     # Optionally clear bits [96, 112) to set as base for 16-bit UUIDs
 *     # UUID=$(echo ${UUID} | sed -r -e 's@^(....)....@\10000@')
 *     echo "// UUID: ${UUID}"
 *     echo ${UUID} \
 *       | sed -r \
 *           -e 's@-@@g' \
 *           -e 's@(..)@0x&,\n@g' \
 *       | sed -e '/^$/d' \
 *       | tac \
 *       | paste -d' ' - - - -
 *     unset UUID
 */
class uuid128_type : public details::uuid_type<16>
{
  using super_type = details::uuid_type<16>;

public:
  /** The Bluetooth Base UUID.
   *
   * I.e.: 00000000-0000-1000-8000-00805F9B34FB */
  static const uuid128_type BLUETOOTH_BASE;

  // Expose base class constructors
  using super_type::super_type;

  /** Convert the UUID to its standard text representation.
   *
   * @note Standard text representation is the canonical 8-4-4-4-12 lower-case
   * text of the big-endian byte order as in [RFC
   * 4122](https://tools.ietf.org/html/rfc4122#section-3). */
  std::string as_string () const noexcept;

  /** Indicate whether the two UUIDs have the same base UUID.
   *
   * Base UUIDs are assumed to be for 16-bit UUIDs, meaning this succeeds if
   * the UUIDs are equal in all but bits [96, 112). */
  bool base_match (const uuid128_type& other) const noexcept;

  /** Construct a derived 128-bit UUID from a 16-bit UUID.
   *
   * This returns a new 128-bit UUID equal to this UUID but with bits [96, 112)
   * replaced by the contents of @p uuid16. */
  uuid128_type from_uuid16 (const uuid16_type& uuid16) const noexcept;

  /** Construct a derived 128-bit UUID from a 16-bit UUID integral value.
   *
   * This returns a new 128-bit UUID equal to this UUID but with bits [96, 112)
   * replaced by the contents of @p uuid16 in little-endian byte order. */
  uuid128_type from_uuid16 (uint16_t uuid16) const noexcept
  {
    return from_uuid16(uuid16_type{uuid16});
  }

  /** Extract the 16-bit UUID as a host byte order integer. */
  uuid16_type::integer_type uuid16 () const noexcept;

  /** Byte-reverse the UUID, converting between big- and little-endian
   * representations.
   *
   * @note This operation reverses the entire 16-byte sequence, to meet the
   * expectations of the Nordic Soft Device API.  Unlike Microsoft the
   * endianness is not isolated within UUID data elements. */
  uuid128_type swap_endian () const noexcept;
};

} // namespace ble
} // namespace pabigot

#if (PABIGOT_OPTION_FULLCPP - 0)
#include <pabigot/ble/fullcpp.hpp>
#endif /* PABIGOT_OPTION_FULLCPP */

#endif /* PABIGOT_BLE_HPP */
