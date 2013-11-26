// SPDX-License-Identifier: Apache-2.0
// Copyright 2018-2019 Peter A. Bigot

#include <algorithm>
#include <cctype>

#include <pabigot/ble/gap.hpp>

namespace {

} // anonymous

namespace pabigot {
namespace ble {
namespace gap {

void
adv_data::set_Flags (unsigned int flags)
{
  /* @todo look for flags above first octet.  There will be some in
   * Bluetooth 5. */
  auto flags8 = static_cast<uint8_t>(flags);
  if (flags8 != flags) {
    return invalidate();
  }
  if (auto raii = start_store(DT_FLAGS, sizeof(flags8))) {
    append(flags8);
  }
}

void
adv_data::set_ShortenedLocalName (const char* name,
                                  ptrdiff_t count)
{
  if (0 > count) {
    count = strlen(name);
  }
  if (auto raii = start_store(DT_SHORTENED_LOCAL_NAME, count)) {
    append(name, count);
  }
}

void
adv_data::set_CompleteLocalName (const char* name,
                                 ptrdiff_t count)
{
  if (0 > count) {
    count = strlen(name);
  }
  if (auto raii = start_store(DT_COMPLETE_LOCAL_NAME, count)) {
    append(name, count);
  }
}

void
adv_data::set_TXPowerLevel (int8_t tx_power)
{
  if (auto raii = start_store(DT_TX_POWER_LEVEL, sizeof(tx_power))) {
    append(tx_power);
  }
}

template <typename uuid_type>
void
adv_data::store_uuids (uint8_t dt,
                       const uuid_type* begin,
                       const uuid_type* end)
{
  const auto span = (end - begin) * begin->size();
  if (auto raii = start_store(dt, span)) {
    append(begin, span);
  }
}

template <>
void
adv_data::set_CompleteListServiceUUID (const uuid16_type* begin,
                                       const uuid16_type* end)
{
  store_uuids(DT_UUID16_COMPLETE, begin, end);
}

template <>
void
adv_data::set_CompleteListServiceUUID (const uuid32_type* begin,
                                       const uuid32_type* end)
{
  store_uuids(DT_UUID32_COMPLETE, begin, end);
}

template <>
void
adv_data::set_CompleteListServiceUUID (const uuid128_type* begin,
                                       const uuid128_type* end)
{
  store_uuids(DT_UUID128_COMPLETE, begin, end);
}

template <>
void
adv_data::set_IncompleteListServiceUUID (const uuid16_type* begin,
                                         const uuid16_type* end)
{
  store_uuids(DT_UUID16_INCOMPLETE, begin, end);
}

template <>
void
adv_data::set_IncompleteListServiceUUID (const uuid32_type* begin,
                                         const uuid32_type* end)
{
  store_uuids(DT_UUID32_INCOMPLETE, begin, end);
}

template <>
void
adv_data::set_IncompleteListServiceUUID (const uuid128_type* begin,
                                         const uuid128_type* end)
{
  store_uuids(DT_UUID128_INCOMPLETE, begin, end);
}

template <>
void
adv_data::set_ListServiceSolicitationUUID (const uuid16_type* begin,
                                           const uuid16_type* end)
{
  store_uuids(DT_SERVICE_SOLICITATION_UUID16, begin, end);
}

template <>
void
adv_data::set_ListServiceSolicitationUUID (const uuid32_type* begin,
                                           const uuid32_type* end)
{
  store_uuids(DT_SERVICE_SOLICITATION_UUID32, begin, end);
}

template <>
void
adv_data::set_ListServiceSolicitationUUID (const uuid128_type* begin,
                                           const uuid128_type* end)
{
  store_uuids(DT_SERVICE_SOLICITATION_UUID128, begin, end);
}

void*
adv_data::set_ManufacturerSpecificData (uint16_t companyID,
                                        size_t span)
{
  void* rv = nullptr;
  if (auto raii = start_store(DT_MANUFACTURER_SPECIFIC_DATA, sizeof(companyID) + span)) {
    append(companyID);
    rv = advance(span);
  }
  return rv;
}

template <>
uint8_t*
adv_data::set_ServiceData (const uuid16_type& uuid,
                           const uint8_t* begin,
                           const uint8_t* end)
{
  const auto span = (end - begin);
  uint8_t* rv = nullptr;
  if (auto raii = start_store(DT_SERVICE_DATA_UUID16, sizeof(uuid) + span)) {
    append(uuid);
    rv = static_cast<uint8_t *>(advance(span));
    memcpy(rv, begin, span);
  }
  return rv;
}

template <>
uint8_t*
adv_data::set_ServiceData (const uuid32_type& uuid,
                           const uint8_t* begin,
                           const uint8_t* end)
{
  const auto span = (end - begin);
  uint8_t* rv = nullptr;
  if (auto raii = start_store(DT_SERVICE_DATA_UUID32, sizeof(uuid) + span)) {
    append(uuid);
    rv = static_cast<uint8_t *>(advance(span));
    memcpy(rv, begin, span);
  }
  return rv;
}

template <>
uint8_t*
adv_data::set_ServiceData (const uuid128_type& uuid,
                           const uint8_t* begin,
                           const uint8_t* end)
{
  const auto span = (end - begin);
  uint8_t* rv = nullptr;
  if (auto raii = start_store(DT_SERVICE_DATA_UUID128, sizeof(uuid) + span)) {
    append(uuid);
    rv = static_cast<uint8_t *>(advance(span));
    memcpy(rv, begin, span);
  }
  return rv;
}

void
adv_data::set_AdvertisingInterval (uint16_t advInterval)
{
  if (auto raii = start_store(DT_ADVERTISING_INTERVAL, sizeof(advInterval))) {
    append(advInterval);
  }
}

void
adv_data::set_SlaveConnectionIntervalRange (uint16_t connIntervalMin,
                                            uint16_t connIntervalMax)
{
  struct {
    uint16_t connIntervalMin;
    uint16_t connIntervalMax;
  } packed;
  packed.connIntervalMin = connIntervalMin;
  packed.connIntervalMax = connIntervalMax;
  if (auto raii = start_store(DT_SLAVE_CONNECTION_INTERVAL_RANGE, sizeof(packed))) {
    append(&packed, sizeof(packed));
  }
}

} // ns gap
} // ns ble
} // ns pabigot
