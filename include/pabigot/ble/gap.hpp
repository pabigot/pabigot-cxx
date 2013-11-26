/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2017-2019 Peter A. Bigot */

/** Bluetooth Low Energy General Access Profile support.
 *
 * @file */

#ifndef PABIGOT_BLE_GAP_HPP
#define PABIGOT_BLE_GAP_HPP
#pragma once

#include <pabigot/ble.hpp>
#include <pabigot/byteorder.hpp>

namespace pabigot {
namespace ble {

/** Bluetooth Low Energy General Access Profile material.
 *
 * Including Advertising and Scan Response infrastructure.
 *
 * @note This material in this module is kept independent of implementation.
 * It should support both Nordic Soft Device APIs and Bluez. */
namespace gap {

/** Number of octets available for a standard advertising or scan response data
 * packet. */
static constexpr size_t ASR_DATA_SIZE{31};

/** Advertising PDU type.
 *
 * Names used are for the primary advertising interpretation of the flags.
 *
 * @see BT-5v6B2.3 */
enum pdu_type_e : uint8_t
{
  PT_ADV_IND = 0x00,
  PT_ADV_DIRECT_IND = 0x01,
  PT_ADV_NONCONN_IND = 0x02,
  PT_SCAN_REQ = 0x03,           // also AUX_SCAN_REQ
  PT_SCAN_RESP = 0x04,
  PT_CONNECT_IND = 0x05,        // also AUX_CONNECT_REQ
  PT_ADV_SCAN_IND = 0x06,
  PT_ADV_EXT_IND = 0x07,        // also AUX_{ADV,SYNC,CHAIN}_IND, AUX_SCAN_RSP
  PT_AUX_CONNECT_RSP = 0x08,
};

/** Advertising Event Type.
 *
 * @see BT-5v2E7.7.65.2 */
enum adv_event_type_e : uint8_t
{
  ET_ADV_IND = 0x00,
  ET_ADV_DIRECT_IND = 0x01,
  ET_ADV_SCAN_IND = 0x02,
  ET_ADV_NONCONN_IND = 0x03,
  ET_SCAN_RSP = 0x04,

  /** Indicates an invalid event type flag.
   *
   * This should only appear when processing ASR packets from a source that
   * does not provide a clean Event Type field, such as the Minew G1 (which
   * appears to provide the event type shifted down 3 bits to throw away all
   * the interesting information). */
  ET_INVALID = 0x80,
};

/** Bits and values for @ref DT_FLAGS.
 *
 * You want to assign one of the DISCOVERABLE values. */
enum flags_data_type_e : uint8_t
{
  FDT_LE_LIMITED = 0x01,
  FDT_LE_GENERAL = 0x02,
  FDT_BREDR_NOTSUP = 0x04,
  FDT_BREDRLE_CTRL = 0x08,
  FDT_BREDRLE_HOST = 0x10,

