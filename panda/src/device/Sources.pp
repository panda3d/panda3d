#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
                   
#begin lib_target
  #define TARGET device
  #define LOCAL_LIBS \
    dgraph display gobj sgraph graph gsgbase ipc mathutil linmath putil

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
    analogNode.I analogNode.h \
    qpanalogNode.I qpanalogNode.h \
    buttonNode.I buttonNode.h  \
    qpbuttonNode.I qpbuttonNode.h  \
    clientAnalogDevice.I clientAnalogDevice.h clientBase.I  \
    clientBase.h clientButtonDevice.I clientButtonDevice.h  \
    clientDevice.I clientDevice.h clientDialDevice.I  \
    clientDialDevice.h clientTrackerDevice.I  \
    clientTrackerDevice.h config_device.h \
    dialNode.I dialNode.h  \
    qpdialNode.I qpdialNode.h  \
    mouse.h \
    mouseAndKeyboard.h \
    trackerData.I trackerData.h \
    trackerNode.I trackerNode.h \
    qptrackerNode.I qptrackerNode.h \
    virtualMouse.h \
    qpvirtualMouse.h

  #define INCLUDED_SOURCES \
    analogNode.cxx \
    qpanalogNode.cxx \
    buttonNode.cxx \
    qpbuttonNode.cxx \
    clientAnalogDevice.cxx  \
    clientBase.cxx clientButtonDevice.cxx clientDevice.cxx  \
    clientDialDevice.cxx clientTrackerDevice.cxx  \
    config_device.cxx \
    dialNode.cxx \
    qpdialNode.cxx \
    mouse.cxx \
    mouseAndKeyboard.cxx \
    trackerData.cxx \
    trackerNode.cxx \
    qptrackerNode.cxx \
    virtualMouse.cxx \
    qpvirtualMouse.cxx

  #define INSTALL_HEADERS \
    analogNode.I analogNode.h \
    qpanalogNode.I qpanalogNode.h \
    buttonNode.I buttonNode.h \
    qpbuttonNode.I qpbuttonNode.h \
    clientAnalogDevice.I clientAnalogDevice.h \
    clientBase.I clientBase.h \
    clientButtonDevice.I clientButtonDevice.h \
    clientDevice.I clientDevice.h \
    clientDialDevice.I clientDialDevice.h \
    clientTrackerDevice.I clientTrackerDevice.h \
    config_device.h mouse.h \
    mouseAndKeyboard.h \
    dialNode.I dialNode.h  \
    qpdialNode.I qpdialNode.h  \
    trackerData.I trackerData.h \
    trackerNode.I trackerNode.h \
    qptrackerNode.I qptrackerNode.h \
    virtualMouse.h \
    qpvirtualMouse.h

  #define IGATESCAN all

#end lib_target

