#ifndef TASKS_H
#define TASKS_H

#include "executor.h"
#include "libdongle.h"
#include "rcsppackagecontext.h"
#include "utils.h"

namespace libdongle {
Task ota_update_client(Executor &ex, int channel, const std::vector<uint8_t> &ufwfile, bool auth1,
                       bool auth2, ota_upgrade_callback_t ota_callbak);
Task ota_update_dongle(Executor &ex, int channel, const std::vector<uint8_t> &ufwFile, bool auth1,
                       bool auth2, ota_upgrade_callback_t ota_callback, bool &needPost);

Task ota_update_dongle_single_post(Executor &ex, int channel, const std::vector<uint8_t> &ufwFile,
                                   bool auth2, ota_upgrade_callback_t ota_callback);
Task read_dongle_info(Executor &ex, ota_upgrade_callback_t callback, uint8_t &is_loader);
Task read_online_device(Executor &ex, dongle_callback_t callback);
} // namespace libdongle

#endif // TASKS_H
