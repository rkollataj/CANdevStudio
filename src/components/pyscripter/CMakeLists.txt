set(COMPONENT_NAME pyscripter)

set(Boost_USE_STATIC_LIBS ON)

set(CDS_BOOST_FIND system)
set(CDS_BOOST_LIBS Boost::system)
if(WIN32)
    list(APPEND CDS_BOOST_FIND date_time)
    list(APPEND CDS_BOOST_LIBS Boost::date_time)
endif()

find_package(Boost COMPONENTS ${CDS_BOOST_FIND} REQUIRED)

if(WIN32)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${BOOST_LIBRARYDIR})
endif()

find_package(PythonLibs 3)

include_directories(${PYTHON_INCLUDE_DIRS})

if(UNIX AND NOT APPLE)
    find_library(LIBRT rt)
endif()

set(SRC
    pyscripter.cpp
    pyscripter_p.cpp
    pyscriptermodel.cpp
    pythonbackend.cpp
    shmemmgr.cpp
    psmessage.cpp
)

add_library(${COMPONENT_NAME} ${SRC})
target_link_libraries(${COMPONENT_NAME} Qt5::Widgets Qt5::Core Qt5::SerialBus nodes cds-common ${CDS_BOOST_LIBS} ${LIBRT})
# ${CMAKE_SOURCE_DIR}/src to fix filename clash with Python context.h
target_include_directories(${COMPONENT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} PUBLIC ${CMAKE_SOURCE_DIR}/src)

include_directories(${CMAKE_SOURCE_DIR}/3rdParty/cxxopts/include)
include_directories(${CMAKE_SOURCE_DIR}/3rdParty/spdlog/include)
include_directories(${CMAKE_SOURCE_DIR}/src/common)

add_executable(CANdevStudio-python pythonfrontend.cpp shmemmgr.cpp psmessage.cpp)
target_link_libraries(CANdevStudio-python ${LIBRT} ${CMAKE_THREAD_LIBS_INIT} ${CDS_BOOST_LIB} ${PYTHON_LIBRARIES} Qt5::Widgets Qt5::Core Qt5::SerialBus)

# To avoid error in Python/object.h that is using slots keyword
target_compile_definitions(CANdevStudio-python PUBLIC QT_NO_KEYWORDS)

# Add custom target to copy CANdevStudio-python to src/gui dir
# to be able to start it from development build
add_custom_command(
        TARGET CANdevStudio-python POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:CANdevStudio-python>
                ${CMAKE_BINARY_DIR}/src/gui)

install(TARGETS CANdevStudio-python
     BUNDLE DESTINATION . COMPONENT Runtime
     RUNTIME DESTINATION ${INSTALL_DEST_BIN} COMPONENT Runtime
)

if(WITH_TESTS)
    add_executable(${COMPONENT_NAME}_test tests/${COMPONENT_NAME}_test.cpp)
    target_link_libraries(${COMPONENT_NAME}_test ${COMPONENT_NAME} Qt5::Test fakeit)
    add_test(NAME ${COMPONENT_NAME}_test COMMAND ${COMPONENT_NAME}_test)

    add_executable(${COMPONENT_NAME}model_test tests/${COMPONENT_NAME}model_test.cpp)
    target_link_libraries(${COMPONENT_NAME}model_test ${COMPONENT_NAME} Qt5::Test fakeit)
    add_test(NAME ${COMPONENT_NAME}model_test COMMAND ${COMPONENT_NAME}model_test)
endif()
