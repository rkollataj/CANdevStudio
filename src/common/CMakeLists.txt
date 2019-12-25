find_package(Threads REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/src/common ${CMAKE_SOURCE_DIR}/3rdParty/spdlog/include)
include_directories(${CMAKE_SOURCE_DIR}/3rdParty/cxxopts/include)

set(SRC
    componentmodel.h
    propertyfields.h
    candbhandler.cpp
    candbpainter.cpp
    sortmodel.cpp
    pythonbackend.cpp
    shmemmgr.cpp
)

add_library(cds-common STATIC ${SRC})
target_include_directories(cds-common INTERFACE ${CMAKE_SOURCE_DIR}/src/common ${CMAKE_SOURCE_DIR}/3rdParty/spdlog/include)
target_link_libraries(cds-common ${CMAKE_THREAD_LIBS_INIT} Qt5::SerialBus nodes CANdb Qt5::Svg Boost::system ${LIBRT})

add_executable(cds-python pythonfrontend.cpp shmemmgr.cpp)
target_link_libraries(cds-python ${LIBRT} ${CMAKE_THREAD_LIBS_INIT} Boost::system)

if(WITH_TESTS)
    add_executable(common_test tests/common_test.cpp)
    target_link_libraries(common_test Qt5::Test cds-common fakeit projectconfig)
    add_test(NAME common_test COMMAND common_test)
endif()

