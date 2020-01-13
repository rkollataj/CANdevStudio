#include "pythonfrontend.h"
#include "psmessage.h"
#include <Python.h>
#include <QApplication>
#include <QThread>
#include <cxxopts.hpp>
#include <datamodeltypes/datadirection.h>
#include <spdlog/fmt/fmt.h>

namespace {
PyMethodDef cdsCommMethods[] = { { "sndFrame", PythonFrontend::sndFrame, METH_VARARGS, "" }, { NULL, NULL, 0, NULL } };

PyModuleDef cdsCommModule
    = { PyModuleDef_HEAD_INIT, "cdsCommModule", NULL, -1, cdsCommMethods, NULL, NULL, NULL, NULL };

PyObject* PyInit_cdsCommModule(void)
{
    return PyModule_Create(&cdsCommModule);
}
} // namespace

PythonFrontend* PythonFrontend::_thisWrapper;

PythonFrontend::PythonFrontend(
    const std::string& shmId, const std::string& inQueueName, const std::string& outQueueName)
{
    _shm.openShm(shmId);
    _inQueue = _shm.openQueue(inQueueName);
    _outQueue = _shm.openQueue(outQueueName);

    _thisWrapper = this;
}

PyObject* PythonFrontend::sndFrame(PyObject*, PyObject* args)
{
    uint32_t id;
    std::vector<uint8_t> payload;
    PyObject* pList;

    if (!PyArg_ParseTuple(args, "IO!", &id, &PyList_Type, &pList)) {
        PyErr_SetString(PyExc_TypeError, "expected parameters (uint, list).");
        return nullptr;
    }

    Py_ssize_t n = PyList_Size(pList);
    for (int i = 0; i < n; i++) {
        PyObject* pItem = PyList_GetItem(pList, i);
        if (!PyLong_Check(pItem)) {
            PyErr_SetString(PyExc_TypeError, "list items must be integers.");
            return nullptr;
        }

        payload.push_back(PyLong_AsUnsignedLong(pItem));
    }

    PsMessage msg = PsMessage::fromFrame(id, payload, Direction::TX);

    _thisWrapper->_shm.writeQueue(_thisWrapper->_outQueue, msg.toArray());

    Py_INCREF(Py_None);
    return Py_None;
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
            std::string dir;

            if (msg.toFrame(id, payload, dir)) {
                std::string frameLine = fmt::format("cdsComm.rcvFrame.emit({}, [", id);

                for (std::size_t i = 0; i < payload.size(); ++i) {
                    if (i > 0) {
                        frameLine += ",";
                    }

                    frameLine += fmt::format("{}", payload[i]);
                }

                frameLine += fmt::format("],'{}')", dir);

                auto state = PyGILState_Ensure();

                PyRun_SimpleString(frameLine.c_str());

                PyGILState_Release(state);
            }
        }
    }

    QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
}

void PythonFrontend::sendBackendCloseMsg()
{
    _shm.writeQueue(_outQueue, PsMessage::createCloseMessage().toArray());
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

    PyImport_AppendInittab("cdsCommModule", &PyInit_cdsCommModule);

    // Initialize the Python interpreter.  Required.
    Py_Initialize();

    PyEval_InitThreads();

    PyRun_SimpleString(R"(
import sys
import cdsCommModule
from PySide2.QtCore import QObject, Signal, Slot
from PySide2.QtWidgets import QApplication

class CdsComm(QObject):
    rcvFrame = Signal(int, list, str)

    def sndFrame(self, id, payload):
        cdsCommModule.sndFrame(id, payload)

app = QApplication(sys.argv)
cdsComm = CdsComm()

)");

    PythonFrontend pf(shmId, inQueue, outQueue);
    pf.start();

    auto state = PyGILState_Ensure();
    PyRun_SimpleString(R"(
#cdsComm.rcvFrame.connect(lambda id, payload, dir: print("Frame: ", id, " ", payload, " ", dir))
cdsComm.rcvFrame.connect(cdsComm.sndFrame)
app.exec_()
)");
    PyGILState_Release(state);

    pf.sendBackendCloseMsg();
    pf.wait();

    PyMem_RawFree(program);

    return 0;
}
