#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "ioloop.h"
#include "libdongle.h"
#include "usbdevice.h"
#include <set>
#include <experimental/coroutine>
#include <list>
#include <thread>

namespace libdongle {

class Task
{
public:
    enum Status {
        RUNNING,
        WAIT_WRITE,
        WAIT_READ,
        WAIT_TASK,
        WAIT_SLEEP,
        WAIT_LOCK_WRITE_CHANNEL,
        FINISHED,
    };

    struct promise_type
    {
        std::experimental::suspend_always initial_suspend() { return {}; }

        struct final_awaiter
        {
            bool await_ready() noexcept { return false; }
            void await_suspend(std::experimental::coroutine_handle<promise_type> h) noexcept;
            void await_resume() noexcept {}
        };

        final_awaiter final_suspend() noexcept { return {}; }
        void unhandled_exception() {}
        Task get_return_object();

        void setRunning();
        void setFinished();
        void setWaitWriteData(const std::shared_ptr<WriteDataRequest> &req, int timeout);
        void setWaitReadChannel(uint32_t waitReadChanncel, int timeout);
        void setWaitLockWriteChannel(uint32_t writeChannel, int timeout);
        void setSleep(int timeout);
        void setWaitTask();
        void setWaitParent(bool e);
        void return_value(bool r) { result = r; }

        ~promise_type();

        Status status{RUNNING};
        uint32_t waitReadChannel{0xFFFF'FFFF};
        uint32_t waitLockWriteChannel{0xFFFF'FFFF};
        std::shared_ptr<WriteDataRequest> waitRequest;
        std::shared_ptr<ReadDataPackage> readedPackage;
        bool hasDeadLine{false};
        std::chrono::time_point<std::chrono::system_clock> deadLine;
        std::experimental::coroutine_handle<> continuation;
        bool result{false};
        bool waitParentTask{false};
    };

    Task() noexcept : m_coroutine{nullptr} {}
    Task(std::experimental::coroutine_handle<promise_type> coroutine) : m_coroutine{coroutine} {}
    Task(Task &&t) noexcept : m_coroutine(t.m_coroutine) { t.m_coroutine = nullptr; }
    ~Task();

    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;

    Task &operator=(Task &&other) noexcept;

    bool ready();

    void resume();

    bool isFinished() const;
    bool isWaitForWrite() const;
    bool isWaitForRead() const;
    bool isWaitLockWriteChannel() const;

    uint32_t waitReadChannel() const;
    uint32_t waitLockWriteChannel() const;

    void setReadDataPackage(const std::shared_ptr<ReadDataPackage> &pkg);
    const std::shared_ptr<ReadDataPackage> &readedPackage() const;
    const std::shared_ptr<WriteDataRequest> &writeDataRequest() const;

    std::experimental::coroutine_handle<promise_type> coro() const { return m_coroutine; }

    bool isDeadLine();

    void setShouldWaitParentTask(bool e);

private:
    std::experimental::coroutine_handle<promise_type> m_coroutine{nullptr};
};

class Executor
{
public:
    Executor(const std::shared_ptr<UsbDevice> &usbDevice);
    void resetUsbDevice(const std::shared_ptr<UsbDevice> &usbDevice);

    ~Executor();

    void runAllTask();

    Task &submitTask(Task &&task);

    void postWriteRequest(const IOLoop::WriteType &req);

    int writeTimeout() const { return m_writeTimeout; }
    int readTimeout() const { return m_readTimeout; }
    int rcspTimeout() const { return m_rcspTimeout; }
    int reconnTimeout() const { return m_reconnTimeout; }

    void removePendingRead(int channel);

    void resetPendingReads();

    void unlockWriteChannel(uint32_t channel);

private:
    std::shared_ptr<UsbDevice> m_usbDevice;
    std::shared_ptr<IOLoop> m_ioLoop;
    std::list<Task> m_tasks;
    IOLoop::WriteQueueType m_writeQueue;
    IOLoop::ReadQueueType m_readQueue;
    std::list<IOLoop::ReadType> m_pendingReads;
    std::thread m_ioThread;
    dongle_callback_t m_dongleCallback;
    std::set<uint32_t> m_currentLockedWriteChannels;
    int m_writeTimeout;
    int m_readTimeout;
    int m_rcspTimeout;
    int m_reconnTimeout;
};

} // namespace libdongle

#endif // EXECUTOR_H
