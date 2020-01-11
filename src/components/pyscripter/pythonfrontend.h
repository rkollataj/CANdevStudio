#ifndef __PYTHONFRONTEND_H
#define __PYTHONFRONTEND_H

#include "shmemmgr.h"
#include <QObject>
#include <iostream>
#include <QThread>

class CommClass : public QObject {
    Q_OBJECT
Q_SIGNALS:
    void dupa(int a);

public Q_SLOTS:
    void dupaSlot() {
        std::cout << "AAA BBB CCCi\n"; 
    }

public:
    int abc(){ return 123; };
};

class PythonFrontend : public QThread {
    Q_OBJECT

public:
    PythonFrontend(const std::string& shmId, const std::string& inQueueName, const std::string& outQueueName);

private:
    void run() override;

private:
    ShMemMgr _shm;
    ShMemMgr::queue_t* _inQueue;
    ShMemMgr::queue_t* _outQueue;
};

#endif /* !__PYTHONFRONTEND_H */
