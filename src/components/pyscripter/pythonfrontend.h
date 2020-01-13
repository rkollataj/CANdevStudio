#ifndef __PYTHONFRONTEND_H
#define __PYTHONFRONTEND_H

#include "shmemmgr.h"
#include <QObject>
#include <iostream>
#include <QThread>

typedef struct _object PyObject;

class PythonFrontend : public QThread {
    Q_OBJECT

public:
    PythonFrontend(const std::string& shmId, const std::string& inQueueName, const std::string& outQueueName);
    static PyObject* sndFrame(PyObject* self, PyObject* args);
    void sendBackendCloseMsg();

private:
    void run() override;

private:
    ShMemMgr _shm;
    ShMemMgr::queue_t* _inQueue;
    ShMemMgr::queue_t* _outQueue;
    static PythonFrontend* _thisWrapper;
};

#endif /* !__PYTHONFRONTEND_H */