  /** BT-5v3C9.2.2 LE-only not discoverable */
  FDT_LE_NON_DISCOVERABLE = FDT_BREDR_NOTSUP,
  /** BT-5v3C9.2.3 LE-only short-term discoverable */
  FDT_LE_LIMITED_DISCOVERABLE = FDT_BREDR_NOTSUP | FDT_LE_LIMITED,
  /** BT-5v3C9.2.4 LE-only long-term discoverable */
  FDT_LE_GENERAL_DISCOVERABLE = FDT_BREDR_NOTSUP | FDT_LE_GENERAL,
};

/** GAP assigned numbers for data type values.
 *
 * For type codes see [assigned
 * numbers](https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile).
 *
 * For structure contents see Core Specification Supplement volume A. */
enum data_type_e : uint8_t
{
  /** adv_data::set_Flags() CSSv7A1.3 */
  DT_FLAGS = 0x01,
  /** adv_data::set_IncompleteListServiceUUID() */
  DT_UUID16_INCOMPLETE = 0x02,
  /** adv_data::set_CompleteListServiceUUID() */
  DT_UUID16_COMPLETE = 0x03,
  /** adv_data::set_IncompleteListServiceUUID() */
  DT_UUID32_INCOMPLETE = 0x04,
  /** adv_data::set_CompleteListServiceUUID() */
  DT_UUID32_COMPLETE = 0x05,
  /** adv_data::set_IncompleteListServiceUUID() */
  DT_UUID128_INCOMPLETE = 0x06,
  /** adv_data::set_CompleteListServiceUUID() */
  DT_UUID128_COMPLETE = 0x07,
  /** adv_data::set_ShortenedLocalName() */
  DT_SHORTENED_LOCAL_NAME = 0x08,
  /** adv_data::set_CompleteLocalName() */
  DT_COMPLETE_LOCAL_NAME = 0x09,
  /** adv_data::set_TXPowerLevel */
  DT_TX_POWER_LEVEL = 0x0A,
  DT_CLASS_OF_DEVICE = 0x0D,
  DT_SIMPLE_PAIRING_HASH_C192 = 0x0E,
  DT_SIMPLE_PAIRING_RANDOMIZER_R192 = 0x0F,
  DT_DEVICE_ID = 0x10,
  DT_SECURITY_MANAGER_OOB_FLAGS = 0x11,
  /** adv_data::set_SlaveConnectionIntervalRange */
  DT_SLAVE_CONNECTION_INTERVAL_RANGE = 0x12,
  /** adv_data::set_ListServiceSolicitationUUID() */
  DT_SERVICE_SOLICITATION_UUID16 = 0x14,
  /** adv_data::set_ListServiceSolicitationUUID() */
  DT_SERVICE_SOLICITATION_UUID128 = 0x15,
  /** adv_data::set_ServiceData() */
  DT_SERVICE_DATA_UUID16 = 0x16,
  DT_PUBLIC_TARGET_ADDRESS = 0x17,
  DT_RANDOM_TARGET_ADDRESS = 0x18,
  DT_APPEARANCE = 0x19,
  /** adv_data::set_AdvertisingInterval */
  DT_ADVERTISING_INTERVAL = 0x1A,
  DT_LE_BLUETOOTH_DEVICE_ADDR = 0x1B,
  DT_LE_ROLE = 0x1C,
  DT_SIMPLE_PAIRING_HASH_C256 = 0x1D,
  DT_SIMPLE_PAIRING_RANDOMIZER_R256 = 0x1E,
  /** adv_data::set_ListServiceSolicitationUUID() */
  DT_SERVICE_SOLICITATION_UUID32 = 0x1F,
  /** adv_data::set_ServiceData() */
  DT_SERVICE_DATA_UUID32 = 0x20,
  /** adv_data::set_ServiceData() */
  DT_SERVICE_DATA_UUID128 = 0x21,
  DT_LE_SECURE_CONN_CONFIRM_VALUE = 0x22,
  DT_LE_SECURE_CONN_RANDOM_VALUE = 0x23,
  DT_URI = 0x24,
  DT_INDOOR_POSITIONING = 0x25,
  DT_TRANSPORT_DISCOVERY_DATA = 0x26,
  DT_LE_SUPPORTED_FEATURES = 0x27,
  DT_CHANNEL_MAP_UPDATE_INDICATION = 0x28,
  DT_PB_ADV = 0x29,
  DT_MESH_MESSAGE = 0x2A,
  DT_MESH_BEACON = 0x2B,
  DT_3D_INFORMATION_DATA = 0x3D,
  /** adv_data::set_ManufacturerSpecificData */
  DT_MANUFACTURER_SPECIFIC_DATA = 0xFF,
};

/** Infrastructure to fill in Advertising and Scan Response
 * Data.
 *
 * @note Not all AD and EIR structures are implemented.
 *
 * For assigned numbers and references to data representation see:
 * https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile
 */
class adv_data : public byteorder::octets_helper
{
protected:
  using super = byteorder::octets_helper;

  /** RAII helper to avoid overruns and to back-fill lengths.
   *
   * The constructor checks whether the expected length would exceed the
   * available space, and if so marks the owning buffer as invalid.  Otherwise
   * it stores the expected length and type tag.
   *
   * The destructor verifies that the owning buffer remains valid, and if
   * content was added after the constructor updates the length octet to record
   * the space used.
   *
   * See usage at start_store(). */
  class store_helper
  {
  private:
    friend class adv_data;

