add_subdirectory(projectconfig)

set(srcs
    main.cpp
    mainwindow.cpp
    mainwindow.ui
    toolbar.ui
    aboutdialog.ui
)

add_executable(CANdevStudio MACOSX_BUNDLE ${GUI_TYPE} ${srcs})
target_link_libraries(CANdevStudio projectconfig)
target_compile_definitions(CANdevStudio PRIVATE $<$<CONFIG:Debug>:CDS_DEBUG=true> $<$<NOT:$<CONFIG:Debug>>:CDS_DEBUG=false>)
target_include_directories(CANdevStudio 
    PRIVATE
        "${CMAKE_SOURCE_DIR}/components/"
        "${CMAKE_BINARY_DIR}/generated"
)

include_directories(${CMAKE_SOURCE_DIR}/3rdParty/cxxopts/include)

add_executable(CANdevStudio-python 
    ${CMAKE_SOURCE_DIR}/src/common/cds-python/pythonfrontend.cpp 
    ${CMAKE_SOURCE_DIR}/src/common/cds-python/shmemmgr.cpp)
target_link_libraries(CANdevStudio-python ${LIBRT} ${CMAKE_THREAD_LIBS_INIT} Boost::system cds-common)

# To avoid error in Python/object.h that is using slots keyword
target_compile_definitions(CANdevStudio-python PUBLIC QT_NO_KEYWORDS) 

install(TARGETS CANdevStudio CANdevStudio-python
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION ${INSTALL_DEST_BIN} COMPONENT Runtime
)
