#ifndef __PYTHONBACKEND_H
#define __PYTHONBACKEND_H

#include <QProcess>
#include "shmemmgr.h"

class PythonBackend {
public:
    PythonBackend();

    bool start(const QString& scriptName);
    void stop();

private:
    QProcess _process;
    ShMemMgr _shm;
    ShMemMgr::queue_t* _inQueue;
    ShMemMgr::queue_t* _outQueue;
    QString _outQueueName;
    QString _inQueueName;

};

#endif /* !__PYTHONBACKEND_H */