    adv_data& owner;
    uint8_t* sp;

    store_helper (adv_data& owner_,
                  uint8_t tag,
                  size_type count) :
      owner{owner_},
      sp{owner.bp_}
    {
      // Increase span to cover the type tag.
      auto span = count + sizeof(tag);
      auto span_octet = static_cast<uint8_t>(span);

      // Valid requires representable span and available space
      if ((span_octet == span)
          && owner.can_advance(sizeof(*sp) + span)) {
        // Store the expected length and the tag
        owner.append(span_octet);
        owner.append(tag);
      } else {
        sp = nullptr;
        owner.invalidate();
      }
    }
  public:
    /** `true` iff the requested space was available on construction. */
    explicit operator bool() const noexcept
    {
      return sp;
    }

    /** Destructor updates length field if content was added since
     * construction. */
    ~store_helper ()
    {
      /* If the owner is valid, construction succeeded, and the pointer
       * advanced past where it was left in construction, then we can and
       * should update length to match the used region. */
      if (owner.valid()
          && sp
          && ((sp + 2U) != owner.bp_)) {
        /* Length is anything past the length octet.  Invalidate if the span
         * doesn't fit in an octet. */
        auto span = owner.bp_ - sp - 1U;
        if (static_cast<uint8_t>(span) == span) {
          *sp = span;
        } else {
          owner.invalidate();
        }
      }
    }
  };

  template <typename uuid_type>
  void store_uuids (uint8_t dt,
                    const uuid_type* begin,
                    const uuid_type* end);

public:
  /** Reference an octet sequence into which advertising data
   * will be written.
   *
   * @param begin pointer to the first octet in the sequence.
   *
   * @param end pointer just past the last octet available for the
   * sequence. */
  adv_data (uint8_t* begin,
            uint8_t* end) :
    super{begin, end}
  { }

  /** Construct from a `std::array` reference. */
  template <size_t count>
  adv_data (std::array<uint8_t, count>& src) :
    super{src.begin(), src.end()}
  { }

  /** Get a typed pointer to the start of the buffer.
   *
   * APIs that consume the prepared ASR packet generally want to receive it as
   * a pointer to an octet. */
  const uint8_t* data () const noexcept
  {
    return static_cast<const uint8_t *>(begin());
  }

  /** Reserve space for a data type record.
   *
   * This function checks whether there is enough space in the buffer for a
   * data type with the given tag and length.  If so, it stores the length and
   * tag; if no, the buffer is invalidated.
   *
   * Use it this way if @p count is a fixed span:
   *
   *     if (start_store(DT_WHATEVER, count)) {
   *        append(count_octets);
   *     }
   *
   * Use it this way if @p count is a maximum span and the actual space
   * required may be less:
   *
   *     if (auto raii = start_store(DT_WHATEVER, count)) {
   *        append(whatever_up_to_count);
   *     }
   *
   * @param tag the data type tag identifying the content to be stored.
   *
   * @param count the number of octets of data required for the record,
   * excluding the data type tag.
   *
   * @return an object that is `true` when converted to `bool` iff the
   * reservation succeeded.  When the object is destructed the length of the
   * data type record is updated to reflect the actual amount of space used if
   * content was added after this function was called. */
  [[nodiscard]] store_helper start_store (uint8_t tag,
                                          size_type count)
  {
    return {*this, tag, count};
  }

  /** Helper to set the Flags data value
   *
   * @param flags providing LE and BR/EDR modes and features */
  void set_Flags (unsigned int flags);

  /** Helper to set the shortened local name.
   *
   * @param name pointer to the name, as a UTF8 character sequence.
   *
   * @param count number of octets in the name.  If a negative value
   * is provided then `strlen(name)` will be used. */
  void set_ShortenedLocalName (const char* name,
                               ptrdiff_t count = -1);

  /** Helper to set the complete local name.
   *
   * @param name pointer to the name.
   *
   * @param name pointer to the name, as a UTF8 character sequence.
   *
   * @param count number of octets in the name.  If a negative value
   * is provided then `strlen(name)` will be used. */
  void set_CompleteLocalName (const char* name,
                              ptrdiff_t count = -1);

