// SPDX-License-Identifier: CC-BY-SA-4.0
// Copyright 2015-2018 Peter A. Bigot

#include <gtest/gtest.h>

#include <pabigot/crc.hpp>

namespace {

TEST(CRC, maskForBits)
{
  using namespace pabigot::crc::details;

  EXPECT_EQ(0x00U, uint8_traits::mask_for_bits(0));
  EXPECT_EQ(0x7FU, uint8_traits::mask_for_bits(7));
  EXPECT_EQ(0xFFU, uint8_traits::mask_for_bits(8));
  EXPECT_EQ(0x7FU, uint16_traits::mask_for_bits(7));
  EXPECT_EQ(0xFFFFU, uint16_traits::mask_for_bits(16));
}

TEST(CRC, uintSupport)
{
  using namespace pabigot::crc::details;

  EXPECT_EQ(0xFFu, 0u+uint_support<8>::mask);
  EXPECT_EQ(0xFFFFu, 0u+uint_support<16>::mask);
  EXPECT_EQ(0xFFFF'FFFFul, 0u+uint_support<32>::mask);

  EXPECT_EQ(0x1Fu, 0u+uint_support<5>::mask);
  EXPECT_EQ(0x7Fu, 0u+uint_support<7>::mask);
  EXPECT_EQ(0x3FFu, 0u+uint_support<10>::mask);
  EXPECT_EQ(0x7FFFu, 0u+uint_support<15>::mask);
  EXPECT_EQ(0x1'FFFFu, 0u+uint_support<17>::mask);
  EXPECT_EQ(0x3FF'FFFF'FFFFull, 0u+uint_support<42>::mask);

  //  basic_crc<std::uint8_t, 0x1234> v{};
}

TEST(CRC, reflect)
{
  using namespace pabigot::crc::details;
  {
    using t = uint_support<3>;
    EXPECT_EQ(0x04u, t::reflect(0x01));
    EXPECT_EQ(0x01u, t::reflect(0x04));
    EXPECT_EQ(0x02u, t::reflect(0x02));
    EXPECT_EQ(0x00u, t::reflect(0x00));
  }
  {
    using t = uint_support<4>;
    EXPECT_EQ(0x0Cu, t::reflect(0x03));
  }
  {
    using t = uint_support<5>;
    EXPECT_EQ(0x12u, t::reflect(0x09));
    EXPECT_EQ(0x15u, t::reflect(0x15));
    EXPECT_EQ(0x14u, t::reflect(0x05));
  }
  {
    using t = uint_support<11>;
    EXPECT_EQ(0x50Eu, t::reflect(0x385));
  }
  {
    using t = uint_support<16>;
    EXPECT_EQ(0x8408u, t::reflect(0x1021));
  }
  {
    using t = uint_support<32>;
    EXPECT_EQ(0xEDB88320u, t::reflect(0x04C11DB7));
  }
  {
    using t = uint_support<64>;
    EXPECT_EQ(0xC96C5795D7870F42ull, t::reflect(0x42F0E1EBA9EA3693ull));
  }
  {
    using crc_type = pabigot::crc::crc<4, 0x03, true, true>;
    EXPECT_EQ(0x0C, crc_type::reflect(0x03));
  }
}

/* see: http://reveng.sourceforge.net/crc-catalogue */
const uint8_t check_dat[] = { '1', '2', '3', '4', '5',
                              '6', '7', '8', '9' };
const std::string check_str{"123456789"};

template <typename CRC>
auto crc_with_residual ()
{
  auto crc = CRC::append(check_dat, check_dat + sizeof(check_dat));
  uint8_t buf[CRC::size]{};
  CRC::store(CRC::finalize(crc), buf);
  return CRC::finalize(CRC::append(buf + 0, buf + sizeof(buf), crc));
}

template <typename Tabler>
auto table_with_residual (const Tabler& table)
{
  auto crc = table.append(check_dat, check_dat + sizeof(check_dat));
  uint8_t buf[table.size]{};
  return table.finalize(table.append(buf, table.store(table.finalize(crc), buf), crc));
}

TEST(CRC4, ITU)
{
  using crc_type = pabigot::crc::crc<4, 0x03, true, true>;
  constexpr crc_type::least_type check{0x07};
  EXPECT_EQ(0x00u, crc_type::finalize(crc_type::append(check_dat, check_dat)));
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));

  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  EXPECT_EQ(crc_type::xorout ^ 0x0, crc_type::residue());

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc.residue, crc_type::residue());
  EXPECT_EQ(crc.residue, table_with_residual(crc));
}

TEST(CRC5, ITU)
{
  using crc_type = pabigot::crc::crc<5, 0x15, true, true>;
  constexpr crc_type::least_type check{0x07};
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));

  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  EXPECT_EQ(crc_type::xorout ^ 0x0, crc_type::residue());

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc.residue, crc_type::residue());
  EXPECT_EQ(crc.residue, table_with_residual(crc));
}

