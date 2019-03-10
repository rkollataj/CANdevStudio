#ifndef __QCANBUSFRAMEDECORATOR_H
#define __QCANBUSFRAMEDECORATOR_H

#include <QObject>

// This allows to access QCanBusFrame from Python
class QCanBusFrameDecorator : public QObject {
    Q_OBJECT

public:
    enum FrameType {
        UnknownFrame = 0x0,
        DataFrame = 0x1,
        ErrorFrame = 0x2,
        RemoteRequestFrame = 0x3,
        InvalidFrame = 0x4
    };
    Q_ENUM(FrameType)

    enum FrameError {
        NoError = 0,
        TransmissionTimeoutError = (1 << 0),
        LostArbitrationError = (1 << 1),
        ControllerError = (1 << 2),
        ProtocolViolationError = (1 << 3),
        TransceiverError = (1 << 4),
        MissingAcknowledgmentError = (1 << 5),
        BusOffError = (1 << 6),
        BusError = (1 << 7),
        ControllerRestartError = (1 << 8),
        UnknownError = (1 << 9),
        AnyError = 0x1FFFFFFFU
        // only 29 bits usable
    };
    Q_DECLARE_FLAGS(FrameErrors, FrameError)
    Q_FLAGS(FrameErrors)

public slots:
    QCanBusFrame* new_QCanBusFrame(quint32 id, const QByteArray& payload)
    {
        return new QCanBusFrame(id, payload);
    }

    QCanBusFrame* new_QCanBusFrame(QCanBusFrame::FrameType type = QCanBusFrame::DataFrame)
    {
        return new QCanBusFrame(type);
    }

    void delete_QCanBusFrame(QCanBusFrame* obj)
    {
        delete obj;
    }

    QCanBusFrame::FrameErrors error(QCanBusFrame* obj) const
    {
        return obj->error();
    }

    quint32 frameId(QCanBusFrame* obj) const
    {
        return obj->frameId();
    }

    QCanBusFrame::FrameType frameType(QCanBusFrame* obj) const
    {
        return obj->frameType();
    }

    bool hasBitrateSwitch(QCanBusFrame* obj) const
    {
        return obj->hasBitrateSwitch();
    }

    bool hasErrorStateIndicator(QCanBusFrame* obj) const
    {
        return obj->hasErrorStateIndicator();
    }

    bool hasExtendedFrameFormat(QCanBusFrame* obj) const
    {
        return obj->hasExtendedFrameFormat();
    }

    bool hasFlexibleDataRateFormat(QCanBusFrame* obj) const
    {
        return obj->hasFlexibleDataRateFormat();
    }

    bool hasLocalEcho(QCanBusFrame* obj) const
    {
        return obj->hasLocalEcho();
    }

    bool isValid(QCanBusFrame* obj) const
    {
        return obj->isValid();
    }

    QByteArray payload(QCanBusFrame* obj) const
    {
        return obj->payload();
    }

    void setBitrateSwitch(QCanBusFrame* obj, bool bitrateSwitch) const
    {
        obj->setBitrateSwitch(bitrateSwitch);
    }

    void setError(QCanBusFrame* obj, QCanBusFrame::FrameErrors error)
    {
        obj->setError(error);
    }

    void setErrorStateIndicator(QCanBusFrame* obj, bool errorStateIndicator)
    {
        obj->setErrorStateIndicator(errorStateIndicator);
    }

    void setExtendedFrameFormat(QCanBusFrame* obj, bool isExtended)
    {
        obj->setExtendedFrameFormat(isExtended);
    }

    void setFlexibleDataRateFormat(QCanBusFrame* obj, bool isFlexibleData)
    {
        obj->setFlexibleDataRateFormat(isFlexibleData);
    }

    void setFrameId(QCanBusFrame* obj, quint32 newFrameId)
    {
        obj->setFrameId(newFrameId);
    }

    void setFrameType(QCanBusFrame* obj, QCanBusFrame::FrameType frameType)
    {
        obj->setFrameType(frameType);
    }

    void setLocalEcho(QCanBusFrame* obj, bool echo)
    {
        obj->setLocalEcho(echo);
    }

    void setPayload(QCanBusFrame* obj, QByteArray& data)
    {
        obj->setPayload(data);
    }
};

#endif /* !__QCANBUSFRAMEDECORATOR_H */
