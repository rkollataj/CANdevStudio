#include "shmemmgr.h"

namespace
{
    const std::string cCondSuffix = "_condition";
}

ShMemMgr::~ShMemMgr()
{
    for (auto&& q : _queueId) {
        removeQueue(q);
    }

    if (_shmId.length()) {
        removeShm(_shmId);
    }
}

void ShMemMgr::createShm(const std::string& name)
{
    using namespace boost::interprocess;

    if (_shmId.length()) {
        removeShm(_shmId);
    }

    _shmId = name;
    _segment = managed_shared_memory(create_only, _shmId.c_str(), 65536);
}

void ShMemMgr::openShm(const std::string& name)
{
    using namespace boost::interprocess;

    _segment = managed_shared_memory(open_only, name.c_str());
}

bool ShMemMgr::removeShm(const std::string& name)
{
    using namespace boost::interprocess;

    return shared_memory_object::remove(name.c_str());
}

void ShMemMgr::createQueue(const std::string& name)
{
    std::string condName = name + cCondSuffix;

    _segment.construct<boost::interprocess::interprocess_condition>(condName.c_str());
    _segment.construct<queue_t>(name.c_str())();
    _queueId.push_back(name);
}

ShMemMgr::queue_t* ShMemMgr::openQueue(const std::string& name)
{
    return _segment.find<queue_t>(name.c_str()).first;
}

bool ShMemMgr::removeQueue(const std::string& name)
{
    return _segment.destroy<queue_t>(name.c_str());
}

void ShMemMgr::writeQueue(queue_t* queue, const std::vector<uint8_t>& data)
{
    shmVectorAlloc_t alloc(_segment.get_segment_manager());

    // Copy data to shared memory
    shmVector_t shData(data.begin(), data.end(), alloc);
    queue->push(shData);
}

std::vector<uint8_t> ShMemMgr::readQueue(queue_t* queue)
{
    std::vector<uint8_t> ret;
    shmVectorAlloc_t alloc(_segment.get_segment_manager());

    shmVector_t shData(alloc);

    if(queue->pop(shData)) {
        // Copy data from shared memory to std::vector 
        ret = std::vector<uint8_t>(shData.begin(), shData.end());
    }

    return ret;
}

