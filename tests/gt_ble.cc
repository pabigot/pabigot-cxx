// SPDX-License-Identifier: CC-BY-SA-4.0
// Copyright 2018-2019 Peter A. Bigot

#include <algorithm>

#include <gtest/gtest.h>

#include <pabigot/ble.hpp>

std::string operator"" _s (const char* p, size_t n)
{
  return std::string(p, n);
}

#define BLE_GAP_ADV_MAX_SIZE 31

#define UUID16_BRACE_INITIALIZER {0x34, 0x12}
#define UUID32_BRACE_INITIALIZER {0x78, 0x56, 0x34, 0x12}
#define UUID128_BRACE_INITIALIZER { \
    0x56, 0x55, 0x54, 0x53,         \
    0x52, 0x51, 0x42, 0x41,         \
    0x32, 0x31, 0x22, 0x21,         \
    0x14, 0x13, 0x12, 0x11,         \
  }

namespace {

TEST(clkType, count)
{
  using namespace pabigot::ble;
  using namespace std::literals::chrono_literals;

  ASSERT_EQ(std::chrono::duration_cast<clk1_type>(625us).count(), 1U);
  ASSERT_EQ(std::chrono::duration_cast<clk1_type>(1250us).count(), 2U);
  ASSERT_EQ(std::chrono::duration_cast<clk2_type>(1250us).count(), 1U);
  ASSERT_EQ(std::chrono::duration_cast<clk2_type>(10ms).count(), 8U);
  ASSERT_EQ(std::chrono::duration_cast<clk5_type>(10ms).count(), 1U);
  ASSERT_EQ(std::chrono::duration_cast<clk5_type>(200ms).count(), 20U);
}

// Expect little-endian storage of 0x1234
void match16_1234 (const pabigot::ble::uuid16_type& u16)
{
  ASSERT_EQ(u16[0], 0x34);
  ASSERT_EQ(u16[1], 0x12);
}

TEST(BLE, uuid16)
{
  using namespace pabigot::ble;
  SCOPED_TRACE("uuid16_type ctors");

#define BRACE_INITIALIZER UUID16_BRACE_INITIALIZER

  { // default
    uuid16_type u16;
    ASSERT_EQ(u16[0], 0x00);
    ASSERT_EQ(u16[1], 0x00);
  }

  { // from const built-in array
    const uint8_t raw[] = BRACE_INITIALIZER;
    uuid16_type u16{raw};
  }

  { // from mutable built-in array
    uint8_t raw[] = BRACE_INITIALIZER;
    match16_1234(uuid16_type{raw});
  }

  { // from const std::array instance
    const std::array<uint8_t, 2> arr = BRACE_INITIALIZER;
    match16_1234(uuid16_type{arr});
  }

  { // from mutable std::array instance
    std::array<uint8_t, 2> arr = BRACE_INITIALIZER;
    match16_1234(uuid16_type{arr});
  }

  { // from/to compatible integer literal
    uint16_t v16{0x1234};
    uuid16_type u16{v16};
    match16_1234(u16);

    ASSERT_EQ(v16, u16.as_integer());

    auto u16b = u16;
    ASSERT_TRUE(std::equal(u16b.begin(), u16b.end(), u16.begin()));
  }

  { // from/to text
    uint16_t v16{0x1AB2};
    uuid16_type u16{v16};
    ASSERT_EQ(u16.as_string(), "1ab2"_s);
  }

#if (WILL_ERROR - 0)
  { // from incompatible integer literal (narrowing)
    uuid16_type u16{0x123456};
    ASSERT_EQ(u16[0], 0x56);
    ASSERT_EQ(u16[1], 0x34);

    auto u16b = u16;
    ASSERT_TRUE(std::equal(u16b.begin(), u16b.end(), u16.begin()));
  }
#endif

#undef BRACE_INITIALIZER
}

// Expect little-endian storage of 0x12345678
void match32_1234 (const pabigot::ble::uuid32_type& u32)
{
    ASSERT_EQ(u32[0], 0x78);
    ASSERT_EQ(u32[1], 0x56);
    ASSERT_EQ(u32[2], 0x34);
    ASSERT_EQ(u32[3], 0x12);
}

TEST(BLE, uuid32)
{
  using namespace pabigot::ble;
  SCOPED_TRACE("uuid32_type ctors");

#define BRACE_INITIALIZER UUID32_BRACE_INITIALIZER

  { // default
    uuid32_type u32;
    ASSERT_EQ(u32[0], 0x00);
    ASSERT_EQ(u32[1], 0x00);
    ASSERT_EQ(u32[2], 0x00);
    ASSERT_EQ(u32[3], 0x00);
  }

  { // from const built-in array
    const uint8_t raw[] = BRACE_INITIALIZER;
    match32_1234(raw);
  }

  { // from mutable built-in array
    uint8_t raw[] = BRACE_INITIALIZER;
    match32_1234(raw);
  }

  { // from const std::array instance
    const std::array<uint8_t, 4> arr = BRACE_INITIALIZER;
    match32_1234(arr);
  }

  { // from mutable std::array instance
    std::array<uint8_t, 4> arr = BRACE_INITIALIZER;
    match32_1234(arr);
  }

  { // from compatible integer literal
    const uint32_t lit = 0x12345678;
    uuid32_type u32{lit};
    match32_1234(u32);
    ASSERT_EQ(lit, u32.as_integer());

    auto u32b = u32;
    ASSERT_TRUE(std::equal(u32b.begin(), u32b.end(), u32.begin()));
  }

  { // from/to text
    uint32_t v32{0x1ABCDEF4};
    uuid32_type u32{v32};
    ASSERT_EQ(u32.as_string(), "1abcdef4"_s);
  }

#undef BRACE_INITIALIZER
}

TEST(BLE, uuid128)
{
  using namespace pabigot::ble;

  ASSERT_STREQ("00000000-0000-1000-8000-00805f9b34fb", uuid128_type::BLUETOOTH_BASE.as_string().c_str());

#define BRACE_INITIALIZER UUID128_BRACE_INITIALIZER

  { // default
    const uint8_t zeroed[16] = {};
    uuid128_type u128;
    ASSERT_TRUE(std::equal(u128.begin(), u128.end(), zeroed));
  }

  { // from const built-in array
    const uint8_t raw[] = BRACE_INITIALIZER;
    uuid128_type u128{raw};
    ASSERT_TRUE(std::equal(u128.begin(), u128.end(), raw));
    ASSERT_STREQ("11121314-2122-3132-4142-515253545556", u128.as_string().c_str());
  }

  { // from mutable built-in array
    uint8_t raw[] = BRACE_INITIALIZER;
    uuid128_type u128{raw};
    ASSERT_TRUE(std::equal(u128.begin(), u128.end(), raw));
  }

  { // from const std::array instance
    const std::array<uint8_t, 16> arr = BRACE_INITIALIZER;
    uuid128_type u128{arr};
    ASSERT_TRUE(std::equal(u128.begin(), u128.end(), arr.data()));
  }

  { // from mutable std::array instance
    std::array<uint8_t, 16> arr = BRACE_INITIALIZER;
    uuid128_type u128{arr};
    ASSERT_TRUE(std::equal(u128.begin(), u128.end(), arr.data()));
  }

  { // Derived 16-bit UUIDs and base match */
    uuid128_type u128{BRACE_INITIALIZER};
    ASSERT_STREQ("11121314-2122-3132-4142-515253545556", u128.as_string().c_str());
    uint16_t uuid{0xABCD};
    auto u16 = u128.from_uuid16(uuid);
    ASSERT_STREQ("1112abcd-2122-3132-4142-515253545556", u16.as_string().c_str());
    ASSERT_EQ(uuid, u16.uuid16());

    ASSERT_TRUE(u128.base_match(u16));
    ASSERT_TRUE(u16.base_match(u128));

    u16[14] = 0x00;
    ASSERT_STREQ("1100abcd-2122-3132-4142-515253545556", u16.as_string().c_str());
    ASSERT_FALSE(u128.base_match(u16));
    u16[14] = 0x12;
    u16[11] = 0x00;
    ASSERT_STREQ("1112abcd-0022-3132-4142-515253545556", u16.as_string().c_str());
    ASSERT_FALSE(u128.base_match(u16));
  }

  { // Reversing the UUID
    uuid128_type u128{BRACE_INITIALIZER};
    ASSERT_STREQ("11121314-2122-3132-4142-515253545556", u128.as_string().c_str());
    auto r128 = u128.swap_endian();
    ASSERT_STREQ("56555453-5251-4241-3231-222114131211", r128.as_string().c_str());
  }

#undef BRACE_INITIALIZER
}

} // ns anonymous
