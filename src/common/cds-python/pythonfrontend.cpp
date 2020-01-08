#include "pythonfrontend.h"
#include <cxxopts.hpp>
#include <log.h>

#include <QApplication>
#include <QThread>

#include <Python.h>
#include <PythonQtClassInfo.h>
#include <PythonQtInstanceWrapper.h>

std::shared_ptr<spdlog::logger> kDefaultLogger;

// clang-format off
static PyMethodDef PythonQtMethods[] = {
  {NULL, NULL, 0, NULL}
};

const char* moduleName = "PythonQt";

static PyModuleDef PythonQtModuleDef = {
  PyModuleDef_HEAD_INIT,
  moduleName,
  NULL,
  -1,
  PythonQtMethods,
  NULL,
  NULL,
  NULL,
  NULL
};
// clang-format on

QHash<QByteArray, PythonQtClassInfo*> _knownClassInfos;
QHash<QByteArray, PyObject*> _packages;
PythonQtObjectPtr _pythonQtModule;
QByteArray _pythonQtModuleName = moduleName;
PythonQtClassInfo* _currentClassInfoForClassWrapperCreation;
QHash<void*, PythonQtInstanceWrapper*> _wrappedObjects;
PythonQtQObjectWrappedCB* _wrappedCB;

PythonQtClassWrapper* createNewPythonQtClassWrapper(
    PythonQtClassInfo* info, PyObject* parentModule, const QByteArray& pythonClassName)
{
    PythonQtClassWrapper* result;

    PyObject* className = PyString_FromString(pythonClassName.constData());

    PyObject* baseClasses = PyTuple_New(1);
    Py_INCREF((PyObject*)&PythonQtInstanceWrapper_Type);
    PyTuple_SET_ITEM(baseClasses, 0, (PyObject*)&PythonQtInstanceWrapper_Type);

    PyObject* typeDict = PyDict_New();
    PyObject* moduleName = PyObject_GetAttrString(parentModule, "__name__");
    PyDict_SetItemString(typeDict, "__module__", moduleName);

    PyObject* args = Py_BuildValue("OOO", className, baseClasses, typeDict);

    // set the class info so that PythonQtClassWrapper_new can read it
    _currentClassInfoForClassWrapperCreation = info;
    // create the new type object by calling the type
    result = (PythonQtClassWrapper*)PyObject_Call((PyObject*)&PythonQtClassWrapper_Type, args, NULL);

    Py_DECREF(baseClasses);
    Py_DECREF(typeDict);
    Py_DECREF(args);
    Py_DECREF(className);

    return result;
}

PyObject* packageByName(const char* name)
{
    if (name == NULL || name[0] == 0) {
        name = "private";
    }
    PyObject* v = _packages.value(name);
    if (!v) {
        v = PyImport_AddModule((_pythonQtModuleName + "." + name).constData());
        _packages.insert(name, v);
        // AddObject steals the reference, so increment it!
        Py_INCREF(v);
        PyModule_AddObject(_pythonQtModule, name, v);
    }
    return v;
}

PythonQtClassInfo* lookupClassInfoAndCreateIfNotPresent(const char* typeName)
{
    PythonQtClassInfo* info = _knownClassInfos.value(typeName);
    if (!info) {
        info = new PythonQtClassInfo();
        info->setupCPPObject(typeName);
        _knownClassInfos.insert(typeName, info);
    }
    return info;
}

void createPythonQtClassWrapper(PythonQtClassInfo* info, const char* package, PyObject* module)
{
    QByteArray pythonClassName = info->className();
    int nestedClassIndex = pythonClassName.indexOf("::");
    bool isNested = false;
    if (nestedClassIndex > 0) {
        pythonClassName = pythonClassName.mid(nestedClassIndex + 2);
        isNested = true;
    }

    PyObject* pack = module ? module : packageByName(package);
    PyObject* pyobj = (PyObject*)createNewPythonQtClassWrapper(info, pack, pythonClassName);

    if (isNested) {
        QByteArray outerClass = QByteArray(info->className()).mid(0, nestedClassIndex);
        PythonQtClassInfo* outerClassInfo = lookupClassInfoAndCreateIfNotPresent(outerClass);
        outerClassInfo->addNestedClass(info);
    } else {
        PyModule_AddObject(pack, info->className(), pyobj);
    }
    if (!module && package && strncmp(package, "Qt", 2) == 0) {
        // since PyModule_AddObject steals the reference, we need a incref once more...
        Py_INCREF(pyobj);
        // put all qt objects into Qt as well
        PyModule_AddObject(packageByName("Qt"), info->className(), pyobj);
    }
    info->setPythonQtClassWrapper(pyobj);
}

void registerClass(const QMetaObject* metaobject)
{
    // we register all classes in the hierarchy
    const QMetaObject* m = metaobject;
    while (m) {
        PythonQtClassInfo* info = lookupClassInfoAndCreateIfNotPresent(m->className());

        info->setTypeSlots(0);
        info->setupQObject(m);
        createPythonQtClassWrapper(info, nullptr, nullptr);
        if (m->superClass()) {
            PythonQtClassInfo* parentInfo = lookupClassInfoAndCreateIfNotPresent(m->superClass()->className());
            info->addParentClass(PythonQtClassInfo::ParentClassInfo(parentInfo));
        }

        m = m->superClass();
    }
}

PyObject* dummyTuple()
{
    static PyObject* dummyTuple = NULL;
    if (dummyTuple == NULL) {
        dummyTuple = PyTuple_New(1);
        PyTuple_SET_ITEM(dummyTuple, 0, PyString_FromString("dummy"));
    }
    return dummyTuple;
}

