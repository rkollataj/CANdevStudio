#include "pythonbackend.h"
#include <QUuid>
#include <log.h>

PythonBackend::PythonBackend()
    : _outQueueName(QUuid::createUuid().toString())
    , _inQueueName(QUuid::createUuid().toString())
{
    _shm.openShm(CdsShMem::id);

    // queues will be automatically destroyed with _shm object
    _shm.createQueue(_outQueueName.toStdString());
    _shm.createQueue(_inQueueName.toStdString());
    _outQueue = _shm.openQueue(_outQueueName.toStdString());
    _inQueue = _shm.openQueue(_inQueueName.toStdString());
}

bool PythonBackend::start(const QString& scriptName)
{
    QStringList args;
    args << "-m" << CdsShMem::id.c_str();
    // backednd out is frontend in
    args << "-i" << _outQueueName;
    args << "-o" << _inQueueName;
    args << "-s" << "fake_script";
    
    _process.setProcessChannelMode(QProcess::ForwardedChannels);
    _process.start("./src/common/cds-python", args);

    std::string str("Test str");
    std::vector<uint8_t> vec(str.begin(), str.end());
    _shm.writeQueue(_outQueue, vec);

    return false;
}

void PythonBackend::stop()
{
    std::string str("End");
    std::vector<uint8_t> vec(str.begin(), str.end());
    _shm.writeQueue(_outQueue, vec);
}
