/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2015-2019 Peter A. Bigot */

/** Templates supporting CRC calculation using Rocksoft^tm Model parameters.
 *
 * @file */

#ifndef PABIGOT_CRC_HPP
#define PABIGOT_CRC_HPP
#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#include <pabigot/byteorder.hpp>

namespace pabigot {

/** Templates supporting CRC calculation using Rocksoft^tm Model parameters.
 *
 * @see http://www.ross.net/crc/download/crc_v3.txt
 * @see http://reveng.sourceforge.net/crc-catalogue/
 */
namespace crc {

/** Internal support for pabigot::crc.
 *
 * Applications should not need to use any of these. */
namespace details {

/** Reverse bits `[0..bits)` of `v`.
 *
 * @tparam UI an unsigned integral type
 *
 * @param v a value, the low `bits` bits of which contain data
 *
 * @param bits the number of valid bits within a word of type @p UI
 *
 * @return the result of reversing the low `bits` bits of `v`.
 *
 * @note Keep this outside of the traits classes since the same underlying
 * `fast_type` may be shared for different sizes. */
template <typename UI>
static constexpr UI
reverse_low_bits (const UI& v,
                  unsigned int bits)
{
  UI msb = (static_cast<UI>(1) << (bits - 1));
  UI lsb = 1;
  UI out = 0;

  /* msb right-shifts; when it becomes zero we're done. */
  while (msb) {
    if (msb & v) {
      out |= lsb;
    }
    msb >>= 1;
    lsb <<= 1;
  }
  return out;
}

/** Compute the #uint_size_traits category value for a required bit length.
 *
 * @tparam B the number of bits required in an unsigned integer.
 *
 * @return the category value for @p B */
template <unsigned int B>
constexpr unsigned int uint_category ()
{
  return (B<=8) + (B<=16) + (B<=32) + (B<=64);
}

/** Entry-point to identifying unsigned integral types supporting a specific
 * number of bits.
 *
 * The concept here is derived from the approach in boost::integer, simplified
 * by excluding signed types and exact representations; restricted by relying
 * on the types in <cstdint> instead of native types; and possibly improved by
 * not assuming the least and fast types are the same:
 *
 * * `width` is the number of bits in the value;
 * * `fast_type` is the fast unsigned type for width bits.  This is highly
 *   likely to have more than `width` bits; keep this in mind when
 *   byte-swapping values.
 * * `least_type` is the least unsigned type for width bits.  When `width` is
 *   not a power of 2 this is highly likely to have more than `width` bits;
 *   keep this in mind when byte-swapping values.
 *
 * @tparam C a category value: 1 for 64 bit, 2 for 32 bit, 3 for 16 bit, and 4
 * for 8 bit.  A specialization is provided for each of these. */
template <unsigned int C>
struct uint_size_traits
{ };

template <>
struct uint_size_traits<4>
{
  static constexpr unsigned int width = 8;
  using fast_type = uint_fast8_t;
  using least_type = uint_least8_t;
};

template <>
struct uint_size_traits<3>
{
  static constexpr unsigned int width = 16;
  using fast_type = uint_fast16_t;
  using least_type = uint_least16_t;
};

template <>
struct uint_size_traits<2>
{
  static constexpr unsigned int width = 32;
  using fast_type = uint_fast32_t;
  using least_type = uint_least32_t;
};

template <>
struct uint_size_traits<1>
{
  static constexpr unsigned int width = 64;
  using fast_type = uint_fast64_t;
  using least_type = uint_least64_t;
};

/** Extend the size-related traits with functionality operating on the specific
 * types.
 *
 * Specifically, provide the ability to bit-reverse a value stored in the low
 * bits of a word.  This supports the requirements of a reflected (LSB-first)
 * algorithm. */
template <unsigned int C>
struct uint_traits : public uint_size_traits<C>
{
  using super = uint_size_traits<C>;
  using typename super::fast_type;
  using typename super::least_type;
  using fast_limits = std::numeric_limits<fast_type>;

  /** Reverse bits `[0..bits)` of `v`.
   *
   * @see reverse_low_bits */
  static constexpr fast_type
  reverse (const fast_type& v,
           unsigned int bits)
  {
    return reverse_low_bits<fast_type>(v, bits);
  }

