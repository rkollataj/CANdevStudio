#ifndef __CANDEVICE_P_H
#define __CANDEVICE_P_H

#include "candeviceinterface.hpp"
#include "candeviceqt.hpp"
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QVariant>
#include <QtCore/QVector>
#include <QtSerialBus/QCanBus>
#include <QtSerialBus/QCanBusFrame>
#include <memory>

class CanDevicePrivate {
public:
    CanDevicePrivate(CanDeviceCtx* ctx)
        : context(ctx)
        , canDevice(ctx->get<CanDeviceInterface>())
    {
    }

    QString mBackend;
    QString mInterface;
    QVector<QCanBusFrame> mSendQueue;
    std::unique_ptr<CanDeviceCtx> context;
    CanDeviceInterface& canDevice;
};

#endif /* !__CANDEVICE_P_H */
