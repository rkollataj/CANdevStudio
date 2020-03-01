#include "pythonbackend.h"
#include "psmessage.h"
#include <QCanBusFrame>
#include <QCoreApplication>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUuid>
#include <QVariant>
#include <log.h>

namespace CdsShMem {
const std::string id = QUuid::createUuid().toString().toStdString();
};

ShMemMgr PythonBackend::_appShm;

PythonBackend::PythonBackend()
    : _outQueueName(QUuid::createUuid().toString())
    , _inQueueName(QUuid::createUuid().toString())
{
    _shm.openShm(CdsShMem::id);

    // queues will be automatically destroyed with _shm object
    _outQueue = _shm.createQueue(_outQueueName.toStdString());
    _inQueue = _shm.createQueue(_inQueueName.toStdString());
}

void PythonBackend::startScript(const QString& scriptName)
{
    QStringList args;
    args << "-m" << CdsShMem::id.c_str();
    // backednd out is frontend in
    args << "-i" << _outQueueName;
    args << "-o" << _inQueueName;
    args << "-s" << scriptName;

    // CANdevStudio-python shall be located in the same folder as main executable
    QString cdsp = QStandardPaths::findExecutable("CANdevStudio-python", { QCoreApplication::applicationDirPath() });

    _process.setProcessChannelMode(QProcess::ForwardedChannels);
    _process.start(cdsp, args);

    // Starts Thread
    start();
}

void PythonBackend::run()
{
    PsMessage msg;

    while (msg.type() != PsMessageType::CLOSE) {
        std::vector<uint8_t> vec = _shm.readQueue(_inQueue);

        msg = PsMessage::fromData(vec);

        if (msg.type() == PsMessageType::FRAME) {
            uint32_t id;
            std::vector<uint8_t> payload;
            std::string dir;

            if (msg.toFrame(id, payload, dir)) {
                QCanBusFrame frame;
                frame.setFrameId(id);
                frame.setPayload(QByteArray(reinterpret_cast<const char*>(payload.data()), payload.size()));

                emit sndFrame(frame);
            }
        } else if (msg.type() == PsMessageType::SIGNAL) {
            uint32_t id;
            std::string name;
            double val;
            std::string dir;

            if (msg.toSignal(id, name, val, dir)) {
                QString signal = fmt::format("0x{:03x}_{}", id, name).c_str();

                emit sndSignal(signal, val);
            }
        }
    }
}

void PythonBackend::stop()
{
    if (_process.state() == QProcess::NotRunning) {
        sendIntMsgClose();
    } else {
        sendMsgClose();
    }

    // wait for backend "read" thread
    wait();
    // wait for frontend process
    _process.waitForFinished();
}

void PythonBackend::sendMsgSignal(uint32_t id, const QString& name, double value, int32_t dir)
{
    _shm.writeQueue(_outQueue, PsMessage::fromSignal(id, name.toStdString(), value, dir).toArray());
}

void PythonBackend::sendMsgFrame(const QCanBusFrame& frame, int32_t dir)
{
    _shm.writeQueue(_outQueue, PsMessage::fromFrame(frame, dir).toArray());
}

void PythonBackend::sendIntMsgClose()
{
    _shm.writeQueue(_inQueue, PsMessage::createCloseMessage().toArray());
}

void PythonBackend::sendMsgClose()
{
    _shm.writeQueue(_outQueue, PsMessage::createCloseMessage().toArray());
}
