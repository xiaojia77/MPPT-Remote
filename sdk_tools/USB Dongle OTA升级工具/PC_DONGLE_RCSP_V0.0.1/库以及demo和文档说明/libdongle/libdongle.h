#ifndef LIBDONGLE_H
#define LIBDONGLE_H

#include "libdongle_global.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct dongle_client_info_t
{
    uint8_t channel_seq;
    uint8_t support_ota;
    uint8_t mac_addr[6];
    char device_name[256 - 8];
};

struct dongle_callback_t
{
    void *ctx;
    void (*on_dongle_info)(void *ctx, uint32_t sdk_version, uint16_t vid, uint16_t pid,
                           uint16_t pidVersion);

    void (*on_refresh_client_info)(void *ctx, uint32_t client_count,
                                   struct dongle_client_info_t *client_infos);

    void (*on_error)(void *ctx, int error, const char *msg);
};

#define OTA_ERR_success 0x00
#define OTA_ERR_no_client 0x01
#define OTA_ERR_auth_failed 0x02
#define OTA_ERR_client_err 0x03
#define OTA_ERR_not_allow 0x04

struct ota_upgrade_callback_t
{
    void *ctx;
    void (*on_start_ota)(void *ctx);
    void (*on_finish_ota)(void *ctx);
    void (*on_need_reconnect)(void *ctx);
    void (*on_progress)(void *ctx, int step, int total);
    void (*on_stop_ota)(void *ctx);
    void (*on_cancel_ota)(void *ctx);
    void (*on_error)(void *ctx, int error, const char *msg);
};

void *libdongle_create_executor(const char *path, size_t len);
bool libdongle_reset_device(const char *path, size_t len);
void libdongle_free_executor(void *ex);

void libdongle_submit_task_list_remote_clients(void *ctx, dongle_callback_t *cb);
void libdongle_submit_task_update_client(void *ctx, int channel, bool auth,
                                         ota_upgrade_callback_t cb);
void libdongle_run_alltask(void *ex);

#ifdef __cplusplus
}
#endif

#endif // LIBDONGLE_H
