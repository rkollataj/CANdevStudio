#ifndef __SHMEMMGR_H
#define __SHMEMMGR_H

#include <string>
#include <vector>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/lockfree/spsc_queue.hpp>

namespace bip = boost::interprocess;

class ShMemMgr {
    using shmVectorAlloc_t
        = boost::interprocess::allocator<uint8_t, boost::interprocess::managed_shared_memory::segment_manager>;
    using shmVector_t = boost::interprocess::vector<uint8_t, shmVectorAlloc_t>;

public:
    ~ShMemMgr();

    void createShm(const std::string& name);
    void openShm(const std::string& name);

    using queue_t = boost::lockfree::spsc_queue<shmVector_t, boost::lockfree::capacity<200>>;

    queue_t* createQueue(const std::string& name);
    queue_t* openQueue(const std::string& name);
    void writeQueue(queue_t* queue, const std::vector<uint8_t>& data);
    std::vector<uint8_t> readQueue(queue_t* queue);

private:
    void removeQueue(const std::string& name);
    bool removeShm(const std::string& name);

private:
    std::string _shmId;
    std::vector<std::string> _queueId;
    boost::interprocess::managed_shared_memory _segment;
    std::map<queue_t*, std::pair<bip::interprocess_condition*, bip::interprocess_mutex*>> _condMap;
    bool _opened{ false };
};

#endif /* !__SHMEMMGR_H */
