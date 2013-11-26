/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2014-2019 Peter A. Bigot */

/** Support for byte order manipulation and packed storage.
 *
 * @file
 */

#ifndef PABIGOT_BYTEORDER_HPP
#define PABIGOT_BYTEORDER_HPP
#pragma once

#include <algorithm>
#include <cinttypes>
#include <climits>
#include <cstring>
#include <type_traits>

#include <pabigot/common.hpp>
#include <pabigot/external/ccbysa30.hpp>

namespace pabigot {

/** Functionality related to byte order and endianness */
namespace byteorder {

/** Enumeration of constants representing various known byte orders.
 *
 * The constants are the host interpretation of a 4-byte sequence {4,
 * 3, 2, 1} as a 32-bit unsigned integer. */
enum class byte_order_enum : uint32_t
{
  /** Representation for a host using little-endian order */
  little_endian = 0x01020304U,

  /** Representation for a host using big-endian order */
  big_endian = 0x04030201U,

  /** Representation for network byte order, which is big-endian */
  network = big_endian,

  /** Representation for PDP byte order.
   *
   * This is identified for completeness but will almost certainly
   * fail to work correctly and isn't really supported.  Use it as a
   * sign that byte order could not be identified. */
  pdp_endian = 0x03040102U,
};

/** Return the #byte_order_enum value for the host.
 *
 * The implementation of this relies on compiler support, as there is no
 * portable C++17 solution to determining runtime byte order at
 * compile-time. */
constexpr byte_order_enum
host_byte_order ()
{
  /* The following test depends on GCC pre-defined macros, because the
   * alternative approach below violates strict aliasing in a way that
   * prevents it from being used in a constexpr function:
   *
   *     using t = union {
   *       char c[4];
   *       byte_order_enum bo;
   *     };
   *     return t{{4, 3, 2, 1}}.bo;
   */
  return (__ORDER_LITTLE_ENDIAN__ == __BYTE_ORDER__) ? byte_order_enum::little_endian
    : (__ORDER_BIG_ENDIAN__ == __BYTE_ORDER__) ? byte_order_enum::big_endian
    : byte_order_enum::pdp_endian;
}

/** The Unicode byte order marker. */
static constexpr wchar_t BOM = u'\uFFFE';

namespace details {

/** Return a byte-swapped representation of an integral value.
 *
 * @tparam T any type that can be converted to an unsigned representation.
 *
 * @param v the host representation of a value of type @p T.
 *
 * @return The byte-swapped unsigned value of @p v. */
template<class T,
         class U = typename std::make_unsigned<T>::type>
constexpr U
byteswap (T v)
{
  return external::ccbysa30::bswap<T>(v);
}

/** Category identifying types that can be constexpr swapped.
 *
 * This is a variable template that identifies types for which byte swapping
 * can be performed with a constant expression.
 *
 * Specifically, these are the types that can be converted through
 * `static_cast` to and from the unsigned integer type of the same size without
 * changing the value. */
template <class T>
constexpr bool
is_constexpr_swappable_v{std::is_integral<T>::value};

/** Category identifying types that can swapped by alias with a byte
 * array.
 *
 * This is the fallback for things that fail is_constexpr_swappable_v().  The
 * process used is to alias a copy of the value with a `uint8_t` array,
 * byte-swap the array, then read the value out of the inactive tag.
 *
 * @note Technically the algorithm underlying this supports reversing any
 * contiguous area of memory.  However, byte-reversing arbitrary structures
 * makes no sense, so this solution is limited to scalar values.  Containers of
 * octet data, for which reversal does make sense, are handled by a different
 * solution. */
template <class T>
constexpr bool
is_alias_swappable_v{std::is_scalar_v<T>
                     && !is_constexpr_swappable_v<T>};


/** Category identifying types that can be swapped by using `std::reverse` on a
 * copy.
 *
 * These are container classes with a bi-directional iterator and octet-sized
 * values. */
template <class T>
constexpr bool
is_other_swappable_v{!is_constexpr_swappable_v<T>
                     && !is_alias_swappable_v<T>};

} // ns details

/** Byte-swap values at compile-time.
 *
 * @note This `constexpr` overload is only available for types where a
 * compile-time byte swap is supported, i.e. integral scalars. */
template <typename T>
constexpr typename std::enable_if<details::is_constexpr_swappable_v<T>, T>::type
byteswap (const T& t)
{
  return details::byteswap(t);
}

/** Byte-swap values by aliasing a copy of the value to a uint8_t
 * sequence. */
template <typename T>
typename std::enable_if<details::is_alias_swappable_v<T>, T>::type
byteswap (const T& t)
{
  union { // @todo c++20 use std::byte
    T value;
    uint8_t raw[sizeof(t)];
  } u = {t};
  std::reverse(u.raw, u.raw + sizeof(u.raw));
  return u.value;
}

/** Byte-swap values by creating a copy and using `std::reverse`.
 *
 * @note This is really intended only for things like `std::array<uint8_t, 6>`.
 * It happens to work on `std::vector<uint8_t>`, but it'll fail to compile with
 * some types, including ones acceptable to details::is_other_swappable_v(). */
template <typename T>
typename std::enable_if<details::is_other_swappable_v<T>, T>::type
byteswap (const T& t)
{
  static_assert(1 == sizeof(*t.begin()), "cannot byte-swap sequences with non-octet values");
  auto value = t;
  std::reverse(value.begin(), value.end());
  return value;
}

/** constexpr-selected byte swap between host and endian.
 *
 * This variant is available when host uses the desired endian and constexpr
 * byte swap is supported for the type. */
template <typename T,
          byte_order_enum endian>
constexpr typename std::enable_if<details::is_constexpr_swappable_v<T>
                                  && (host_byte_order() == endian), T>::type
hostswap (const T& t)
{
  return t;
}

/** constexpr-selected byte swap between host and endian.
 *
 * This variant is available when host uses the desired endian and constexpr
 * byte swap is not supported for the type. */
template <typename T,
          byte_order_enum endian>
typename std::enable_if<!details::is_constexpr_swappable_v<T>
                        && (host_byte_order() == endian), T>::type
hostswap (const T& t)
{
  return t;
}

/** constexpr-selected byte swap between host and endian.
 *
 * This variant is available when host does not use the desired endian and
 * constexpr byte swap is supported for the type. */
template <typename T,
          byte_order_enum endian>
constexpr typename std::enable_if<details::is_constexpr_swappable_v<T>
                                  && (host_byte_order() != endian), T>::type
hostswap (const T& t)
{
  return byteswap(t);
}

/** constexpr-selected byte swap between host and endian.
 *
 * This variant is available when host does not use the desired endian and
 * constexpr byte swap is not supported for the type. */
template <typename T,
          byte_order_enum endian>
typename std::enable_if<!details::is_constexpr_swappable_v<T>
                        && (host_byte_order() != endian), T>::type
hostswap (const T& t)
{
  return byteswap(t);
}

/** Convert between host and little-endian byte order.
 *
 * @note This `constexpr` overload is only available for types where a
 * constexpr byte swap is supported, i.e. integral scalars. */
template <typename T>
constexpr typename std::enable_if<details::is_constexpr_swappable_v<T>, T>::type
host_x_le (const T& v)
{
  return hostswap<T, byte_order_enum::little_endian>(v);
}

/** Convert between host and little-endian byte order. */
template <typename T>
typename std::enable_if<!details::is_constexpr_swappable_v<T>, T>::type
host_x_le (const T& v)
{
  return hostswap<T, byte_order_enum::little_endian>(v);
}

/** Convert between host and big-endian byte order.
 *
 * @note This `constexpr` overload is only available for types where a
 * constexpr byte swap is supported, i.e. integral scalars. */
template <typename T>
constexpr typename std::enable_if<details::is_constexpr_swappable_v<T>, T>::type
host_x_be (const T& v)
{
  return hostswap<T, byte_order_enum::big_endian>(v);
}

/** Convert between host and big-endian byte order. */
template <typename T>
typename std::enable_if<!details::is_constexpr_swappable_v<T>, T>::type
host_x_be (const T& v)
{
  return hostswap<T, byte_order_enum::big_endian>(v);
}

/** Convert between big-endian and little-endian byte order.
 *
 * @note This `constexpr` overload is only available for types where a
 * constexpr byte swap is supported, i.e. integral scalars. */
template <typename T>
constexpr typename std::enable_if<details::is_constexpr_swappable_v<T>, T>::type
be_x_le (const T& v)
{
  return byteswap(v);
}

/** Convert between big-endian and little-endian byte order. */
template <typename T>
typename std::enable_if<!details::is_constexpr_swappable_v<T>, T>::type
be_x_le (const T& v)
{
  return byteswap(v);
}

/** Convert between host and network byte order.
 *
 * @note This `constexpr` overload is only available for types where a
 * constexpr byte swap is supported, i.e. integral scalars. */
template <typename T>
constexpr typename std::enable_if<details::is_constexpr_swappable_v<T>, T>::type
host_x_network (const T& v)
{
  return host_x_be(v);
}

/** Convert between host and network byte order. */
template <typename T>
typename std::enable_if<!details::is_constexpr_swappable_v<T>, T>::type
host_x_network (const T& v)
{
  return host_x_be(v);
}

/** Infrastructure to fill an octet buffer with data.
 *
 * This allows safe invocation for as much data is desired, allowing overrun
 * detection at the end rather than on each addition.
 *
 * @note This structure references but does not own the memory of the buffer
 * that it manages. */
class octets_helper
{
public:
  /** Type used for span values. */
  using size_type = std::size_t;

