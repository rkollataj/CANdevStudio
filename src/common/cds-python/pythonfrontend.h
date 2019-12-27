#ifndef __PYTHONFRONTEND_H
#define __PYTHONFRONTEND_H

#include "shmemmgr.h"

class PythonFrontend {
public:
    PythonFrontend(const std::string& shmId, const std::string& inQueueName, const std::string& outQueueName);
    bool start();

private:
    ShMemMgr _shm;
    ShMemMgr::queue_t* _inQueue;
    ShMemMgr::queue_t* _outQueue;
};

#endif /* !__PYTHONFRONTEND_H */