  /** Helper to set the TX Power Level data value
   *
   * @param txPower transmission power level in dBm. */
  void set_TXPowerLevel (int8_t txPower);

  /** Helper for complete listing of a single UUID service
   *
   * @tparam uuid_type one of uuid16_type, uuid32_type, or @link
   * details::uuid_type uuid128_type@endlink.
   *
   * @param uuid the UUID to be stored in the structure. */
  template <typename uuid_type>
  void set_CompleteListServiceUUID (const uuid_type& uuid)
  {
    return set_CompleteListServiceUUID(&uuid, 1 + &uuid);
  }

  /** Helper for complete listing of multiple UUID services
   *
   * @tparam uuid_type one of uuid16_type, uuid32_type, or @link
   * details::uuid_type uuid128_type@endlink.
   *
   * @param begin start of the range providing the service UUIDs.
   *
   * @param end end of the range providing the service UUIDs. */
  template <typename uuid_type>
  void set_CompleteListServiceUUID (const uuid_type* begin,
                                    const uuid_type* end);

  /** Helper for incomplete listing a single UUID service
   *
   * @tparam uuid_type one of uuid16_type, uuid32_type, or @link
   * details::uuid_type uuid128_type@endlink.
   *
   * @param uuid the UUID to be stored in the structure. */
  template <typename uuid_type>
  void set_IncompleteListServiceUUID (const uuid_type& uuid)
  {
    return set_IncompleteListServiceUUID(&uuid, 1 + &uuid);
  }

  /** Helper for incomplete listing of multiple UUID services
   *
   * @tparam uuid_type one of uuid16_type, uuid32_type, or @link
   * details::uuid_type uuid128_type@endlink.
   *
   * @param begin start of the range providing the service UUIDs.
   *
   * @param end end of the range providing the service UUIDs. */
  template <typename uuid_type>
  void set_IncompleteListServiceUUID (const uuid_type* begin,
                                      const uuid_type* end);

  /** Helper for listing a single service solicitation UUID
   *
   * @tparam uuid_type one of uuid16_type, uuid32_type, or @link
   * details::uuid_type uuid128_type@endlink.
   *
   * @param uuid the UUID to be stored in the structure. */
  template <typename uuid_type>
  void set_ListServiceSolicitationUUID (const uuid_type& uuid)
  {
    return set_ListServiceSolicitationUUID(&uuid, 1 + &uuid);
  }

  /** Helper for listing multiple service solicitation UUIDs
   *
   * @tparam uuid_type one of uuid16_type, uuid32_type, or @link
   * details::uuid_type uuid128_type@endlink.
   *
   * @param begin start of the range providing the service UUIDs.
   *
   * @param end end of the range providing the service UUIDs. */
  template <typename uuid_type>
  void set_ListServiceSolicitationUUID (const uuid_type* begin,
                                        const uuid_type* end);

  /** Helper to set service data
   *
   * @tparam uuid_type one of uuid16_type, uuid32_type, or @link
   * details::uuid_type uuid128_type@endlink.
   *
   * @param begin start of the service data to be stored in the payload.
   *
   * @param end end of the service data to be stored in the payload
   *
   * @return pointer into the ASR data region at which `data` was
   * copied.  This can be used to update the advertising payload when
   * the data changes, as long as the data length does not change. */
  template <typename uuid_type>
  uint8_t* set_ServiceData (const uuid_type& uuid,
                            const uint8_t* begin,
                            const uint8_t* end);

  /** Helper to set service data
   *
   * @tparam uuid_type one of uuid16_type, uuid32_type, or @link
   * details::uuid_type uuid128_type@endlink.
   *
   * @param data the data to be sent
   *
   * @return pointer into the ASR data region at which `data` was
   * copied.  This can be used to update the advertising payload when
   * the data changes. */
  template <typename uuid_type,
            std::size_t len>
  uint8_t* set_ServiceData (const uuid_type& uuid,
                            const std::array<uint8_t, len>& data)
  {
    return set_ServiceData(uuid, data.begin(), data.end());
  }