TEST(CRC5, USB)
{
  using crc_type = pabigot::crc::crc<5, 0b00101, true, true, 0x1F, 0x1F>;
  constexpr crc_type::least_type check{0x19};
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));

  // Non-zero residual not correctly handled for sub-byte widths
  //  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  //  EXPECT_EQ(crc_type::xorout ^ 0x6, crc_type::residue());

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  //  EXPECT_EQ(crc.residue, crc_type::residue());
  //  EXPECT_EQ(crc.residue, table_with_residual(crc));
}

TEST(CRC8, std)
{
  using crc_type = pabigot::crc::crc<8, 0x07>;
  constexpr crc_type::least_type check{0xF4};
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));

  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  EXPECT_EQ(crc_type::xorout ^ 0x00, crc_type::residue());

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc.residue, crc_type::residue());
  EXPECT_EQ(crc.residue, table_with_residual(crc));

  EXPECT_EQ(0x00u, crc.table[0]);
  EXPECT_EQ(0x07u, crc.table[1]);
  EXPECT_EQ(0xF3u, crc.table[255]);
}

TEST(CRC12, x3GPP)
{
  // NB: This is the only cross-endian algorithm in the catalog
  using crc_type = pabigot::crc::crc<12, 0x80F, false, true>;
  constexpr crc_type::least_type check{0xDAF};
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_str.cbegin(), check_str.cend())));

  // NB: 12-bit width residuals not handled correctly
  // EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  // EXPECT_EQ(crc_type::xorout ^ 0x0000, crc_type::residue());

  static constexpr auto table = crc_type::instantiate_tabler();
  EXPECT_EQ(check, table.finalize(table.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, table.finalize(table.append(check_str.cbegin(), check_str.cend())));
}

TEST(CRC12, CDMA2000)
{
  using crc_type = pabigot::crc::crc<12, 0xF13, false, false, -1>;
  constexpr crc_type::least_type check{0xD4D};
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
}

TEST(CRC15, std)
{
  using crc_type = pabigot::crc::crc<15, 0x4599>;
  constexpr crc_type::least_type check{0x059E};
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
}

TEST(CRC16, XMODEM)
{
  using crc_type = pabigot::crc::crc<16, 0x1021>;
  constexpr crc_type::least_type check{0x31C3};
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());

  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  EXPECT_EQ(crc_type::xorout ^ 0x0000, crc_type::residue());

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc.residue, crc_type::residue());
  EXPECT_EQ(crc.residue, table_with_residual(crc));

  EXPECT_EQ(0x1021u, crc.table[1]);
  EXPECT_EQ(0x2042u, crc.table[2]);
  EXPECT_EQ(0x1EF0u, crc.table[255]);
}

TEST(CRC16, X25)
{
  using crc_type = pabigot::crc::crc<16, 0x1021, true, true, -1, -1>;
  constexpr crc_type::least_type check{0x906E};

  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_str.cbegin(), check_str.cend())));

  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  EXPECT_EQ(crc_type::xorout ^ 0xF0B8, crc_type::residue());

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc.residue, crc_type::residue());
  EXPECT_EQ(crc.residue, table_with_residual(crc));
}

TEST(CRC16, DNP)
{
  using crc_type = pabigot::crc::crc<16, 0x3d65, true, true, 0, -1>;
  constexpr crc_type::least_type check{0xEA82};

  EXPECT_EQ(0x0000u, crc_type::init);
  EXPECT_EQ(0xFFFFu, crc_type::xorout);
  EXPECT_EQ(0xFFFFu, crc_type::finalize(crc_type::append(check_dat, check_dat)));
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_str.cbegin(), check_str.cend())));

  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  EXPECT_EQ(crc_type::xorout ^ 0x66C5u, crc_type::residue()); // bit-inverse of reveng

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc.residue, crc_type::residue());
  EXPECT_EQ(crc.residue, table_with_residual(crc));

  EXPECT_EQ(0x365Eu, crc.table[1]);
  EXPECT_EQ(0x6CBCu, crc.table[2]);
  EXPECT_EQ(0x1235u, crc.table[255]);
}

TEST(CRC16, EN13757)
{
  using crc_type = pabigot::crc::crc<16, 0x3d65, false, false, 0, -1>;
  constexpr crc_type::least_type check{0xC2B7};

  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_str.cbegin(), check_str.cend())));

  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  EXPECT_EQ(crc_type::xorout ^ 0xA366u, crc_type::residue());

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc.residue, crc_type::residue());
  EXPECT_EQ(crc.residue, table_with_residual(crc));
}

TEST(CRC16, DECTxR)
{
  using crc_type = pabigot::crc::crc<16, 0x0589, false, false, 0, 1>;
  constexpr crc_type::least_type check{0x007E};
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_str.cbegin(), check_str.cend())));

  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  EXPECT_EQ(crc_type::xorout ^ 0x0589, crc_type::residue());

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc.residue, crc_type::residue());
  EXPECT_EQ(crc.residue, table_with_residual(crc));
}

