// SPDX-License-Identifier: CC-BY-SA-4.0
// Copyright 2018 Peter A. Bigot

#include <gtest/gtest.h>

#include <pabigot/ble/gap.hpp>

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

TEST(GAP, StoreHelper)
{
  using namespace pabigot::ble;

  {
    std::array<uint8_t, 3> buf;
    gap::adv_data ad{buf};

    ASSERT_TRUE(ad.valid());
    ASSERT_EQ(ad.size(), 0U);
    ASSERT_EQ(ad.max_size(), buf.max_size());

    const uint8_t flags = 0x06;
    ad.set_Flags(flags);
    ASSERT_EQ(buf[0], 2);
    ASSERT_TRUE(gap::DT_FLAGS == buf[1]);
    ASSERT_EQ(buf[2], flags);
    ASSERT_TRUE(ad.valid());
  }

  {
    /* Diagnose too short */
    std::array<uint8_t, 2> buf;
    gap::adv_data ad{buf};

    ASSERT_TRUE(ad.valid());
    ASSERT_EQ(ad.size(), 0U);
    ASSERT_EQ(ad.max_size(), buf.max_size());
    ad.set_Flags(0);
    ASSERT_FALSE(ad.valid());
  }

  {
    std::array<uint8_t, 12> buf;
    gap::adv_data ad{buf};
    uint32_t v = 0x12345678;
    const uint8_t fv = 0xA5;

    /* Confirm that length defaults to requested count, but is reset to actual
     * length when raii is destructed */
    if (auto raii = ad.start_store(1, 6)) {
      ASSERT_EQ(buf[0], 1 + 6);
      ASSERT_NE(6, sizeof(v));
      ad.append(v);
      ASSERT_EQ(buf[0], 1 + 6);
    }
    ASSERT_EQ(buf[0], 1 + sizeof(v));
    ASSERT_EQ(ad.size(), 1U + 1U + sizeof(v));

    /* Confirm that length does not get corrected if raii does not cover the
     * conditional body. */
    auto bp = buf.data() + ad.size();
    printf("%p targeted\n", bp);
    if (ad.start_store(2U, 3U)) {
      ASSERT_EQ(*bp, 1U + 3U);
      *bp = fv;
      ASSERT_TRUE(ad.append(fv));
      ASSERT_EQ(*bp, fv);
    }
    ASSERT_EQ(*bp, fv);
    ASSERT_EQ(ad.size(), 6 + 3);

    /* Confirm that resetting an invalid buffer while a false store_helper
     * exists does not mutate the buffer.  (Supported by clearing sp in
     * store_helper when reservation fails and testing that along with valid()
     * in the destructor.) */
    bp = buf.data() + ad.size();
    if (auto raii = ad.start_store(3, sizeof(v))) {
      FAIL();
    } else {
      /* Reset while raii is still live, length should not be mutated. */
      ASSERT_FALSE(raii);
      ad.reset();
      *bp = fv;
    }
    ASSERT_EQ(*bp, fv);
  }

}

