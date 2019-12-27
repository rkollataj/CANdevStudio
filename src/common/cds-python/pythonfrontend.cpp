#include "pythonfrontend.h"
#include <cxxopts.hpp>

PythonFrontend::PythonFrontend(const std::string& shmId, const std::string& inQueueName, const std::string& outQueueName)
{
    _shm.openShm(shmId);
    _inQueue = _shm.openQueue(inQueueName);
    _outQueue = _shm.openQueue(outQueueName);
}

bool PythonFrontend::start()
{
    while(true) {
        std::vector<uint8_t> vec = _shm.readQueue(_inQueue);

        std::string str(vec.begin(), vec.end());

        std::cout << "Msg: " << str << std::endl;

        if(str == "End") {
            break;
        }
    }

    std::cout << "Bye bye!" << std::endl;

    return false;
}

int main(int argc, char** argv)
{
    cxxopts::Options options(argv[0], "CANdevStudio Python scripts launcher");

    // clang-format off
    options.add_options()
    ("m,memory", "Shared memory name", cxxopts::value<std::string>(), "name")
    ("o,output", "Shared memory output queue name", cxxopts::value<std::string>(), "name")
    ("i,input", "Shared memory input queue name", cxxopts::value<std::string>(), "name")
    ("s,script", "Python script path", cxxopts::value<std::string>(), "path")
    ("h,help", "Show help message");
    // clang-format on

    const auto&& result = options.parse(argc, argv);

    if ((result.count("m") == 0) || (result.count("o") == 0) || (result.count("i") == 0) || (result.count("s") == 0)
        || result.count("h")) {
        std::cerr << options.help() << std::endl;
        return -1;
    }

     auto shmId = result["m"].as<std::string>();
     auto inQueue = result["i"].as<std::string>();
     auto outQueue = result["o"].as<std::string>();
     auto scriptName = result["s"].as<std::string>();

     PythonFrontend pf(shmId, inQueue, outQueue);
     pf.start();

    return 0;
}
