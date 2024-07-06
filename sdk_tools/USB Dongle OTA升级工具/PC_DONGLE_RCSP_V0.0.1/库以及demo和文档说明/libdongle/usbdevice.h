#ifndef USBDEVICE_H
#define USBDEVICE_H

#include <memory>
#include <string>
#include <vector>

namespace libdongle {

class UsbDevInfo
{
    std::string m_hardwareID;
    std::string m_devicePath;
    std::string m_friendlyName;

public:
    UsbDevInfo(std::string hardwareId, std::string devicePath, std::string friendlyName)
        : m_hardwareID(hardwareId), m_devicePath(devicePath), m_friendlyName(friendlyName)
    {}

    const std::string &id() const { return m_hardwareID; }
    const std::string &path() const { return m_devicePath; }
    const std::string &name() const { return m_friendlyName; }
};

class AsyncInfoImpl;
class AsyncInfo
{
public:
    AsyncInfo(AsyncInfoImpl *impl) : m_impl(impl) {}
    ~AsyncInfo();

    bool wait(int ms);
    int size();

    uint32_t getReadSize();

    AsyncInfoImpl *impl() { return m_impl; }

private:
    AsyncInfoImpl *m_impl{nullptr};
};

class UsbDeviceImpl;
class UsbDevice
{
public:
    UsbDevice();
    ~UsbDevice();

    bool openAsync(const std::string &path);
    std::shared_ptr<AsyncInfo> writeHIDAsync(const void *buffer, size_t buffer_len);
    std::shared_ptr<AsyncInfo> readHIDAsync(void *buffer, size_t read_len);
    uint32_t readHIDWithTimeout(void *buffer, size_t read_len, uint32_t ms);
    bool writeHID(const void *buffer, size_t buffer_len);

    static std::vector<UsbDevInfo> iterAllHidDevice();

private:
    UsbDeviceImpl *m_impl{nullptr};
};

} // namespace libdongle

#endif // USBDEVICE_H
