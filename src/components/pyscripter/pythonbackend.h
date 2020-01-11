#ifndef __PYTHONBACKEND_H
#define __PYTHONBACKEND_H

#include "shmemmgr.h"
#include <QProcess>

namespace CdsShMem {
extern const std::string id;
};

class QCanBusFrame;

class PythonBackend {
public:
    PythonBackend();

    bool start(const QString& scriptName);
    void stop();

    void sendMsgFrame(const QCanBusFrame& frame);
    void sendMsgClose();

public:
    static ShMemMgr _appShm;

private:
    QProcess _process;
    ShMemMgr _shm;
    ShMemMgr::queue_t* _inQueue;
    ShMemMgr::queue_t* _outQueue;
    QString _outQueueName;
    QString _inQueueName;
};

#endif /* !__PYTHONBACKEND_H */
