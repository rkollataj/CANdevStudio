#include "pythonfrontend.h"
#include "psmessage.h"
#include <Python.h>
#include <QApplication>
#include <QThread>
#include <cxxopts.hpp>
#include <spdlog/fmt/fmt.h>

PythonFrontend::PythonFrontend(
    const std::string& shmId, const std::string& inQueueName, const std::string& outQueueName)
{
    _shm.openShm(shmId);
    _inQueue = _shm.openQueue(inQueueName);
    _outQueue = _shm.openQueue(outQueueName);
}

void PythonFrontend::run()
{
    PsMessage msg;

    while (msg.type() != PsMessageType::CLOSE) {
        std::vector<uint8_t> vec = _shm.readQueue(_inQueue);

        msg = PsMessage::fromData(vec);

        if (msg.type() == PsMessageType::FRAME) {
            uint32_t id;
            std::vector<uint8_t> payload;

            if (msg.toFrame(id, payload)) {
                std::string frameLine = fmt::format("cdsComm.rcvFrame.emit({}, [", id);

                for (std::size_t i = 0; i < payload.size(); ++i) {
                    if (i > 0) {
                        frameLine += ",";
                    }

                    frameLine += fmt::format("{}",payload[i]);
                }

                frameLine += "])";

                auto state = PyGILState_Ensure();

                PyRun_SimpleString(frameLine.c_str());

                PyGILState_Release(state);
            }
        }
    }

    QApplication::quit();
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

    wchar_t* program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }

    // Pass argv[0] to the Python interpreter
    Py_SetProgramName(program);

    // Initialize the Python interpreter.  Required.
    Py_Initialize();

    PyEval_InitThreads();

    PyRun_SimpleString(R"(
import sys
from PySide2.QtCore import QObject, Signal, Slot
from PySide2.QtWidgets import QApplication

class CdsComm(QObject):
    # create two new signals on the fly: one will handle
    # int type, the other will handle strings
    rcvFrame = Signal(int, list)

app = QApplication(sys.argv)
cdsComm = CdsComm()

)");

    PythonFrontend pf(shmId, inQueue, outQueue);
    pf.start();

    auto state = PyGILState_Ensure();
    PyRun_SimpleString(R"(
cdsComm.rcvFrame.connect(lambda id, payload: print("Frame: ", id, " ", payload)) 
app.exec_()
)");
    PyGILState_Release(state);

    pf.wait();

    std::cout << "bye bye!\n";

    PyMem_RawFree(program);

    return 0;
}

