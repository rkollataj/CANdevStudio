#ifndef __PYTHONFRONTEND_H
#define __PYTHONFRONTEND_H

#include "shmemmgr.h"
#include <QObject>
#include <QThread>
#include <atomic>
#include <iostream>

typedef struct _object PyObject;

class PythonFrontend : public QThread {
    Q_OBJECT

public:
    PythonFrontend(const std::string& shmId, const std::string& inQueue, const std::string& outQueue);
    static PyObject* sndFrame(PyObject* self, PyObject* args);
    static PyObject* sndSignal(PyObject* self, PyObject* args);
    void sendBackendCloseMsg();

private:
    void run() override;

public:
    std::atomic_bool _pyRunning;

private:
    ShMemMgr _shm;
    ShMemMgr::queue_t* _inQueue;
    ShMemMgr::queue_t* _outQueue;
};

#endif /* !__PYTHONFRONTEND_H */
