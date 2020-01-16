#include "pythonfrontend.h"
#include "psmessage.h"
#include <Python.h>
#include <QApplication>
#include <QThread>
#include <cxxopts.hpp>
#include <datamodeltypes/datadirection.h>
#include <spdlog/fmt/fmt.h>

namespace {

// clang-format off
PyMethodDef cdsCommMethods[] = {
    { "init", PythonFrontend::init, METH_VARARGS, "" },
    { "sndFrame", PythonFrontend::sndFrame, METH_VARARGS, "" },
    { "sndSignal", PythonFrontend::sndSignal, METH_VARARGS, "" },
    { nullptr, nullptr, 0, nullptr }
};
// clang-format on

PyModuleDef cdsCommModule
    = { PyModuleDef_HEAD_INIT, "cdsCommModule", nullptr, -1, cdsCommMethods, nullptr, nullptr, nullptr, nullptr };

PyObject* PyInit_cdsCommModule(void)
{
    return PyModule_Create(&cdsCommModule);
}

std::unique_ptr<PythonFrontend> pf;

} // namespace

std::string PythonFrontend::shmId;
std::string PythonFrontend::inQueue;
std::string PythonFrontend::outQueue;
std::string PythonFrontend::scriptName;

PythonFrontend::PythonFrontend()
{
    _shm.openShm(PythonFrontend::shmId);
    _inQueue = _shm.openQueue(PythonFrontend::inQueue);
    _outQueue = _shm.openQueue(PythonFrontend::outQueue);
}

PyObject* PythonFrontend::init(PyObject*, PyObject*)
{
    pf = std::make_unique<PythonFrontend>();
    pf->start();

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject* PythonFrontend::sndFrame(PyObject*, PyObject* args)
{
    uint32_t id;
    std::vector<uint8_t> payload;
    PyObject* pList;

    if (!PyArg_ParseTuple(args, "IO!", &id, &PyList_Type, &pList)) {
        PyErr_SetString(PyExc_TypeError, "expected parameters (uint, list[uint]).");
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

    pf->_shm.writeQueue(pf->_outQueue, msg.toArray());

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject* PythonFrontend::sndSignal(PyObject*, PyObject* args)
{
    uint32_t id;
    const char* name;
    double val;

    if (!PyArg_ParseTuple(args, "Isd", &id, &name, &val)) {
        PyErr_SetString(PyExc_TypeError, "expected parameters (uint, str, float).");
        return nullptr;
    }

    PsMessage msg = PsMessage::fromSignal(id, name, val);

    pf->_shm.writeQueue(pf->_outQueue, msg.toArray());

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
        } else if (msg.type() == PsMessageType::SIGNAL) {
            uint32_t id;
            std::string dir;
            double value;
            std::string name;

            if (msg.toSignal(id, name, value, dir)) {
                std::string frameLine = fmt::format("cdsComm.rcvSignal.emit({}, '{}', {}, '{}')", id, name, value, dir);

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

    PythonFrontend::shmId = result["m"].as<std::string>();
    PythonFrontend::inQueue = result["i"].as<std::string>();
    PythonFrontend::outQueue = result["o"].as<std::string>();
    PythonFrontend::scriptName = result["s"].as<std::string>();

    wchar_t* program = Py_DecodeLocale(argv[0], nullptr);
    if (program == nullptr) {
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
from PySide2.QtCore import QObject, Signal

class CdsComm(QObject):
    rcvFrame = Signal(int, list, str)
    rcvSignal = Signal(int, str, float, str)

    def sndFrame(self, id, payload):
        cdsCommModule.sndFrame(id, payload)

    def sndSignal(self, id, name, val):
        cdsCommModule.sndSignal(id, name, val)

    def init(self):
        cdsCommModule.init()
)");

    auto state = PyGILState_Ensure();
    PyRun_SimpleString(R"(
from PySide2.QtWidgets import QApplication

app = QApplication(sys.argv)

cdsComm = CdsComm()
cdsComm.init();

# Loopback setup
cdsComm.rcvFrame.connect(cdsComm.sndFrame)
cdsComm.rcvSignal.connect(cdsComm.sndSignal)
app.exec_()
)");
    PyGILState_Release(state);

    pf->sendBackendCloseMsg();
    pf->wait();

    PyMem_RawFree(program);

    return 0;
}
