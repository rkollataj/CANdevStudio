

set(srcs
    main.cpp
    mainwindow.cpp
    mainwindow.ui
    propertyeditordialog.ui
    propertymodel.cpp
    propertyeditordialog.cpp
)

add_executable(CANdevStudio ${srcs})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../components/")
target_link_libraries(CANdevStudio Qt5::Widgets candevice canrawview canrawsender cds-common nodes projectconfig)
target_compile_definitions(CANdevStudio PRIVATE $<$<CONFIG:Debug>:CDS_DEBUG=true> $<$<NOT:$<CONFIG:Debug>>:CDS_DEBUG=false>)

install(TARGETS CANdevStudio
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT runtime)

set(SRC ${CMAKE_INSTALL_PREFIX}/bin/CANdevStudio)
set(BUNDLE_DIR ${PROJECT_BINARY_DIR}/bundle)
set(DIRS ${CMAKE_INSTALL_PREFIX}/lib)

install(CODE "
   include(BundleUtilities)
   file(COPY \"${SRC}\" DESTINATION \"${BUNDLE_DIR}\")
   fixup_bundle(\"${BUNDLE_DIR}/CANdevStudio\" \"\"   \"${DIRS}\")
   " COMPONENT runtime)