TEST(GAP, ASRData)
{
  using namespace pabigot::ble;
  uint8_t buffer[BLE_GAP_ADV_MAX_SIZE] = {};
  gap::adv_data ad{buffer, buffer + sizeof(buffer) / sizeof(*buffer)};

  ASSERT_EQ(ad.size(), 0U);
  ASSERT_EQ(ad.max_size(), sizeof(buffer));
  ASSERT_EQ(ad.begin(), static_cast<void *>(buffer));
  ASSERT_EQ(ad.data(), buffer);

  // Flags
  ad.set_Flags(23);
  ASSERT_EQ(ad.size(), 3U);
  ASSERT_TRUE(ad.valid());
  ASSERT_EQ(buffer[0], 2);
  ASSERT_EQ(buffer[1], gap::DT_FLAGS);
  ASSERT_EQ(buffer[2], 23);

  // confirm clear works
  ad.reset();
  ASSERT_EQ(ad.size(), 0U);
  ASSERT_EQ(ad.max_size(), sizeof(buffer));
  ASSERT_EQ(buffer[0], 0);
  ASSERT_EQ(buffer[1], 0);
  ASSERT_EQ(buffer[2], 0);
  ASSERT_TRUE(ad.valid());

  // TX Power Level
  ad.set_TXPowerLevel(-4);
  ASSERT_EQ(ad.size(), 3U);
  ASSERT_TRUE(ad.valid());
  ASSERT_EQ(buffer[0], 2);
  ASSERT_EQ(buffer[1], gap::DT_TX_POWER_LEVEL);
  ASSERT_EQ(buffer[2], 0xFC);

  std::array<uint8_t, 6> serviceData = { 1, 2, 3, 4, 5, 6 };

  { // UUID16 tests
    const uuid16_type uuid{0x1234};
    ASSERT_EQ(uuid[0], 0x34);
    ASSERT_EQ(uuid[1], 0x12);

    ad.reset();
    ad.set_CompleteListServiceUUID(uuid);
    ASSERT_EQ(ad.size(), 4U);
    ASSERT_EQ(buffer[0], 3);
    ASSERT_EQ(buffer[1], gap::DT_UUID16_COMPLETE);
    ASSERT_EQ(buffer[2], 0x34);
    ASSERT_EQ(buffer[3], 0x12);
    ad.reset();
    ad.set_IncompleteListServiceUUID(uuid);
    ASSERT_EQ(ad.size(), 4U);
    ASSERT_EQ(buffer[0], 3);
    ASSERT_EQ(buffer[1], gap::DT_UUID16_INCOMPLETE);
    ASSERT_EQ(buffer[2], 0x34);
    ASSERT_EQ(buffer[3], 0x12);
    ad.reset();
    ad.set_ListServiceSolicitationUUID(uuid);
    ASSERT_EQ(ad.size(), 4U);
    ASSERT_EQ(buffer[0], 3);
    ASSERT_EQ(buffer[1], gap::DT_SERVICE_SOLICITATION_UUID16);
    ASSERT_EQ(buffer[2], 0x34);
    ASSERT_EQ(buffer[3], 0x12);
    ad.reset();
    auto dp = ad.set_ServiceData(uuid, serviceData);
    ASSERT_EQ(ad.size(), 10U);
    ASSERT_EQ(buffer[0], 9);
    ASSERT_EQ(buffer[1], gap::DT_SERVICE_DATA_UUID16);
    ASSERT_EQ(buffer[2], 0x34);
    ASSERT_EQ(buffer[3], 0x12);
    ASSERT_EQ(dp, buffer + 4);
    ASSERT_TRUE(std::equal(serviceData.begin(), serviceData.end(), dp));
  }

  { // UUID32 tests
    const std::array<uint8_t, 4> arr = UUID32_BRACE_INITIALIZER;
    const uuid32_type uuid{arr};
    ad.reset();
    ad.set_CompleteListServiceUUID(uuid);
    ASSERT_EQ(ad.size(), 6U);
    ASSERT_EQ(buffer[0], 5);
    ASSERT_EQ(buffer[1], gap::DT_UUID32_COMPLETE);
    ASSERT_TRUE(std::equal(arr.begin(), arr.end(), buffer + 2));
    ad.reset();
    ad.set_IncompleteListServiceUUID(uuid);
    ASSERT_EQ(ad.size(), 6U);
    ASSERT_EQ(buffer[0], 5);
    ASSERT_EQ(buffer[1], gap::DT_UUID32_INCOMPLETE);
    ASSERT_TRUE(std::equal(arr.begin(), arr.end(), buffer + 2));
    ad.reset();
    ad.set_ListServiceSolicitationUUID(uuid);
    ASSERT_EQ(ad.size(), 6U);
    ASSERT_EQ(buffer[0], 5);
    ASSERT_EQ(buffer[1], gap::DT_SERVICE_SOLICITATION_UUID32);
    ASSERT_TRUE(std::equal(arr.begin(), arr.end(), buffer + 2));
    ad.reset();
    auto dp = ad.set_ServiceData(uuid, serviceData);
    ASSERT_EQ(ad.size(), 12U);
    ASSERT_EQ(buffer[0], 11);
    ASSERT_EQ(buffer[1], gap::DT_SERVICE_DATA_UUID32);
    ASSERT_TRUE(std::equal(arr.begin(), arr.end(), buffer + 2));
    ASSERT_EQ(dp, buffer + 6);
    ASSERT_TRUE(std::equal(serviceData.begin(), serviceData.end(), dp));
  }

  { // UUID128 tests
    const std::array<uint8_t, 16> arr = UUID128_BRACE_INITIALIZER;
    const uuid128_type uuid{arr};
    ad.reset();
    ad.set_CompleteListServiceUUID(uuid128_type{arr});
    ASSERT_EQ(ad.size(), 18U);
    ASSERT_EQ(buffer[0], 17);
    ASSERT_EQ(buffer[1], gap::DT_UUID128_COMPLETE);
    ASSERT_TRUE(std::equal(arr.begin(), arr.end(), buffer + 2));
    ad.reset();
    ad.set_IncompleteListServiceUUID(uuid128_type{arr});
    ASSERT_EQ(ad.size(), 18U);
    ASSERT_EQ(buffer[0], 17);
    ASSERT_EQ(buffer[1], gap::DT_UUID128_INCOMPLETE);
    ASSERT_TRUE(std::equal(arr.begin(), arr.end(), buffer + 2));
    ad.reset();
    ad.set_ListServiceSolicitationUUID(uuid128_type{arr});
    ASSERT_EQ(ad.size(), 18U);
    ASSERT_EQ(buffer[0], 17);
    ASSERT_EQ(buffer[1], gap::DT_SERVICE_SOLICITATION_UUID128);
    ASSERT_TRUE(std::equal(arr.begin(), arr.end(), buffer + 2));
    ad.reset();
    auto dp = ad.set_ServiceData(uuid, serviceData);
    ASSERT_TRUE(ad.valid());
    ASSERT_EQ(ad.size(), 24U);
    ASSERT_EQ(buffer[0], 23);
    ASSERT_EQ(buffer[1], gap::DT_SERVICE_DATA_UUID128);
    ASSERT_TRUE(std::equal(arr.begin(), arr.end(), buffer + 2));
    ASSERT_EQ(dp, buffer + 18);
    ASSERT_TRUE(std::equal(serviceData.begin(), serviceData.end(), dp));
    ad.reset();
    dp = ad.set_ServiceData(uuid, &serviceData, sizeof(serviceData));
    ASSERT_TRUE(ad.valid());
    ASSERT_EQ(ad.size(), 24U);
    ASSERT_EQ(buffer[0], 23);
    ASSERT_EQ(buffer[1], gap::DT_SERVICE_DATA_UUID128);
    ASSERT_TRUE(std::equal(arr.begin(), arr.end(), buffer + 2));
    ASSERT_EQ(dp, buffer + 18);
    ASSERT_TRUE(std::equal(serviceData.begin(), serviceData.end(), dp));
  }

  { // Advertising Interval
    ad.reset();
    ad.set_AdvertisingInterval(0x1234);
    ASSERT_EQ(ad.size(), 4U);
    ASSERT_EQ(buffer[0], 3);
    ASSERT_EQ(buffer[1], gap::DT_ADVERTISING_INTERVAL);
    ASSERT_EQ(buffer[2], 0x34);
    ASSERT_EQ(buffer[3], 0x12);
    ASSERT_TRUE(ad.valid());
  }

  { // Slave Connection Interval Range
    ad.reset();
    ad.set_SlaveConnectionIntervalRange(0x0006, 0x0C80);
    ASSERT_EQ(ad.size(), 6U);
    ASSERT_EQ(buffer[0], 5);
    ASSERT_EQ(buffer[1], gap::DT_SLAVE_CONNECTION_INTERVAL_RANGE);
    ASSERT_EQ(buffer[2], 0x06);
    ASSERT_EQ(buffer[3], 0x00);
    ASSERT_EQ(buffer[4], 0x80);
    ASSERT_EQ(buffer[5], 0x0C);
    ASSERT_TRUE(ad.valid());
  }

  { // Local Name
    const char name[] = "MyD\0Nul";
    ASSERT_EQ(strlen(name), 3U);
    ASSERT_EQ(name[3], 0);
    ASSERT_EQ(sizeof(name), 8U);

    ad.reset();
    ad.set_ShortenedLocalName(name);
    ASSERT_EQ(ad.size(), 5U);
    ASSERT_EQ(buffer[0], 4);
    ASSERT_EQ(buffer[1], gap::DT_SHORTENED_LOCAL_NAME);
    ASSERT_EQ(buffer[2], name[0]);
    ASSERT_EQ(buffer[3], name[1]);
    ASSERT_EQ(buffer[4], name[2]);

    ad.reset();
    ad.set_ShortenedLocalName(name, 1);
    ASSERT_EQ(ad.size(), 3U);
    ASSERT_EQ(buffer[0], 2);
    ASSERT_EQ(buffer[1], gap::DT_SHORTENED_LOCAL_NAME);
    ASSERT_EQ(buffer[2], name[0]);
    ASSERT_EQ(buffer[3], 0);

    ad.reset();
    ad.set_CompleteLocalName(name);
    ASSERT_EQ(ad.size(), 5U);
    ASSERT_EQ(buffer[0], 4);
    ASSERT_EQ(buffer[1], gap::DT_COMPLETE_LOCAL_NAME);
    ASSERT_EQ(buffer[2], name[0]);
    ASSERT_EQ(buffer[3], name[1]);
    ASSERT_EQ(buffer[4], name[2]);

    ad.reset();
    ad.set_CompleteLocalName(name, sizeof(name) - 1);
    ASSERT_EQ(ad.size(), 9U);
    ASSERT_EQ(buffer[0], 8);
    ASSERT_EQ(buffer[1], gap::DT_COMPLETE_LOCAL_NAME);
    ASSERT_EQ(memcmp(buffer + 2, name, sizeof(name) - 1), 0);
  }

  { // Manufacturer Specific Data
    ad.reset();
    auto dp = ad.set_ManufacturerSpecificData(0x1234, sizeof(serviceData));
    ASSERT_TRUE(ad.valid());
    ASSERT_EQ(ad.size(), 10U);
    ASSERT_EQ(buffer[0], 9);
    ASSERT_EQ(buffer[1], gap::DT_MANUFACTURER_SPECIFIC_DATA);
    ASSERT_EQ(buffer[2], 0x34);
    ASSERT_EQ(buffer[3], 0x12);
    ASSERT_EQ(dp, buffer + 4);
  }

}

} // ns anonymous
