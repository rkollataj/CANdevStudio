#include "shmemmgr.h"
#include <QUuid>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

namespace {
const std::string cMutexSuffix = "_mutex";
const std::string cCondSuffix = "_condition";
} // namespace

ShMemMgr::~ShMemMgr()
{
    for (auto&& q : _queueId) {
        removeQueue(q);
    }

    if (_shmId.length()) {
        removeShm(_shmId);
    }
}

#include <iostream>

void ShMemMgr::createShm(const std::string& name)
{
    using namespace boost::interprocess;

    if (_shmId.length()) {
        removeShm(_shmId);
    }

    _shmId = name;
    _segment = managed_shared_memory(create_only, _shmId.c_str(), 65536);
    _opened = true;
}

void ShMemMgr::openShm(const std::string& name)
{
    using namespace boost::interprocess;

    if (!_opened) {
        _segment = managed_shared_memory(open_only, name.c_str());
        _opened = true;
    }
}

bool ShMemMgr::removeShm(const std::string& name)
{
    using namespace boost::interprocess;

    return shared_memory_object::remove(name.c_str());
}

ShMemMgr::queue_t* ShMemMgr::createQueue(const std::string& name)
{
    std::string condName = name + cCondSuffix;
    std::string mutexName = name + cMutexSuffix;

    auto c = _segment.construct<boost::interprocess::interprocess_condition>(condName.c_str())();
    auto m = _segment.construct<boost::interprocess::interprocess_mutex>(mutexName.c_str())();
    auto q = _segment.construct<queue_t>(name.c_str())();

    _condMap[q] = std::make_pair(c, m);

    _queueId.push_back(name);

    return q;
}

ShMemMgr::queue_t* ShMemMgr::openQueue(const std::string& name)
{
    std::string condName = name + cCondSuffix;
    std::string mutexName = name + cMutexSuffix;

    auto c = _segment.find<bip::interprocess_condition>(condName.c_str()).first;
    auto m = _segment.find<bip::interprocess_mutex>(mutexName.c_str()).first;
    auto q = _segment.find<queue_t>(name.c_str()).first;

    _condMap[q] = std::make_pair(c, m);

    return q;
}

void ShMemMgr::removeQueue(const std::string& name)
{
    std::string condName = name + cCondSuffix;
    std::string mutexName = name + cMutexSuffix;

    _segment.destroy<bip::interprocess_mutex>(mutexName.c_str());
    _segment.destroy<bip::interprocess_condition>(condName.c_str());
    _segment.destroy<queue_t>(name.c_str());
}

void ShMemMgr::writeQueue(queue_t* queue, const std::vector<uint8_t>& data)
{
    shmVectorAlloc_t alloc(_segment.get_segment_manager());

    // Copy data to shared memory
    shmVector_t shData(data.begin(), data.end(), alloc);
    queue->push(shData);

    _condMap[queue].first->notify_one();
}

std::vector<uint8_t> ShMemMgr::readQueue(queue_t* queue)
{
    std::vector<uint8_t> ret;
    shmVectorAlloc_t alloc(_segment.get_segment_manager());

    shmVector_t shData(alloc);

    if (queue->pop(shData)) {
        // Copy data from shared memory to std::vector
        ret = std::vector<uint8_t>(shData.begin(), shData.end());
    } else {
        bip::scoped_lock<bip::interprocess_mutex> lock(*_condMap[queue].second);
        _condMap[queue].first->wait(lock);

        ret = readQueue(queue);
    }

    return ret;
}