  /** Helper to calculate the value of a B-bit mask as an instance of the
   * unsigned type T.
   *
   * Need this evaluated at compile-time, supporting both cases where T can
   * represent strictly more bits than are needed and where T represents exactly
   * that many bits.
   *
   * @tparam T an unsigned integral type.
   *
   * @param bits the number of bits to be masked.  If @p bits is greater than the
   * number of bits representable in @p T undefined behavior may result. */
  static constexpr fast_type
  mask_for_bits (unsigned int bits)
  {
    return ((bits == fast_limits::digits)
            ? static_cast<fast_type>(-1)
            : ((static_cast<fast_type>(1) << bits) - 1));
  }

};

/** @link uint_traits@endlink for `uint8_t` */
using uint8_traits = uint_traits<4>;
/** @link uint_traits@endlink for `uint16_t` */
using uint16_traits = uint_traits<3>;
/** @link uint_traits@endlink for `uint32_t` */
using uint32_traits = uint_traits<2>;
/** @link uint_traits@endlink for `uint64_t` */
using uint64_traits = uint_traits<1>;

/** Type and policy traits providing support for operations on `B`-bit values.
 *
 * @tparam B the number of bits as a positive value not exceeding 64. */
template <unsigned int B>
struct uint_support
{
  /** The number of bits for which this class is targeted.
   *
   * This is a lower bound on the number of bits supported by #fast_type and
   * #least_type, and influences the values of #msbit, #lsbit, #mask, and the
   * default upper bit position used in reflect(). */
  static constexpr unsigned int width = B;

  /** Characteristics of the unsigned integer support required for a #width-bit
   * value. */
  using uint_traits = details::uint_traits<uint_category<B>()>;

  /** The type used to hold values while they're being operated on.
   *
   * This is the fastest unsigned type capable of holding at least #width
   * bits. */
  using fast_type = typename uint_traits::fast_type;

  /** The type used to hold values while they're being stored.
   *
   * This is the smallest type capable of holding at least #width bits. */
  using least_type = typename uint_traits::least_type;

  /** The number of bits used in #fast_type, should that be useful. */
  static constexpr unsigned int fast_width = std::numeric_limits<fast_type>::digits;

  /** The number of bits used in #least_type, should that be useful. */
  static constexpr unsigned int least_width = std::numeric_limits<least_type>::digits;

  /** A mask value that will discard bits at and above bit number #width in a
   * value. */
  static constexpr fast_type mask{uint_traits::mask_for_bits(width)};

  /** Mask isolating the most significant bit in a value. */
  static constexpr fast_type msbit{static_cast<fast_type>(1) << (width - 1)};

  /** Mask isolating the least significant bit in a value. */
  static constexpr fast_type lsbit{static_cast<fast_type>(1)};

  /** Reflect a value around its center.
   *
   * @param v the value to be reflected
   *
   * @param n the number of bits of @p in that are valid.  Bit `n-1` of @p in
   * is reflected into bit `0` of the returned value; similarly bit `0` of @p
   * in shows up in bit `n-1` of the returned value.  Defaults to #width, but
   * `8` and other values may be required when reflecting input chunks.
   */
  static constexpr fast_type
  reflect (const fast_type& v,
           unsigned int n = width)
  {
    return uint_traits::reverse(v, n);
  }

  /** Apply message bits into the CRC.
   *
   * @param poly the normal-form polynomial for the CRC.
   *
   * @param crc the remainder polynomial for the previously encountered message
   * bits.
   *
   * @param msg bits of message, already reflected and shifted so the first bit
   * is in bit `n-1` and the last bit is in bit `0`.
   *
   * @param n the number of bits in @p msg that carry message bits.  This must
   * not exceed #width.
   *
   * @return the CRC after incorporating @p msg using @p poly. */
  static constexpr fast_type
  crc_apply (const fast_type& poly,
             const fast_type& crc,
             const fast_type& msg,
             const unsigned int n)
  {
    auto rv = crc;
    rv ^= msg << (width - n);
    for (unsigned int bi{}; bi < n; ++bi) {
      bool xor_poly{msbit & rv};
      rv <<= 1;
      if (xor_poly) {
        rv ^= poly;
      }
    }
    return mask & rv;
  }
};

/** Use a template parameter pack to fill out a 256-element
 * std::initializer_list for a std::array. */
template <typename CRC,
         size_t... n>
constexpr auto lookup_table (std::index_sequence<n...>)
{
  using least_type = typename CRC::least_type;
  return std::array<least_type, sizeof...(n)>{{CRC::lookup_for_byte(n)...}};
}

/** Return a std::array for the CRC byte-indexed table.
 *
 * @tparam A fully instantiated @link crc@endlink class.
 *
 * @return a `std::array` with 256 elements where element `i` is the result of
 * invoking crc::lookup_for_byte on `i`. */
template <typename CRC>
constexpr auto lookup_table()
{
  return lookup_table<CRC>(std::make_integer_sequence<size_t, 256>{});
}

/** CRC core framework using Rocksoft^tm Model characteristics.
 *
 * This class supports the basic operations that are common when
 * parameterized by polynomial and output processing.
 *
 * @tparam W initializes #width.
 *
 * @tparam Rin initializes #refin. */
template <unsigned int W,
          bool Rin = false>
class base_crc
{
public:
  /** The width of the CRC in bits. */
  static constexpr unsigned int width = W;

