include_directories(${CMAKE_SOURCE_DIR}/src/common ${CMAKE_SOURCE_DIR}/3rdParty/spdlog/include)
include_directories(${CMAKE_SOURCE_DIR}/3rdParty/cxxopts/include)
include_directories(${CMAKE_SOURCE_DIR}/src/common/cds-python/pythonqt)

# Required for use of "QtCore/private/qmetaobjectbuilder_p.h" in "PythonQt.cpp"
include_directories(${Qt5Core_PRIVATE_INCLUDE_DIRS})

set(SRC
    componentmodel.h
    propertyfields.h
    candbhandler.cpp
    candbpainter.cpp
    sortmodel.cpp
    cds-python/pythonbackend.cpp
    cds-python/shmemmgr.cpp

    cds-python/pythonqt/PythonQtBoolResult.cpp
    cds-python/pythonqt/PythonQtClassInfo.cpp
    cds-python/pythonqt/PythonQtClassWrapper.cpp
    cds-python/pythonqt/PythonQtConversion.cpp
    cds-python/pythonqt/PythonQt.cpp
    cds-python/pythonqt/PythonQtImporter.cpp
    cds-python/pythonqt/PythonQtInstanceWrapper.cpp
    cds-python/pythonqt/PythonQtMethodInfo.cpp
    cds-python/pythonqt/PythonQtMisc.cpp
    cds-python/pythonqt/PythonQtObjectPtr.cpp
    cds-python/pythonqt/PythonQtProperty.cpp
    cds-python/pythonqt/PythonQtQFileImporter.cpp
    cds-python/pythonqt/PythonQtSignal.cpp
    cds-python/pythonqt/PythonQtSignalReceiver.cpp
    cds-python/pythonqt/PythonQtSlot.cpp
    cds-python/pythonqt/PythonQtSlotDecorator.cpp
    cds-python/pythonqt/PythonQtStdDecorators.cpp
    cds-python/pythonqt/PythonQtStdIn.cpp
    cds-python/pythonqt/PythonQtStdOut.cpp
    cds-python/pythonqt/PythonQtThreadSupport.cpp
)

add_library(cds-common STATIC ${SRC})
target_include_directories(cds-common INTERFACE ${CMAKE_SOURCE_DIR}/src/common ${CMAKE_SOURCE_DIR}/3rdParty/spdlog/include ${CMAKE_SOURCE_DIR}/src/common/cds-python/pythonqt)
target_link_libraries(cds-common ${CMAKE_THREAD_LIBS_INIT} Qt5::SerialBus nodes CANdb Qt5::Svg Boost::system ${LIBRT} Python3::Python)

if(WITH_TESTS)
    add_executable(common_test tests/common_test.cpp)
    target_link_libraries(common_test Qt5::Test cds-common fakeit projectconfig)
    add_test(NAME common_test COMMAND common_test)
endif()

