include_directories(${CMAKE_SOURCE_DIR}/src/common ${CMAKE_SOURCE_DIR}/3rdParty/spdlog/include)
include_directories(${CMAKE_SOURCE_DIR}/3rdParty/cxxopts/include)

set(SRC
    componentmodel.h
    propertyfields.h
    candbhandler.cpp
    candbpainter.cpp
    sortmodel.cpp
    cds-python/pythonbackend.cpp
    cds-python/shmemmgr.cpp
)

add_library(cds-common STATIC ${SRC})
target_include_directories(cds-common INTERFACE ${CMAKE_SOURCE_DIR}/src/common ${CMAKE_SOURCE_DIR}/3rdParty/spdlog/include)
target_link_libraries(cds-common ${CMAKE_THREAD_LIBS_INIT} Qt5::SerialBus nodes CANdb Qt5::Svg Boost::system ${LIBRT})

if(WITH_TESTS)
    add_executable(common_test tests/common_test.cpp)
    target_link_libraries(common_test Qt5::Test cds-common fakeit projectconfig)
    add_test(NAME common_test COMMAND common_test)
endif()

