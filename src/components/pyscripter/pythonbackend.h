#ifndef __PYTHONBACKEND_H
#define __PYTHONBACKEND_H

#include "shmemmgr.h"
#include <QProcess>
#include <QThread>

namespace CdsShMem {
extern const std::string id;
};

class QCanBusFrame;

class PythonBackend : public QThread {
    Q_OBJECT

public:
    PythonBackend();

    void startScript(const QString& scriptName);
    void stop();

    void sendMsgFrame(const QCanBusFrame& frame, int32_t dir);
    void sendMsgClose();

signals:
    void sndFrame(const QCanBusFrame& frame);

public:
    static ShMemMgr _appShm;

private:
    void run() override;

private:
    QProcess _process;
    ShMemMgr _shm;
    ShMemMgr::queue_t* _inQueue;
    ShMemMgr::queue_t* _outQueue;
    QString _outQueueName;
    QString _inQueueName;
};

#endif /* !__PYTHONBACKEND_H */