  /** Helper to set service data
   *
   * @tparam uuid_type one of uuid16_type, uuid32_type, or @link
   * details::uuid_type uuid128_type@endlink.
   *
   * @param buf pointer to the data to be sent
   *
   * @param count the number of bytes to be sent
   *
   * @return pointer into the ASR data region at which `data` was
   * copied.  This can be used to update the advertising payload when
   * the data changes. */
  template <typename uuid_type>
  uint8_t* set_ServiceData (const uuid_type& uuid,
                            const void* buf,
                            size_t count)
  {
    auto ptr = static_cast<const uint8_t*>(buf);
    return set_ServiceData(uuid, ptr, ptr + count);
  }

  /** Helper to set the Advertising Interval data value.
   *
   * @param advInterval interval between advertisements, measured in
   * 625 us ticks.  Valid range is 32 (20 ms) to 65535 (about 41 s). */
  void set_AdvertisingInterval (uint16_t advInterval);

  /** Helper to set the preferred connectional interval range.
   *
   * @param connIntervalMin minimum value for connection interval,
   * measured in 1250 us ticks.  Allowed (but unverified) range is
   * 0x0006 through 0x0C80 inclusive.  Pass -1 for no specific
   * minimum.
   *
   * @param connIntervalMax maximum value for connection interval,
   * measured in 1250 us ticks.  Allowed (but unverified) range is
   * 0x0006 through 0x0C80 inclusive, and a non-negative value must
   * not be less than `connIntervalMin`. Pass -1 for no specific
   * maximum.  interval between advertisements, measured in 625 us
   * ticks. */
  void set_SlaveConnectionIntervalRange (uint16_t connIntervalMin,
                                         uint16_t connIntervalMax);

  /** Helper to set Manufacturer Specific Data.
   *
   * @note The returned pointer is aligned to the requirements of the
   * advertising PDU.  If octet-level access is not desired the object accessed
   * though the pointer should be
   * [packed](https://gcc.gnu.org/onlinedocs/gcc-8.1.0/gcc/Common-Type-Attributes.html#index-packed-type-attribute).
   *
   * @param companyID the assigned company identifier.  Pass `-1` for the
   * reserved test identifier.
   *
   * @param span number of octets to be stored in the payload.
   *
   * @return pointer into the ASR data region at which the data starts.  This
   * should be used to set the advertising payload initially and when the data
   * changes, as long as the data length does not change. */
  void* set_ManufacturerSpecificData (uint16_t companyID,
                                      size_t span);
};

/** @cond DOXYGEN_EXCLUDE */
template <>
void adv_data::set_CompleteListServiceUUID (const uuid16_type* begin,
                                            const uuid16_type* end);
template <>
void adv_data::set_CompleteListServiceUUID (const uuid32_type* begin,
                                            const uuid32_type* end);
template <>
void adv_data::set_CompleteListServiceUUID (const uuid128_type* begin,
                                            const uuid128_type* end);
template <>
void adv_data::set_IncompleteListServiceUUID (const uuid16_type* begin,
                                              const uuid16_type* end);
template <>
void adv_data::set_IncompleteListServiceUUID (const uuid32_type* begin,
                                              const uuid32_type* end);
template <>
void adv_data::set_IncompleteListServiceUUID (const uuid128_type* begin,
                                              const uuid128_type* end);
template <>
void adv_data::set_ListServiceSolicitationUUID (const uuid16_type* begin,
                                                const uuid16_type* end);
template <>
void adv_data::set_ListServiceSolicitationUUID (const uuid32_type* begin,
                                                const uuid32_type* end);
template <>
void adv_data::set_ListServiceSolicitationUUID (const uuid128_type* begin,
                                                const uuid128_type* end);
template <>
uint8_t*
adv_data::set_ServiceData (const uuid16_type& uuid,
                           const uint8_t* begin,
                           const uint8_t* end);
template <>
uint8_t*
adv_data::set_ServiceData (const uuid32_type& uuid,
                           const uint8_t* begin,
                           const uint8_t* end);
template <>
uint8_t*
adv_data::set_ServiceData (const uuid128_type& uuid,
                           const uint8_t* begin,
                           const uint8_t* end);
/** @endcond */

} // ns gap
} // ns ble
} // ns pabigot

#endif /* PABIGOT_BLE_GAP_HPP */
