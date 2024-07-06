#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace libdongle {

std::string toUtf8(const std::wstring &str);
std::wstring toUtf16(const std::string &str);

template<typename T>
T read_be(const void *_ptr, size_t off)
{
    T v              = 0;
    const uint8_t *p = (const uint8_t *) _ptr;
    for (size_t i = 0; i < sizeof(T); ++i) {
        v = (v << 8) | p[i + off];
    }
    return v;
}

template<typename T>
void store_be(void *_ptr, size_t off, T v)
{
    uint8_t *p = (uint8_t *) _ptr;
    for (size_t i = 0; i < sizeof(T); ++i) {
        p[i + off] = (v >> (sizeof(T) - i - 1) * 8) & 0xFF;
    }
}

#define DONGLE_CMD_CHANNEL 0
#define DONGLE_REPLY_CHANNEL 1
#define USB_CHANNEL 2
#define BLE_CLIENT_0_CHANNEL 3
#define BLE_CLIENT_1_CHANNEL 4
#define BLE_CLIENT_2_CHANNEL 5
#define BLE_CLIENT_3_CHANNEL 6
#define BLE_CLIENT_4_CHANNEL 7
#define BLE_CLIENT_5_CHANNEL 8
#define BLE_CLIENT_6_CHANNEL 9
#define BLE_CLIENT_7_CHANNEL 10

#define OP_RCSP_DATA 0x00
#define OP_GET_DONGLE_INFO 0x01
#define OP_GET_ONLINE_CLIENT 0x02
#define OP_INFO_CLIENT_STATUS 0x03
#define OP_CONNECT_CLIENT 0x04
#define OP_DISCONNECT_CLIENT 0x05
#define OP_SET_DEV_AUTH_STATUS 0x06
#define OP_CUSTOM 0xFF

#define OP_OTA_QUERY_DEVICE_INFO 0x03
#define OP_OTA_SWITCH_COMM_MODE 0x0B
#define OP_OTA_QUERY_FILE_HEAD_OFFSET 0xE1
#define OP_OTA_SEND_FILE_HEAD 0xE2
#define OP_OTA_ENTER_UPGRADE_MODE 0xE3
#define OP_OTA_EXIT_UPGRADE_MODE 0xE4
#define OP_OTA_REQUEST_FILE_DATA 0xE5
#define OP_OTA_GET_DEVICE_UPGRADE_STATUS 0xE6
#define OP_OTA_FORCE_RESET_DEVICE 0xE7
#define OP_OTA_INFO_UPGRADE_CONTENT_LEN 0xE8

#define RCSP_REPLY_success 0x00
#define RCSP_REPLY_failed 0x01
#define RCSP_REPLY_undef 0x02
#define RCSP_REPLY_busy 0x03
#define RCSP_REPLY_unack 0x04
#define RCSP_REPLY_crcerr 0x05
#define RCSP_REPLY_data_crcerr 0x06
#define RCSP_REPLY_param_ovf 0x07
#define RCSP_REPLY_data_ovf 0x08
#define RCSP_REPLY_lrcerr 0x09

#define OTA_STATUS_finished 0x00
#define OTA_STATUS_data_crcerr 0x01
#define OTA_STATUS_failed 0x02
#define OTA_STATUS_key_mismatch 0x03
#define OTA_STATUS_file_err 0x04
#define OTA_STATUS_uboot_mismatch 0x05
#define OTA_STATUS_len_err 0x06
#define OTA_STATUS_flash_err 0x07
#define OTA_STATUS_timeout 0x08
#define OTA_STATUS_same_file 0x09
#define OTA_STATUS_boot_loader_finish 0x80

#define OTA_ALLOW_UPDATE_yes 0x00
#define OTA_ALLOW_UPDATE_low_power 0x01
#define OTA_ALLOW_UPDATE_ufw_err 0x02
#define OTA_ALLOW_UPDATE_same_ver 0x03
#define OTA_ALLOW_UPDATE_tws_disconn 0x04
#define OTA_ALLOW_UPDATE_not_inbox 0x05

#define BIT(x) ((1u << (x)))
#define DEV_INFO_backup_type 8
#define DEV_INFO_update_status 9
#define DEV_INFO_pidvid 10

}; // namespace libdongle

#endif // UTILS_H
