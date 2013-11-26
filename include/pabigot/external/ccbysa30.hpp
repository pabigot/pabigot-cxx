/* SPDX-License-Identifier: (Apache-2.0 & CC-BY-SA-3.0) */

/** Material from external sources with CC-BY-SA-3.0 licensing.
 *
 * The attribution for such material is documented here, isolated to simplify
 * license compliance checking.
 *
 * @file */

#ifndef PABIGOT_EXTERNAL_CCBYSA30_HPP
#define PABIGOT_EXTERNAL_CCBYSA30_HPP
#pragma once

#include <cinttypes>
#include <climits>
#include <type_traits>
#include <utility>

namespace pabigot {
namespace external {

/** External material licensed under Creative Commons Attribution Share Alike
 * 3.0.
 *
 * @see https://spdx.org/licenses/CC-BY-SA-3.0.html */
namespace ccbysa30 {

namespace details {

/** C++17 constexpr byte-swapping implementation.
 *
 * @see https://en.cppreference.com/w/cpp/language/fold#Example
 * @see http://stackoverflow.com/a/36937049 */
template<class T,
         std::size_t... N>
constexpr T
bswap_impl(T i, std::index_sequence<N...>)
{
  return ((((i >> (N * CHAR_BIT)) & std::uint8_t(-1))
           << ((sizeof(T) - 1 - N) * CHAR_BIT))
          | ...);
}

} // ns details

/** C++17 constexpr byte-swapping implementation.
 *
 * @see https://en.cppreference.com/w/cpp/language/fold#Example
 * @see http://stackoverflow.com/a/36937049 */
template<class T,
         class U = typename std::make_unsigned<T>::type>
constexpr U
bswap(T i)
{
  return details::bswap_impl<U>(i, std::make_index_sequence<sizeof(T)>{});
}

} // ns external
} // ns external
} // ns pabigot

#endif /* PABIGOT_EXTERNAL_CCBYSA30_HPP */
