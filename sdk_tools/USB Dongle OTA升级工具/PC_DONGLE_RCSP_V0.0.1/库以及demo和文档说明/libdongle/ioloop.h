#ifndef IOLOOP_H
#define IOLOOP_H

#include "blockingqueue.h"
#include "usbdevice.h"
#include <atomic>

namespace libdongle {

class ReadDataPackage
{
    uint32_t m_channel;
    uint32_t m_opcode;
    std::vector<uint8_t> m_data;

public:
    ReadDataPackage(uint32_t channel, uint32_t opcode) : m_channel(channel), m_opcode(opcode) {}
    ReadDataPackage(uint32_t channel, uint32_t opcode, std::vector<uint8_t> data)
        : m_channel(channel), m_opcode(opcode), m_data(data)
    {}

    uint32_t channel() { return m_channel; }
    uint32_t opcode() { return m_opcode; }
    const std::vector<uint8_t> &data() const { return m_data; }
};

class WriteDataRequest
{
    std::vector<uint8_t> m_buffer;
    int m_status;

public:
    enum {
        PENDING,
        DONE,
        FAILED,
    };

    WriteDataRequest(std::vector<uint8_t> buffer) : m_buffer{buffer}, m_status{PENDING} {}

    void setStatus(int status) { m_status = status; }

    int status() { return m_status; }
    const std::vector<uint8_t> &buffer() const { return m_buffer; }

    static std::shared_ptr<WriteDataRequest> make(uint32_t channel, uint32_t opcode);
    static std::shared_ptr<WriteDataRequest> make(uint32_t channel, uint32_t opcode,
                                                  const void *buffer, size_t buffer_len);

    static std::shared_ptr<WriteDataRequest> makeConnClient(uint32_t clientChannel,
                                                            const void *mac_addr,
                                                            bool connect_back);
    static std::shared_ptr<WriteDataRequest> makeSetClientAuthStatus(uint32_t clientChannel,
                                                                     uint32_t flag);
    static std::shared_ptr<WriteDataRequest> makeDisconnClient(uint32_t clientChannel);

    static std::shared_ptr<WriteDataRequest> makeRcsp(uint32_t clientChannel, const std::vector<uint8_t> &body);
};

class IOLoopImpl;

class IOLoop
{
public:
    typedef std::shared_ptr<WriteDataRequest> WriteType;
    typedef std::shared_ptr<BlockingQueue<WriteType>> WriteQueueType;
    typedef std::shared_ptr<ReadDataPackage> ReadType;
    typedef std::shared_ptr<BlockingQueue<ReadType>> ReadQueueType;

    IOLoop(const std::shared_ptr<UsbDevice> &usbDev, const WriteQueueType &writeQueue,
           const ReadQueueType &readQueue);
    ~IOLoop();

    void runLoop();

    void requestStop();

private:
    std::unique_ptr<IOLoopImpl> m_impl;
};

}; // namespace libdongle

#endif // IOLOOP_H
