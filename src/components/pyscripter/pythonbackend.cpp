#include "pythonbackend.h"
#include "psmessage.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QUuid>
#include <log.h>
#include <QCanBusFrame>

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

bool PythonBackend::start(const QString& scriptName)
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

    return false;
}

void PythonBackend::stop()
{
    sendMsgClose();
    _process.waitForFinished();
}

void PythonBackend::sendMsgFrame(const QCanBusFrame& frame, const QString& dir)
{
    _shm.writeQueue(_outQueue, PsMessage::fromFrame(frame, dir).toArray());
}

void PythonBackend::sendMsgClose()
{
    _shm.writeQueue(_outQueue, PsMessage::createCloseMessage().toArray());
}