TEST(CRC24, std)
{
  using crc_type = pabigot::crc::crc<24, 0x864CFB, false, false, 0xB704CE, 0>;
  constexpr crc_type::least_type check{0x21CF02};
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));

  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  EXPECT_EQ(crc_type::xorout ^ 0u, crc_type::residue());

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc.residue, crc_type::residue());
  EXPECT_EQ(crc.residue, table_with_residual(crc));
}

TEST(CRC24, BLE)
{
  using crc_type = pabigot::crc::crc<24, 0x00065b, true, true, 0x555555>;
  constexpr crc_type::least_type check{0xc25a56};
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));

  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  EXPECT_EQ(crc_type::xorout ^ 0u, crc_type::residue());

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc.residue, crc_type::residue());
  EXPECT_EQ(crc.residue, table_with_residual(crc));
}

TEST(CRC32, std)
{
  using crc_type = pabigot::crc::CRC32; // crc<32, 0x04c11db7, true, true, -1, -1>;
  constexpr crc_type::least_type check{0xCBF43926};

  EXPECT_EQ(0xFFFFFFFFu, crc_type::init);
  EXPECT_EQ(0xFFFFFFFFu, crc_type::xorout);
  EXPECT_EQ(0x00000000u, crc_type::finalize(crc_type::append(check_dat, check_dat)));

  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  EXPECT_EQ(crc_type::xorout ^ 0xDEBB20E3u, crc_type::residue());

  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc.residue, crc_type::residue());
  EXPECT_EQ(crc.residue, table_with_residual(crc));

  EXPECT_EQ(0x00000000u, crc.table[0]);
  EXPECT_EQ(0x77073096u, crc.table[1]);
  EXPECT_EQ(0x2D02EF8Du, crc.table[255]);
}

TEST(CRC32, BZIP2)
{
  using crc_type = pabigot::crc::crc<32, 0x04c11db7, false, false, -1, -1>;
  constexpr crc_type::least_type check{0xFC891918};

  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_str.cbegin(), check_str.cend())));

  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  EXPECT_EQ(crc_type::xorout ^ 0xC704DD7Bu, crc_type::residue());

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc.residue, crc_type::residue());
  EXPECT_EQ(crc.residue, table_with_residual(crc));
}

TEST(CRC32, MPEG2)
{
  using crc_type = pabigot::crc::crc<32, 0x04c11db7, false, false, -1, 0>;
  constexpr crc_type::least_type check{0x0376e6e7};

  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_str.cbegin(), check_str.cend())));

  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  EXPECT_EQ(crc_type::xorout ^ 0U, crc_type::residue());

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc.residue, crc_type::residue());
  EXPECT_EQ(crc.residue, table_with_residual(crc));
}

TEST(CRC32, POSIX)
{
  using crc_type = pabigot::crc::crc<32, 0x04c11db7, false, false, 0, -1>;
  constexpr crc_type::least_type check{0x765E7680};

  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_str.cbegin(), check_str.cend())));

  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  EXPECT_EQ(crc_type::xorout ^ 0xC704DD7Bu, crc_type::residue());

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc.residue, crc_type::residue());
  EXPECT_EQ(crc.residue, table_with_residual(crc));
}

TEST(CRC64, std)
{
  using crc_type = pabigot::crc::crc<64, 0x42f0e1eba9ea3693, false, false, 0, 0>;
  constexpr crc_type::least_type check{0x6C40DF5F0B497347 };

  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_str.cbegin(), check_str.cend())));

  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  EXPECT_EQ(crc_type::xorout ^ 0U, crc_type::residue());

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc.residue, crc_type::residue());
  EXPECT_EQ(crc.residue, table_with_residual(crc));
}

TEST(CRC64, XZ)
{
  using crc_type = pabigot::crc::crc<64, 0x42f0e1eba9ea3693, true, true, -1, -1>;
  constexpr crc_type::least_type check{0x995DC9BBDF1939FA};

  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc_type::finalize(crc_type::append(check_str.cbegin(), check_str.cend())));

  EXPECT_EQ(crc_type::residue(), crc_with_residual<crc_type>());
  EXPECT_EQ(crc_type::xorout ^ 0x49958C9ABD7D353Fu, crc_type::residue());

  static constexpr auto crc = crc_type::instantiate_tabler();
  EXPECT_EQ(check, crc.finalize(crc.append(check_dat, check_dat + sizeof(check_dat))));
  EXPECT_EQ(check, crc.finalize(crc.append(check_str.cbegin(), check_str.cend())));
  EXPECT_EQ(crc.residue, crc_type::residue());
  EXPECT_EQ(crc.residue, table_with_residual(crc));
}

} // ns anonymous