PythonQtInstanceWrapper* createNewPythonQtInstanceWrapper(
    QObject* obj, PythonQtClassInfo* info, void* wrappedPtr = nullptr)
{
    // call the associated class type to create a new instance...
    PythonQtInstanceWrapper* result
        = (PythonQtInstanceWrapper*)PyObject_Call(info->pythonQtClassWrapper(), dummyTuple(), NULL);

    result->setQObject(obj);
    result->_wrappedPtr = wrappedPtr;
    result->_ownedByPythonQt = false;
    result->_useQMetaTypeDestroy = false;

    if (wrappedPtr || obj) {

        // if this object is reference counted, we ref it:
        PythonQtVoidPtrCB* refCB = info->referenceCountingRefCB();
        if (refCB) {
            (*refCB)(wrappedPtr);
        }

        if (wrappedPtr) {
            _wrappedObjects.insert(wrappedPtr, result);
        } else {
            _wrappedObjects.insert(obj, result);
            if (obj->parent() == NULL && _wrappedCB) {
                // tell someone who is interested that the qobject is wrapped the first time, if it has no parent
                (*_wrappedCB)(obj);
            }
        }
    }
    return result;
}

void removeWrapperPointer(void* obj)
{
    _wrappedObjects.remove(obj);
}

PythonQtInstanceWrapper* findWrapperAndRemoveUnused(void* obj)
{
    PythonQtInstanceWrapper* wrap = _wrappedObjects.value(obj);
    if (wrap && !wrap->_wrappedPtr && wrap->_obj == NULL) {
        // this is a wrapper whose QObject was already removed due to destruction
        // so the obj pointer has to be a new QObject with the same address...
        // we remove the old one and set the copy to NULL
        wrap->_objPointerCopy = NULL;
        removeWrapperPointer(obj);
        wrap = NULL;
    }
    return wrap;
}

PyObject* wrapQObject(QObject* obj)
{
    if (!obj) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    PythonQtInstanceWrapper* wrap = findWrapperAndRemoveUnused(obj);
    if (wrap && wrap->_wrappedPtr) {
        // uh oh, we want to wrap a QObject, but have a C++ wrapper at that
        // address, so probably that C++ wrapper has been deleted earlier and
        // now we see a QObject with the same address.
        // Do not use the old wrapper anymore.
        wrap = NULL;
    }

    if (!wrap) {
        // smuggling it in...
        PythonQtClassInfo* classInfo = _knownClassInfos.value(obj->metaObject()->className());
        if (!classInfo || classInfo->pythonQtClassWrapper() == NULL) {
            registerClass(obj->metaObject());
            classInfo = _knownClassInfos.value(obj->metaObject()->className());
        }
        wrap = createNewPythonQtInstanceWrapper(obj, classInfo);
    } else {
        Py_INCREF(wrap);
    }

    return (PyObject*)wrap;
}

void addObject(PyObject* object, const QString& name, QObject* qObject)
{
    if (PyModule_Check(object)) {
        PyModule_AddObject(object, QStringToPythonCharPointer(name), wrapQObject(qObject));
    } else if (PyDict_Check(object)) {
        PyDict_SetItemString(object, QStringToPythonCharPointer(name), wrapQObject(qObject));
    } else {
        PyObject_SetAttrString(object, QStringToPythonCharPointer(name), wrapQObject(qObject));
    }
}

PythonQtClassInfo* currentClassInfoForClassWrapperCreation()
{
    PythonQtClassInfo* info = _currentClassInfoForClassWrapperCreation;
    _currentClassInfoForClassWrapperCreation = NULL;
    return info;
}

extern void initializeSlots(PythonQtClassWrapper* wrap);

static PyObject* PythonQtClassWrapper_alloc(PyTypeObject* self, Py_ssize_t nitems)
{
    // call the default type alloc
    PyObject* obj = PyType_Type.tp_alloc(self, nitems);

    // take current class type, if we are called via newPythonQtClassWrapper()
    PythonQtClassWrapper* wrap = (PythonQtClassWrapper*)obj;
    wrap->_classInfo = currentClassInfoForClassWrapperCreation();
    if (wrap->_classInfo) {
        initializeSlots(wrap);
    }

    return obj;
}

////////////////////////////////////////////////////////////////

PythonFrontend::PythonFrontend(
    const std::string& shmId, const std::string& inQueueName, const std::string& outQueueName)
{
    _shm.openShm(shmId);
    _inQueue = _shm.openQueue(inQueueName);
    _outQueue = _shm.openQueue(outQueueName);
}

bool PythonFrontend::start()
{
    while (true) {
        std::vector<uint8_t> vec = _shm.readQueue(_inQueue);

        std::string str(vec.begin(), vec.end());

        std::cout << "Msg: " << str << std::endl;

        if (str == "End") {
            break;
        }
    }

    std::cout << "Bye bye!" << std::endl;

    return false;
}

PyObject* evalScript(PyObject* object, const QString& script, int start = Py_file_input)
{
    PythonQtObjectPtr p;
    PyObject* dict = NULL;

    if (PyModule_Check(object)) {
        dict = PyModule_GetDict(object);
    } else if (PyDict_Check(object)) {
        dict = object;
    }

    return PyRun_String(QStringToPythonCharPointer(script), start, dict, dict);
}

/////////////////////////////////////////////////////

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

        PyRun_SimpleString("cds.speak_word.emit(' o w morde!')");

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
