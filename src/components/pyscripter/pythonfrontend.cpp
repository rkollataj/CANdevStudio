#include "pythonfrontend.h"
#include <cxxopts.hpp>

#include <QApplication>
#include <QThread>

#include <Python.h>

static PyObject* SpamError;

static PyObject* spam_system(PyObject* self, PyObject* args)
{
    const char* command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;
    sts = system(command);
    if (sts < 0) {
        PyErr_SetString(SpamError, "System command failed");
        return NULL;
    }
    return PyLong_FromLong(sts);
}

static PyObject* spam_dupaSlot(PyObject* self, PyObject* args)
{
    int sts = 0;

    std::cout << "Dupa Slot!\n";

    return PyLong_FromLong(sts);
}

static PyMethodDef SpamMethods[] = {
    { "system", spam_system, METH_VARARGS, "Execute a shell command." },
    { "dupaSlot", spam_dupaSlot, METH_VARARGS, "Execute a shell command." },
    { NULL, NULL, 0, NULL } /* Sentinel */
};

static struct PyModuleDef spammodule = { PyModuleDef_HEAD_INIT, "spam", /* name of module */
    nullptr, /* module documentation, may be NULL */
    -1, /* size of per-interpreter state of the module,
           or -1 if the module keeps state in global variables. */
    SpamMethods };

PyMODINIT_FUNC PyInit_spam(void)
{
    return PyModule_Create(&spammodule);
}

int main(int argc, char** argv)
{
    wchar_t* program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }

    /* Add a built-in module, before Py_Initialize */
    PyImport_AppendInittab("spam", PyInit_spam);

    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(program);

    /* Initialize the Python interpreter.  Required. */
    Py_Initialize();

    PyEval_InitThreads();
    //PyEval_SaveThread();

    PyRun_SimpleString(R"(
import sys
from PySide2.QtCore import QObject, Signal, Slot
from PySide2.QtWidgets import QApplication

class Communicate(QObject):
    # create two new signals on the fly: one will handle
    # int type, the other will handle strings
    speak_number = Signal(int)
    speak_word = Signal(str)

app = QApplication(sys.argv)

)");


    auto th = QThread::create([]{
        std::cout<<"przed sleep\n";
        QThread::sleep(2);
        std::cout<<"po sleep\n";
        
        auto state = PyGILState_Ensure();

        PyRun_SimpleString("cds.speak_word.emit(' o w mordeeeee!')");

        PyGILState_Release(state);
    });

    QObject::connect(th, &QThread::started, [] {

            std::cout<<"th started\n";

        });


    QObject::connect(th, &QThread::finished, [] {

            std::cout<<"th finished\n";

        });

    th->start();

    auto state = PyGILState_Ensure();

    PyRun_SimpleString(R"(
import sys
import spam
from PySide2.QtWidgets import QApplication, QPushButton, QWidget, QVBoxLayout, QLabel

def func(word):
    label.setText("A qq " + word)

layout = QVBoxLayout()
widget = QWidget()
widget.setLayout(layout)

button = QPushButton("Call func")
button.clicked.connect(spam.dupaSlot)

label = QLabel("Dummy text")

layout.addWidget(button)
layout.addWidget(label)

widget.show()

cds = Communicate()
cds.speak_word.connect(func)

cds.speak_word.emit("aaaaaa")

app.exec_()
)");

        PyGILState_Release(state);

            std::cout<<"zzzzz\n";

    PyMem_RawFree(program);

    // cxxopts::Options options(argv[0], "CANdevStudio Python scripts launcher");

    //// clang-format off
    // options.add_options()
    //("m,memory", "Shared memory name", cxxopts::value<std::string>(), "name")
    //("o,output", "Shared memory output queue name", cxxopts::value<std::string>(), "name")
    //("i,input", "Shared memory input queue name", cxxopts::value<std::string>(), "name")
    //("s,script", "Python script path", cxxopts::value<std::string>(), "path")
    //("h,help", "Show help message");
    //// clang-format on

    // const auto&& result = options.parse(argc, argv);

    // if ((result.count("m") == 0) || (result.count("o") == 0) || (result.count("i") == 0) || (result.count("s") == 0)
    //    || result.count("h")) {
    //    std::cerr << options.help() << std::endl;
    //    return -1;
    //}

    // auto shmId = result["m"].as<std::string>();
    // auto inQueue = result["i"].as<std::string>();
    // auto outQueue = result["o"].as<std::string>();
    // auto scriptName = result["s"].as<std::string>();

    // PythonFrontend pf(shmId, inQueue, outQueue);
    // pf.start();

    //    QApplication qapp(argc, argv);
    //    CommClass comm;

    //    wchar_t* program = Py_DecodeLocale(argv[0], NULL);
    //    if (program == NULL) {
    //        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
    //        exit(1);
    //    }
    //    Py_SetProgramName(program);
    //    Py_Initialize();

    //    PyObject* dict = PyImport_GetModuleDict();
    //    PyObject* obj = PyDict_GetItemString(dict, "__main__");

    //    PythonQtClassWrapper_Type.tp_base = &PyType_Type;
    //    PythonQtClassWrapper_Type.tp_alloc = PythonQtClassWrapper_alloc;

    //    // add our own python object types for classes
    //    if (PyType_Ready(&PythonQtClassWrapper_Type) < 0) {
    //        std::cerr << "could not initialize PythonQtClassWrapper_Type"
    //                  << ", in " << __FILE__ << ":" << __LINE__ << std::endl;
    //    }
    //    Py_INCREF(&PythonQtClassWrapper_Type);

    //    _pythonQtModule = PyModule_Create(&PythonQtModuleDef);

    //    addObject(obj, "comm", &comm);

    //    evalScript(obj, R"(

    // from time import time,ctime
    //#print('Today is ', comm.abc())
    // print('Today is ')

    //)");

    //    if (Py_FinalizeEx() < 0) {
    //        exit(120);
    //    }

    //    PyMem_RawFree(program);

    // init PythonQt and Python itself
    // PythonQt::init();
    // get a smart pointer to the __main__ module of the Python interpreter
    // PythonQtObjectPtr context = PythonQt::self()->getMainModule();
    // add a QObject as variable of name "example" to the namespace of the __main__ module
    // PyExampleObject example;
    // context.addObject("example", &example);
    // do something
    // context.evalScript("print example");
    // context.evalScript("def multiply(a,b):\n  return a*b;\n");
    // QVariantList args;
    // args << 42 << 47;
    // QVariant result = context.call("multiply", args);

    return 0;
}