  /** Reference an octet range into which data will be written.
   *
   * @param begin pointer to the first octet in the sequence.
   *
   * @param end pointer just past the last octet available for the sequence. */
  octets_helper (uint8_t* begin,
                 uint8_t* end) :
    begin_{begin},
    end_{end}
  {
    reset();
  }

  /** Reference an octet sequence into which data will be written.
   *
   * @param begin pointer to the first octet in the sequence.
   *
   * @param count the number of octets in the sequence. */
  octets_helper (uint8_t* begin,
                 size_type count) :
    octets_helper{begin, begin + count}
  { }

  /** Remove all content from the buffer.
   *
   * After this valid() will be `true`, the buffer will be zeroed, and
   * the next write will begin at the buffer start. */
  void reset () noexcept
  {
    if (begin_) {
      memset(begin_, 0, end_ - begin_);
    }
    bp_ = begin_;
  }

  /** Indicates whether advance() caused an error.
   *
   * @return `false` iff advance() was invoked to reserve more space than was
   * available or invalidate() was called since the last reset(). */
  const bool valid () const noexcept
  {
    return bp_;
  }

  /** Explicitly mark the buffer invalid.
   *
   * This might be used by subclasses that use a more piece-wise conception of
   * whether an allocation will fit. */
  void invalidate () noexcept
  {
    bp_ = nullptr;
  }

