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
    PythonFrontend();
    static PyObject* init(PyObject* self, PyObject* args);
    static PyObject* sndFrame(PyObject* self, PyObject* args);
    static PyObject* sndSignal(PyObject* self, PyObject* args);
    void sendBackendCloseMsg();

private:
    void run() override;

public:
    static std::string shmId;
    static std::string inQueue;
    static std::string outQueue;
    static std::string scriptName;

private:
    ShMemMgr _shm;
    ShMemMgr::queue_t* _inQueue;
    ShMemMgr::queue_t* _outQueue;
};

#endif /* !__PYTHONFRONTEND_H */