  /** The width of the CRC in bytes. */
  static constexpr unsigned int size = (7 + width) / 8;

protected:
  /** Traits class providing native integral types and
   * parameter-independent values supporting #width-bit operations. */
  using support_traits = details::uint_support<width>;

public:
  using uint_traits = typename support_traits::uint_traits;

  /** A fast native unsigned integral type capable of holding
   * #width-bit CRC values.
   *
   * @warning This value may have more octets or bits than are required to
   * store the CRC. */
  using fast_type = typename support_traits::fast_type;

  /** A small native unsigned integral type capable of holding
   * #width-bit CRC values.
   *
   * @warning This value may have more octets or bits than are required to
   * store the CRC. */
  using least_type = typename support_traits::least_type;

  /** The mask that discards bits of a #fast_type that would be
   * outside the #width limit. */
  static constexpr fast_type mask = support_traits::mask;

  /** `true` iff the input message bit stream is reflected.
   *
   * The CRC is a linear shift register; it is necessary to know which bit in a
   * byte should be fed in first:
   * * When `refin` is `false` the most significant bit of the byte is consumed
   *   first.  This is the "big-endian" configuration.
   * * When `refin` is `true` the least significant bit of the byte is consumed
   *   first.  This is the "little-endian" configuration.
   *
   * This parameter also affects crc::store() in that CRCs with #width greater
   * than 8 must use the byte order that matches the bit order when catenated
   * to the end of a message to ensure that the aggregate CRC matches the
   * crc::residual(). */
  static constexpr bool refin = Rin;

  /** Delegate to support_traits::reflect */
  static constexpr fast_type
  reflect (const fast_type& v)
  {
    return support_traits::reflect(v, width);
  }

  /** Store a finalized CRC into a buffer in a way that enables crc::residue().
   *
   * @warning If the CRC #width is not a multiple of 8 the stored value will
   * not be correct, as this namespace provides no API to operate on messages
   * that are not an integral number of octets.
   *
   * @param crc the finalized CRC of a message.
   *
   * @param bp pointer to space of #size bytes immediately following the
   * message, into which the CRC can be stored.
   *
   * @return pointer into the region identified by @p bp immediately following
   * the stored CRC. */
  static constexpr uint8_t*
  store (fast_type crc,
         uint8_t* bp)
  {
    auto nb = size;
    while (nb--) {
      if (refin) {
        *bp++ = crc;
        crc >>= 8;
      } else {
        *bp++ = crc >> (8 * nb);
      }
    }
    return bp;
  }

protected:

  /** Calculate a single lookup table entry using the given
   * polynomial.
   *
   * @param index the table index for which the precomputed CRC
   * adjustment is desired.
   *
   * @param poly the polynomial for which the table is being
   * generated.
   *
   * @return the effective CRC adjustment using @p poly when the next
   * 8 bits to shift out of the CRC are @p index.  */
  static constexpr least_type
  lookup_for_byte (const fast_type& poly,
                   std::uint_fast8_t byte)
  {
    if (refin) {
      byte = support_traits::reflect(byte, 8);
    }

    auto rv = crc_apply(poly, 0, byte, 8);
    /* Assume the common case that refin==refout.  For cross-endian we need to
     * correct this during finalize. */
    if (refin) {
      rv = reflect(rv);
    }
    return mask & rv;
  }

