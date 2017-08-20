#ifndef CANDEVICEQT_HPP_JYBV8GIQ
#define CANDEVICEQT_HPP_JYBV8GIQ

#include "candeviceinterface.hpp"
#include <QtSerialBus/QCanBus>
#include <QtSerialBus/QCanBusDevice>

struct CanDeviceQt : public CanDeviceInterface {
    virtual void setFramesWrittenCbk(const framesWritten_t& cb) override
    {
        QObject::connect(_device.get(), &QCanBusDevice::framesWritten, cb);
    }

    virtual void setFramesReceivedCbk(const framesReceived_t& cb) override
    {
        QObject::connect(_device.get(), &QCanBusDevice::framesReceived, cb);
    }

    virtual void setErrorOccurredCbk(const errorOccurred_t& cb) override
    {
        QObject::connect(_device.get(), &QCanBusDevice::errorOccurred, cb);
    }

    virtual bool init(const QString& backend, const QString& iface) override
    {
        _device.reset(QCanBus::instance()->createDevice(backend.toUtf8(), iface));
        if (!_device) {
            throw std::runtime_error("Unable to create candevice");
        }

        return true;
    }

    virtual bool writeFrame(const QCanBusFrame& frame) override { return _device->writeFrame(frame); }

    virtual bool connectDevice() override { return _device->connectDevice(); }
    virtual qint64 framesAvailable() override { return _device->framesAvailable(); }

    virtual QCanBusFrame readFrame() noexcept override { return _device->readFrame(); }

private:
    std::unique_ptr<QCanBusDevice> _device;
};

#endif /* end of include guard: CANDEVICEQT_HPP_JYBV8GIQ */
