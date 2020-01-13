#include "pythonbackend.h"
#include "psmessage.h"
#include <QCanBusFrame>
#include <QCoreApplication>
#include <QFileInfo>
#include <QUuid>
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
    args << "-s"
         << "fake_script";

    // CANdevStudio-python shall be located in the same folder as main executable
    QFileInfo fi(QCoreApplication::applicationFilePath());
    auto frontendPath = QCoreApplication::applicationDirPath()
        + fi.completeBaseName().replace("CANdevStudio", "/CANdevStudio-python");

    _process.setProcessChannelMode(QProcess::ForwardedChannels);
    _process.start(frontendPath, args);

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
                frame.setPayload(QByteArray(reinterpret_cast<const char *>(payload.data()), payload.size()));

                emit sndFrame(frame);
            }
        }
    }
}

void PythonBackend::stop()
{
    sendMsgClose();
    // wait for backend "read" thread
    wait();
    // wait for frontend process
    _process.waitForFinished();
}

void PythonBackend::sendMsgFrame(const QCanBusFrame& frame, int32_t dir)
{
    _shm.writeQueue(_outQueue, PsMessage::fromFrame(frame, dir).toArray());
}

void PythonBackend::sendMsgClose()
{
    _shm.writeQueue(_outQueue, PsMessage::createCloseMessage().toArray());
}