  /** Apply message bits into a CRC.
   *
   * @param poly the normal-form polynomial for the CRC.
   *
   * @param crc the remainder polynomial for the previously
   * encountered message bits.
   *
   * @param msg bits of message, already reflected.
   *
   * @param n the number of bits in @p msg that carry message bits.
   *
   * @return the CRC after incorporating @p msg using @p poly. */
  static constexpr fast_type
  crc_apply (const fast_type& poly,
             const fast_type& crc,
             fast_type msg,
             unsigned int n)
  {
    auto rv = crc;
    while (n > width) {
      fast_type submsg = msg >> (n - width);
      rv = support_traits::crc_apply(poly, rv, submsg, width);
      n -= width;
    }
    return support_traits::crc_apply(poly, rv, msg, n);
  }
};

} // ns details

/** Class encapsulating everything necessary for table-driven CRC calculations.
 *
 * The table is an instance member; everything else is a constexpr class static
 * member.  Construct these as global objects at compile-time with definitions
 * like:
 *
 *     constexpr auto crc = pabigot::crc::crc<32, 0x04c11db7, true, true, -1, -1>::instantiate_tabler();
 *
 * Use them like:
 *
 *     uint8_t* cp = prepare_message(buf);
 *     auto pre = crc.append(buf, cp);
 *     const uint8_t* ep = crc.store(crc.finalize(pre), cp);
 *     transmit(buf, ep);
 *
 * On reception:
 *
 *     auto len = receive(buf);
 *     if (crc.residue != crc.finalize(crc.append(buf, buf + len))) {
 *       // error in aggregate message
 *     }
 */
template <typename CRC>
class Tabler : public CRC::uint_traits
{
  using super_ = typename CRC::uint_traits;

public:
  /** The underlying @ref crc type. */
  using crc_type = CRC;

  /** A time-efficient unsigned integral type capable of holding checksum values. */
  using typename super_::fast_type;

  /** A space-efficient unsigned integral type capable of holding checksum values. */
  using typename super_::least_type;

  /** How the CRC table elements are stored. */
  using table_type = std::array<least_type, 256>;

  /** Rocksoft^TM model parameters for the checksum algorithm supported by this
   * type. */
  struct params {
    /** Number of bits in the CRC */
    static constexpr unsigned int width = CRC::width;
    /** CRC polynomial */
    static constexpr least_type poly = CRC::poly;
    /** CRC initial value before message bits are processed */
    static constexpr least_type init = CRC::init;
    /** Value used to mutate unreflected final CRC */
    static constexpr least_type xorout = CRC::xorout;
    /** `true` iff input data is to be reflected */
    static constexpr bool refin = CRC::refin;
    /** `true` iff the final CRC is to be reflected */
    static constexpr bool refout = CRC::refout;
  };

  /** The number of bytes required to store a CRC value of #width bits. */
  static constexpr size_t size = CRC::size;

private:

  /** The initial CRC for table-based checksum calculations.
   *
   * @return the unfinalized checksum for table calculations with no message
   * bits consumed. */
  static constexpr fast_type
  make_init ()
  {
    fast_type crc = CRC::init;

    /* The table incorporates the effects of input reflection, but for that to
     * work the initial value needs to be reflected. */
    if (CRC::refin) {
      crc = super_::reverse(crc, CRC::width);
    }
    return crc;
  }

  // CRC is the only thing allowed to construct these.  Nobody can copy them.
  friend CRC;

  constexpr Tabler () :
    table{details::lookup_table<CRC>()}
  { }

  Tabler (const Tabler&) = delete;
  Tabler& operator= (const Tabler&) = delete;
  Tabler (const Tabler&&) = delete;
  Tabler& operator= (const Tabler&&) = delete;

public:

  /** Store a finalized CRC into a buffer in a way that enables crc::residue().
   *
   * @param crc the finalized CRC of a message.
   *
   * @param bp pointer to space of #size bytes immediately following the
   * message, into which the CRC can be stored.
   *
   * @return pointer into the region identified by @p bp immediately following
   * the stored CRC. */
  uint8_t*
  store (least_type crc,
         uint8_t* bp) const
  {
    size_t nb{size};
    while (nb--) {
      if (CRC::refin) {
        *bp++ = crc;
        crc >>= 8;
      } else {
        *bp++ = crc >> (8 * nb);
      }
    }
    return bp;
  }

  /** Table-driven update of a CRC given a data octet.
   *
   * @param octet the next octet of message bits.
   *
   * @param crc the CRC value calculated over all previous message bits.  Start
   * with #init, which is the defaulted value.
   *
   * @return the unreflected CRC value over all message bits through this
   * invocation.
   *
   * @see finalize() */
  constexpr fast_type
  append (uint8_t octet,
          fast_type crc = init) const
  {
    constexpr fast_type index_mask = 0xFFu;
    if (CRC::refin) {
      crc = table[(crc ^ octet) & index_mask] ^ (crc >> 8);
    } else {
      crc = table[((crc >> (CRC::width - 8)) ^ octet) & index_mask] ^ (crc << 8);
    }
    return CRC::mask & crc;
  }

