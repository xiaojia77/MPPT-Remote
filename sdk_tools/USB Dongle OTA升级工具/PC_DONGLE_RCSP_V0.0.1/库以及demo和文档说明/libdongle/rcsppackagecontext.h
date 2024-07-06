#ifndef RCSPPACKAGECONTEXT_H
#define RCSPPACKAGECONTEXT_H

#include <memory>
#include <stdint.h>
#include <vector>

namespace libdongle {

class RcspCommand
{
public:
    RcspCommand(uint8_t seq, std::vector<uint8_t> body) : m_seq(seq), m_body(std::move(body)) {}

    uint8_t seq() const { return m_seq; }
    const std::vector<uint8_t> &body() const { return m_body; }

private:
    uint8_t m_seq;
    std::vector<uint8_t> m_body;
};

class RcspPackageContext
{
public:
    struct parse_info_t
    {
        bool isReply;
        bool needReply;
        uint8_t seq;
        uint8_t opCode;
        uint8_t status;
        const void *payload;
        size_t payload_len;
    };

    struct device_info_t
    {
        struct
        {
            bool support_backup;
            bool need_bootloader;
            uint8_t no_backup_conn;
        } backup;
        struct
        {
            uint16_t vid;
            uint16_t pid;
            uint16_t uid;
        } pidvid;
        struct
        {
            bool is_sdk;    // sdk or loader
            bool is_normal; // normal update or force update
            bool is_ext;
        } upstatus;
    };

    RcspPackageContext() {}
    ~RcspPackageContext();

    std::shared_ptr<RcspCommand> makeCommand(uint8_t opCode, bool needReply, const void *payload,
                                             size_t payload_len);

    std::shared_ptr<RcspCommand> makeReply(uint8_t opCode, uint8_t status, uint8_t seq,
                                           const void *payload, size_t payload_len);

    static bool getOpcode(const std::vector<uint8_t> &data, uint8_t &opcode);
    static bool parsePackage(const void *data, size_t data_len, parse_info_t &info);

    std::shared_ptr<RcspCommand> makeQueryDeviceInfoCmd(uint32_t mask);
    std::shared_ptr<RcspCommand> makeQueryFileHeadOffsetCmd();
    std::shared_ptr<RcspCommand> makeQueryAllowUpdateCmd(const void *data, size_t data_len);
    std::shared_ptr<RcspCommand> makeEnterUpgradeModeCmd();
    std::shared_ptr<RcspCommand> makeSwitchCommCmd(uint8_t dev, uint8_t reconn);
    std::shared_ptr<RcspCommand> makeRequestFileDataReply(uint8_t seq, const void *data,
                                                          size_t data_len);
    std::shared_ptr<RcspCommand> makeQueryDeviceUpdateStatusCmd();
    std::shared_ptr<RcspCommand> makeInfoUpdateContentLenReply(uint8_t seq);
    std::shared_ptr<RcspCommand> makeForceRebootCmd();

    static bool isReplyEnterMode(const std::vector<uint8_t> &data, uint8_t seq, bool &canUpdate);
    static bool isReplyQueryFileHeadOffset(const std::vector<uint8_t> &data, uint8_t seq,
                                           uint32_t &offset, uint16_t &size);
    static bool isReplyAllowUpdate(const std::vector<uint8_t> &data, uint8_t seq,
                                   uint8_t &result);
    static bool isRequestFileDataCmd(const std::vector<uint8_t> &data, uint8_t &seq,
                                     uint32_t &fileOffset, uint16_t &data_len);
    static bool isInfoUpdateContentLenCmd(const std::vector<uint8_t> &data, uint8_t &seq,
                                          uint32_t &total_len, uint32_t &current_len);
    static bool isReplyQueryDeviceUpdateStatus(const std::vector<uint8_t> &data, uint8_t seq,
                                               uint8_t &status);
    static bool isReplyQueryDeviceInfo(const std::vector<uint8_t> &data, uint8_t seq,
                                       device_info_t &di);
    static bool isReplySwitchComm(const std::vector<uint8_t> &data, uint8_t seq,
                                  uint8_t &reconn);
    static bool isReplyForceReboot(const std::vector<uint8_t> &data, uint8_t seq, uint8_t &status);

private:
    uint8_t m_seq{0};
};

} // namespace libdongle

#endif // RCSPPACKAGECONTEXT_H
