#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
                   
#begin lib_target
  #define TARGET device
  #define LOCAL_LIBS \
    dgraph display gobj gsgbase ipc mathutil linmath putil

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
    qpanalogNode.I qpanalogNode.h \
    qpbuttonNode.I qpbuttonNode.h  \
    clientAnalogDevice.I clientAnalogDevice.h clientBase.I  \
    clientBase.h clientButtonDevice.I clientButtonDevice.h  \
    clientDevice.I clientDevice.h clientDialDevice.I  \
    clientDialDevice.h clientTrackerDevice.I  \
    clientTrackerDevice.h config_device.h \
    qpdialNode.I qpdialNode.h  \
    mouseAndKeyboard.h \
    trackerData.I trackerData.h \
    qptrackerNode.I qptrackerNode.h \
    qpvirtualMouse.h

  #define INCLUDED_SOURCES \
    qpanalogNode.cxx \
    qpbuttonNode.cxx \
    clientAnalogDevice.cxx  \
    clientBase.cxx clientButtonDevice.cxx clientDevice.cxx  \
    clientDialDevice.cxx clientTrackerDevice.cxx  \
    config_device.cxx \
    dialNode.cxx \
    qpdialNode.cxx \
    mouseAndKeyboard.cxx \
    trackerData.cxx \
    qptrackerNode.cxx \
    qpvirtualMouse.cxx

  #define INSTALL_HEADERS \
    qpanalogNode.I qpanalogNode.h \
    qpbuttonNode.I qpbuttonNode.h \
    clientAnalogDevice.I clientAnalogDevice.h \
    clientBase.I clientBase.h \
    clientButtonDevice.I clientButtonDevice.h \
    clientDevice.I clientDevice.h \
    clientDialDevice.I clientDialDevice.h \
    clientTrackerDevice.I clientTrackerDevice.h \
    config_device.h \
    mouseAndKeyboard.h \
    qpdialNode.I qpdialNode.h  \
    trackerData.I trackerData.h \
    qptrackerNode.I qptrackerNode.h \
    qpvirtualMouse.h

  #define IGATESCAN all

#end lib_target