  /** Table-driven calculation of a CRC from a sequence of octet values.
   *
   * @tparam InputIterator iterator over inputs.  InputIterator::value_type
   * must be convertible to an unsigned 8-bit value without changing the value
   * content.
   *
   * @param first beginning of the range over which CRCs will be calculated.
   *
   * @param last beginning of the range over which CRCs will be calculated.
   *
   * @param crc the CRC value calculated over all previous message bits.  Start
   * with #init, which is the defaulted value.
   *
   * @return the unreflected CRC value over all message bits through this
   * invocation.
   *
   * @see finalize() */
  template <typename InputIterator>
  constexpr fast_type
  append (InputIterator first,
          InputIterator last,
          fast_type crc = init) const
  {
    using src_type = typename std::iterator_traits<InputIterator>::value_type;
    using limits = std::numeric_limits<src_type>;

    /* Unsigned 8-bit or signed 7-bit data passes this test */
    constexpr auto bits_per_chunk = limits::digits + (limits::is_signed ? 1U : 0U);
    static_assert(bits_per_chunk == 8,
                  "cannot do table-driven from non-octet source");

    auto rv = crc;
    while (last != first) {
      rv = append(static_cast<uint8_t>(*first), rv);
      ++first;
    }
    return rv;
  }

  /** Perform necessary post-processing to get the final checksum.
   *
   * @param crc an unreflected unmodified CRC over input bits.
   *
   * @return the reflected modified CRC. */
  constexpr least_type
  finalize (fast_type crc) const
  {
    crc ^= CRC::xorout;

    /* As long as the input and output endian match the result doesn't need
     * additional processing because the tables values are already reflected.
     * If they differ, then the output needs to be reflected. */
    if (CRC::refin ^ CRC::refout) {
      crc = super_::reverse(crc, CRC::width);
    }
    return static_cast<least_type>(crc);
  }

  /** The CRC initial value in the form required for table calculations. */
  static constexpr fast_type init = make_init();

  /** The @link crc::residue value expected@endlink when calculating the
   * finalized CRC over an aggregate of a message and its store()d checksum. */
  static constexpr least_type residue = CRC::residue();

  /** The CRC table, indexed by unreflected input octet. */
  const table_type table;
};

/** CRC calculation using Rocksoft^tm Model characteristics.
 *
 * @tparam W the @link details::base_crc::width width@endlink of the CRC in
 * bits.
 *
 * @tparam Poly the @link poly CRC polynomial@endlink in normal form.
 *
 * @tparam Rin whether the input bit stream is @link details::base_crc::refin
 * reflected@endlink.
 *
 * @tparam Rout whether the output bit stream is @link refout
 * reflected@endlink.
 *
 * @tparam Init the @link init@endlink initial value of the CRC register before
 * any bits have been processed.  A value of `-1` may be used to start with all
 * bits set.
 *
 * @tparam XorOut the value @link xorout subtracted@endlink from the calculated
 * result prior to returning it.  A value of `-1` may be used to bitwise invert
 * the value.
 *
 * @see http://www.ross.net/crc/download/crc_v3.txt
 * @see http://reveng.sourceforge.net/crc-catalogue/
 * @see http://users.ece.cmu.edu/~koopman/crc/index.html
 * @see https://en.wikipedia.org/wiki/Mathematics_of_cyclic_redundancy_checks
 * especially the Koopman references
 */
template <unsigned int W,
          uintmax_t Poly,
          bool Rin = false,
          bool Rout = false,
          uintmax_t Init = 0,
          uintmax_t XorOut = 0>
class crc : public details::base_crc<W, Rin>
{
  using super_ = details::base_crc<W, Rin>;
  using this_type = crc<W, Poly, Rin, Rout, Init, XorOut>;

protected:
  using typename super_::support_traits;

public:
  using typename super_::fast_type;
  using typename super_::least_type;

  /** The @ref Tabler associated with this CRC type. */
  using tabler_type = Tabler<this_type>;

  /** `true` iff the output CRC is to be reflected. */
  static constexpr bool refout = Rout;