  /** Get a pointer to the start of buffer.
   *
   * @return as described, but a null pointer if not @ref valid. */
  const void* begin () const noexcept
  {
    return bp_ ? begin_ : nullptr;
  }

  /** Get a pointer to the end of the filled part of the buffer.
   *
   * I.e. the span from [begin(), end()) has filled content.
   *
   * @return as described, but a null pointer if not @ref valid. */
  const void* end () const noexcept
  {
    return bp_ ? bp_ : nullptr;
  }

  /** Number of octets stored in the buffer.
   *
   * @note The returned size is zero if not @ref valid. */
  const size_type size () const noexcept
  {
    return bp_ ? (bp_ - begin_) : 0U;
  }

  /** Number of unused octets available in the buffer.
   *
   * @note The returned size is zero if not @ref valid. */
  const size_type available () const noexcept
  {
    return bp_ ? (end_ - bp_) : 0U;
  }

  /** Maximum number of octets supported by the buffer. */
  const size_type max_size () const noexcept
  {
    return end_ - begin_;
  }

  /** Allocate a region and return a pointer to it or a nullptr if the advance
   * went too far.
   *
   * @param s the number of octets required.
   *
   * @return a pointer into the allocated buffer that allows writing @p s
   * octets.  The pointer is null (and valid() will now return false) if the
   * buffer had already become invalid, or did so as a result of advancing by
   * @p s. */
  void* advance (size_type s) noexcept
  {
    void* rv = nullptr;
    if (can_advance(s)) {
      rv = bp_;
      bp_ += s;
    } else {
      bp_ = nullptr;
    }
    return rv;
  }

  /** Indicate whether advance() would succeed for a given span.
   *
   * @param s as with advance()
   *
   * @return `true` if the buffer is @ref valid and has sufficient space for @p
   * s octets, otherwise `false`.  A `false` return does not invalidate a @ref
   * valid buffer. */
  bool can_advance (size_type s) const noexcept
  {
    return bp_ ? (static_cast<size_type>(end_ - bp_) >= s) : false;
  }

  /** Append a value to the buffer.
   *
   * This invokes advance() and if the space reservation was successful stores
   * the range.
   *
   * @param sp pointer to the value to be appended.
   *
   * @param span the number of octets to be appended.
   *
   * @return as with advance() */
  bool append (const void* sp,
               size_type span) noexcept
  {
    if (auto dp = advance(span)) {
      memmove(dp, sp, span);
    }
    return valid();
  }

  /** Append a value to the buffer.
   *
   * The byte-order of the value is native.  If a non-native order is required
   * use append_le() or append_be(), or use host_x_le() or host_x_be() on the
   * argument.
   *
   * @tparam T the underlying type of the value.
   *
   * @param value the value to append.
   *
   * @return as with advance() */
  template <typename T>
  bool append (const T& value) noexcept
  {
    return append(&value, sizeof(value));
  }

  /** As with append() but stores the value converted to big-endian byte
   * order. */
  template <typename T>
  bool append_be (const T& value) noexcept
  {
    return append<T>(host_x_be(value));
  }

  /** As with append() but stores the value converted to little-endian byte
   * order. */
  template <typename T>
  bool append_le (const T& value) noexcept
  {
    return append<T>(host_x_le(value));
  }

protected:
  friend class oob_helper;

  /* Pointer to the start of the buffer.  If null the buffer cannot be used. */
  uint8_t* const begin_;

  /* Pointer to the end of the buffer.  If null the buffer cannot be used. */
  uint8_t* const end_;

  /* Pointer to the current location in the buffer.  If null the buffer is
   * invalid due to advance() asking for too much. */
  uint8_t* bp_ = nullptr;
};

} // ns byteorder
} // ns pabigot

#endif /* PABIGOT_BYTEORDER_HPP */
