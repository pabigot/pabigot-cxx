// SPDX-License-Identifier: CC-BY-SA-4.0
// Copyright 2014-2019 Peter A. Bigot

#include <arpa/inet.h>

#include <array>
#include <vector>

#include <gtest/gtest.h>

#include <pabigot/byteorder.hpp>

namespace {

using namespace pabigot::byteorder;

TEST(ByteOrder, Size1)
{
  union {
    char c;
    unsigned char u;
    uint8_t b;
  } u;

  u.c = 0;
  EXPECT_EQ(0, byteswap(u.c));
  EXPECT_EQ(0, byteswap(u.u));
  EXPECT_EQ(0, byteswap(u.b));

  constexpr uint8_t ce{23};
  constexpr uint8_t ec{byteswap(ce)};
  EXPECT_EQ(ce, ec);
}

TEST(ByteOrder, Size2)
{
  union {
    int16_t i;
    uint16_t u;
  } u;

  u.u = 0x1234U;
  EXPECT_EQ(0x3412U, byteswap(u.u));
  EXPECT_EQ(0x3412, byteswap(u.i));
  u.u = 0xe12dU;
  EXPECT_EQ(0x2de1U, byteswap(u.u));
  EXPECT_EQ(0x2de1, byteswap(u.i));
  u.u = 0x2de1U;
  EXPECT_EQ(0xe12dU, byteswap(u.u));
  EXPECT_EQ(static_cast<int16_t>(0xe12dU), byteswap(u.i));

  const uint16_t pat{0x0182};
  const uint16_t tap{0x8201};
  EXPECT_EQ(tap, byteswap(pat));

  constexpr uint16_t ce{0x1234};
  constexpr uint16_t ec{byteswap(ce)};
  EXPECT_EQ(0x3412, ec);
}

TEST(ByteOrder, Size4)
{
  using bit_type = uint32_t;
  using sbit_type = std::make_signed<bit_type>::type;
  union {
    sbit_type i;
    bit_type u;
  } u;
  const bit_type pat{0x01820384U};
  const bit_type tap{0x84038201U};

  u.u = pat;
  EXPECT_EQ(tap, byteswap(u.u));
  EXPECT_EQ(static_cast<sbit_type>(tap), byteswap(u.i));
  u.u = tap;
  EXPECT_EQ(pat, byteswap(u.u));
  EXPECT_EQ(static_cast<sbit_type>(pat), byteswap(u.i));

  EXPECT_EQ(tap, byteswap(pat));

  constexpr uint32_t ce{0x12345678};
  constexpr uint32_t ec{byteswap(ce)};
  EXPECT_EQ(0x78563412U, ec);
}

TEST(ByteOrder, Size8)
{
  using bit_type = uint64_t;
  using sbit_type = std::make_signed<bit_type>::type;
  union {
    sbit_type i;
    bit_type u;
  } u;
  const bit_type pat{0x0182038405860788U};
  const bit_type tap{0x8807860584038201U};

  u.u = pat;
  EXPECT_EQ(tap, byteswap(u.u));
  EXPECT_EQ(static_cast<sbit_type>(tap), byteswap(u.i));
  u.u = tap;
  EXPECT_EQ(pat, byteswap(u.u));
  EXPECT_EQ(static_cast<sbit_type>(pat), byteswap(u.i));

  EXPECT_EQ(tap, byteswap(pat));

  constexpr uint64_t ce{0x123456789abcdef0};
  constexpr uint64_t ec{byteswap(ce)};
  EXPECT_EQ(0xf0debc9a78563412, ec);
}

TEST(ByteOrder, ConstexprSwappable)
{
  ASSERT_TRUE(details::is_constexpr_swappable_v<uint32_t>);
  ASSERT_TRUE(details::is_constexpr_swappable_v<int32_t>);
  ASSERT_FALSE(details::is_constexpr_swappable_v<float>);
  using array_type = std::array<uint8_t, 4>;
  ASSERT_FALSE(details::is_constexpr_swappable_v<array_type>);

  {
    constexpr int8_t ce{0x12};
    constexpr int8_t ec{byteswap(ce)};
    ASSERT_EQ(0x12, ec);
  }

  {
    constexpr int16_t ce{0x1234};
    constexpr int16_t ec{byteswap(ce)};
    ASSERT_EQ(0x3412, ec);
  }

  {
    constexpr int32_t ce{0x12345678};
    constexpr int32_t ec{byteswap(ce)};
    EXPECT_EQ(0x78563412, ec);
  }

  {
    constexpr int64_t ce{0x12340fedcba95678};
    constexpr int64_t ec{byteswap(ce)};
    EXPECT_EQ(0x7856a9cbed0f3412, ec);
  }

  {
    constexpr volatile int32_t ce{0x12345678};
    const int32_t ec{byteswap(ce)};
    EXPECT_EQ(0x78563412, ec);
  }
}

/* Expected values when swapping doubles, which cannot be done at
 * compile-time due to aliasing violations. */
static constexpr double normal_d64{1.2345678912345599e+46};
static constexpr double swapped_d64{2.7116644990337695e-126};

TEST(ByteOrder, AliasSwappable)
{
  ASSERT_FALSE(details::is_alias_swappable_v<uint32_t>);
  ASSERT_TRUE(details::is_alias_swappable_v<float>);

  const double dv{normal_d64};
  auto vd = byteswap(dv);
  ASSERT_EQ(swapped_d64, vd);
}

TEST(ByteOrder, OtherSwappable)
{
  using array_type = std::array<uint8_t, 4>;
  ASSERT_TRUE(details::is_other_swappable_v<array_type>);
  {
    array_type av{{0, 1, 2, 3}};
    auto va = byteswap(av);
    EXPECT_EQ(av[0], va[3]);
    EXPECT_EQ(av[1], va[2]);
    EXPECT_EQ(av[2], va[1]);
    EXPECT_EQ(av[3], va[0]);
  }

  using v8_type = std::vector<uint8_t>;
  ASSERT_TRUE(details::is_other_swappable_v<v8_type>);
  {
    v8_type av{{0, 1, 2, 3}};
    auto va = byteswap(av);
    EXPECT_EQ(av[0], va[3]);
    EXPECT_EQ(av[1], va[2]);
    EXPECT_EQ(av[2], va[1]);
    EXPECT_EQ(av[3], va[0]);
  }

  /* The overload resolution check succeeds, but the static assertion for octet
   * members fails.  This is the best we can do without std::is_container() or
   * something else that accepts the types we're interested without producing
   * garbage like std::iterator_traits<unsigned int>::iterator_category. */
  using v16_type = std::vector<uint16_t>;
  ASSERT_TRUE(details::is_other_swappable_v<std::vector<uint16_t>>);
  {
    v16_type av{{0, 1, 2, 3}};
    //auto va = byteswap(av);
    (void)av;
  }

  {
    const std::string s1{"abcd"};
    auto s2 = byteswap(s1);
    ASSERT_STREQ("dcba", s2.c_str());
  }

}

#if 0
/* This produces a compile-time error.  Not as helpful a diagnostic as one
 * might want, but a diagnostic. */
TEST(ByteOrder, NonArithmetic)
{
  struct {
    char c[4];
  } vi = { {1,2,3,4} };
  auto vo = byteswap(vi);
  EXPECT_EQ(vi.c[0], vo.c[3]);
  EXPECT_EQ(vi.c[1], vo.c[2]);
  EXPECT_EQ(vi.c[2], vo.c[1]);
  EXPECT_EQ(vi.c[3], vo.c[0]);
}
#endif

TEST(ByteOrder, Endian)
{
  union {
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    uint8_t b[8];
  } u;
  const uint16_t bo16{0x1234};
  const uint32_t bo32{0x12345678};

  u.u16 = host_x_le(bo16);
  EXPECT_EQ(0x34U, u.b[0]);
  EXPECT_EQ(0x12U, u.b[1]);
  u.u16 = host_x_be(bo16);
  EXPECT_EQ(0x12U, u.b[0]);
  EXPECT_EQ(0x34U, u.b[1]);
  u.u16 = be_x_le(u.u16);
  EXPECT_EQ(0x34U, u.b[0]);
  EXPECT_EQ(0x12U, u.b[1]);
  u.u16 = be_x_le(u.u16);
  EXPECT_EQ(0x12U, u.b[0]);
  EXPECT_EQ(0x34U, u.b[1]);

  u.u32 = host_x_le(bo32);
  EXPECT_EQ(0x78U, u.b[0]);
  EXPECT_EQ(0x56U, u.b[1]);
  EXPECT_EQ(0x34U, u.b[2]);
  EXPECT_EQ(0x12U, u.b[3]);
  u.u32 = host_x_be(bo32);
  EXPECT_EQ(0x12U, u.b[0]);
  EXPECT_EQ(0x34U, u.b[1]);
  EXPECT_EQ(0x56U, u.b[2]);
  EXPECT_EQ(0x78U, u.b[3]);
  u.u32 = be_x_le(u.u32);
  EXPECT_EQ(0x78U, u.b[0]);
  EXPECT_EQ(0x56U, u.b[1]);
  EXPECT_EQ(0x34U, u.b[2]);
  EXPECT_EQ(0x12U, u.b[3]);
  u.u32 = be_x_le(u.u32);
  EXPECT_EQ(0x12U, u.b[0]);
  EXPECT_EQ(0x34U, u.b[1]);
  EXPECT_EQ(0x56U, u.b[2]);
  EXPECT_EQ(0x78U, u.b[3]);

  uint16_t ai16 = ::htons(bo16);
  EXPECT_EQ(ai16, host_x_network(bo16));
  ai16 = ::ntohs(bo16);
  EXPECT_EQ(ai16, host_x_network(bo16));

  uint32_t ai32 = ::htonl(bo32);
  EXPECT_EQ(ai32, host_x_network(bo32));
  ai32 = ::ntohl(bo32);
  EXPECT_EQ(ai32, host_x_network(bo32));
}

TEST(ByteOrder, HostOrder)
{
  constexpr byte_order_enum ct_order{
#if __BYTE_ORDER == __LITTLE_ENDIAN
  byte_order_enum::little_endian
#elif __BYTE_ORDER == __BIG_ENDIAN
  byte_order_enum::big_endian
#elif __BYTE_ORDER == __PDP_ENDIAN
  byte_order_enum::pdp_endian
#else /* __BYTE_ORDER */
  /* Brace initialization disallows narrowing so this branch will
   * produce a syntax error. */
  nullptr
#endif /* __BYTE_ORDER */
  };
  constexpr auto hbo = host_byte_order();
  ASSERT_EQ(ct_order, hbo);
};

TEST(ByteOrder, Unicode)
{
  const std::wstring bom{BOM};
  EXPECT_EQ(1U, bom.length());
  EXPECT_EQ(BOM, bom[0]);
}

TEST(ByteOrder, HostSwap)
{
  ASSERT_EQ(byte_order_enum::little_endian, host_byte_order());
  constexpr uint16_t ce{0x1234};
  const double dv{normal_d64};

  constexpr auto ecl = hostswap<decltype(ce), byte_order_enum::little_endian>(ce);
  ASSERT_EQ(ecl, ce);
  constexpr auto ecb = hostswap<decltype(ce), byte_order_enum::big_endian>(ce);
  ASSERT_EQ(ecb, 0x3412);

  const double vdl{hostswap<decltype(dv), byte_order_enum::little_endian>(dv)};
  ASSERT_EQ(vdl, normal_d64);
  const double vdb{hostswap<decltype(dv), byte_order_enum::big_endian>(dv)};
  ASSERT_EQ(vdb, swapped_d64);
}

TEST(ByteOrder, beXle)
{
  ASSERT_EQ(byte_order_enum::little_endian, host_byte_order());
  constexpr uint16_t ce{0x1234};
  const double dv{normal_d64};

  constexpr auto ec = be_x_le(ce);
  ASSERT_EQ(ec, 0x3412);

  const double vd{be_x_le(dv)};
  ASSERT_EQ(vd, swapped_d64);
}

TEST(ByteOrder, hostXe)
{
  ASSERT_EQ(byte_order_enum::little_endian, host_byte_order());
  constexpr uint16_t ce{0x1234};
  const double dv{normal_d64};

  constexpr auto ecl = host_x_le(ce);
  ASSERT_EQ(ecl, ce);
  constexpr auto ecb = host_x_be(ce);
  ASSERT_EQ(ecb, 0x3412);
  constexpr auto ecn = host_x_network(ce);
  ASSERT_EQ(ecn, ecb);

  const double vdl{host_x_le(dv)};
  ASSERT_EQ(vdl, normal_d64);
  const double vdb{host_x_be(dv)};
  ASSERT_EQ(vdb, swapped_d64);
  const double vdn{host_x_network(dv)};
  ASSERT_EQ(vdn, vdb);
}

TEST(ByteOrder, octets_helper)
{
  uint8_t buf[6];
  octets_helper oh{buf, sizeof(buf)};

  ASSERT_EQ(oh.size(), 0U);
  ASSERT_EQ(oh.max_size(), sizeof(buf));
  ASSERT_EQ(oh.available(), sizeof(buf));
  ASSERT_EQ(oh.begin(), buf);
  ASSERT_EQ(oh.end(), buf);

  ASSERT_TRUE(oh.can_advance(0));
  ASSERT_TRUE(oh.can_advance(sizeof(buf)));
  ASSERT_FALSE(oh.can_advance(1 + sizeof(buf)));

  uint32_t u32{0x12345678};
  ASSERT_TRUE(oh.append(u32));
  ASSERT_EQ(oh.size(), sizeof(u32));
  ASSERT_TRUE(std::equal(static_cast<const uint8_t*>(oh.begin()),
                         static_cast<const uint8_t*>(oh.end()),
                         reinterpret_cast<const uint8_t*>(&u32)));

  ASSERT_FALSE(oh.can_advance(sizeof(u32)));
  ASSERT_EQ(oh.advance(sizeof(u32)), nullptr);
  ASSERT_FALSE(oh.valid());
  ASSERT_EQ(oh.begin(), nullptr);
  ASSERT_EQ(oh.end(), nullptr);
  ASSERT_EQ(oh.available(), 0U);
  ASSERT_EQ(oh.size(), 0U);
  ASSERT_EQ(oh.max_size(), sizeof(buf));

  oh.reset();
  ASSERT_TRUE(oh.valid());
  ASSERT_EQ(oh.size(), 0U);
  ASSERT_EQ(oh.max_size(), sizeof(buf));
  ASSERT_EQ(oh.available(), sizeof(buf));
  ASSERT_EQ(oh.begin(), buf);
  ASSERT_EQ(oh.end(), buf);

  oh.append<uint8_t>(0x7D);
  oh.append_be<uint16_t>(0x1234);
  ASSERT_EQ(oh.size(), 3U);
  ASSERT_EQ(buf[0], 0x7D);
  ASSERT_EQ(buf[1], 0x12);
  ASSERT_EQ(buf[2], 0x34);

  oh.invalidate();
  ASSERT_FALSE(oh.valid());
}

} // ns anonymous