  /** Value expected for `CRC(x || XE(CRC(x)))`.
   *
   * When the checksum of the message is @link store stored@endlink at the end
   * of the message a receiver can calculate the finalized checksum of the
   * message plus the stored checksum.  This value will be equal to the
   * residue() if no errors are discovered.
   *
   * @note The residues produced by this function may not match those specified
   * by the [reveng catalog](http://reveng.sourceforge.net/crc-catalogue/)
   * because the classic check algorithm used for residues does not apply
   * crc::xorout to the final value.  See, for example, [this
   * code](https://tools.ietf.org/html/rfc1662#appendix-C.3) for the classic
   * CRC-32 algorithm.  The value here is correct when compared with a @link
   * finalize finalized@endlink checksum.
   *
   * @warning This library does not properly handle residue calculation for
   * messages that are not an integral number of bytes, or CRCs with a width
   * that is not a multiple of 8 bits. */
  static constexpr least_type residue ()
  {
    uint8_t buf[super_::size]{};
    store(finalize(init), buf);
    return finalize(append(buf, buf + sizeof(buf)));
  }

  using super_::store;

  /** The CRC polynomial in normal form (all bits except bit #width
   * which is known to be 1 are provided). */
  static constexpr fast_type poly = super_::mask & Poly;

  /** The initial value of the CRC register before any message bits
   * have been processed. */
  static constexpr least_type init = super_::mask & Init;

  /** A value subtracted (xor'd) from the calculated result prior to returning
   * it. */
  static constexpr least_type xorout = super_::mask & XorOut;

  /** Augment a checksum with additional data.
   *
   * @tparam InputIterator iterator over inputs.  InputIterator::value_type
   * must be convertible to an unsigned 8-bit value without changing the value
   * content.
   *
   * @param first beginning of the range over which CRCs will be calculated.
   *
   * @param last beginning of the range over which CRCs will be calculated.
   *
   * @param crc the CRC value calculated over all previous message bits.  Start
   * with #init, which is the defaulted value.
   *
   * @return the unreflected CRC value over all message bits through this
   * invocation.  This may be passed as `crc` to process more message content.
   *
   * @see finalize() */
  template <typename InputIterator>
  static constexpr fast_type
  append (InputIterator first,
          InputIterator last,
          const fast_type& crc = init)
  {
    using src_type = typename std::iterator_traits<InputIterator>::value_type;
    constexpr auto bits_per_chunk =
      std::numeric_limits<src_type>::digits
      + (std::numeric_limits<src_type>::is_signed ? 1U : 0U);
    static_assert(bits_per_chunk <= support_traits::fast_width,
                  "cannot aggregate from source exceeding CRC operating type capacity");
    auto rv = crc;
    while (last != first) {
      /* Support signed values (viz. `signed char`) by first casting them to
       * the unsigned least type, so they can promote to the unsigned fast type
       * without violating narrowing rules. */
      fast_type chunk = static_cast<least_type>(*first);
      ++first;
      if (super_::refin) {
        chunk = support_traits::reflect(chunk, bits_per_chunk);
      }
      rv = super_::crc_apply(poly, rv, chunk, bits_per_chunk);
    }
    return rv;
  }

  /** Delegate to super_::lookup_for_byte. */
  static constexpr least_type
  lookup_for_byte (std::uint_fast8_t byte)
  {
    return super_::lookup_for_byte(poly, byte);
  }

  /** Calculate the final CRC value for a sequence.
   *
   * #refout and #xorout are applied to the final register value and the result
   * returned.
   *
   * @note A common operation on CRCs is to store them into a buffer for
   * transfer to another system, particularly to enable use of residue().  Be
   * aware that if #width is not a power of 2 then #least_type is likely to
   * have padding bits in it, for which a basic byte-swap for endianness will
   * produce anomalous results.  See store() for a partial solution.
   *
   * @param crc the CRC resulting after all message bits have been calculated.
   *
   * @return the CRC of the message in smallest type capable of holding it. */
  static constexpr least_type
  finalize (fast_type crc)
  {
    if (refout) {
      crc = super_::reflect(crc);
    }
    crc ^= xorout;
    return static_cast<least_type>(crc);
  }

  /** Construct an object that does table-driven CRC calculations for this
   * algorithm. */
  static constexpr tabler_type instantiate_tabler ()
  {
    return {};
  }
};

/** The standard 32-bit CRC algorithm.
 *
 * @see http://reveng.sourceforge.net/crc-catalogue/17plus.htm#crc.cat-bits.32 */
using CRC32 = crc<32, 0x04c11db7, true, true, -1, -1>;

} // ns crc
} // ns pabigot

#endif /* PABIGOT_CRC_HPP */
