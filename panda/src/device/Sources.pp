#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c
                   
#begin lib_target
  #define TARGET p3device
  #define LOCAL_LIBS \
    p3dgraph p3display p3gobj p3gsgbase p3mathutil p3linmath p3putil

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
    analogNode.I analogNode.h \
    buttonNode.I buttonNode.h  \
    clientAnalogDevice.I clientAnalogDevice.h clientBase.I  \
    clientBase.h clientButtonDevice.I clientButtonDevice.h  \
    clientDevice.I clientDevice.h clientDialDevice.I  \
    clientDialDevice.h clientTrackerDevice.I  \
    clientTrackerDevice.h config_device.h \
    dialNode.I dialNode.h  \
    mouseAndKeyboard.h \
    trackerData.I trackerData.h \
    trackerNode.I trackerNode.h \
    virtualMouse.h

  #define INCLUDED_SOURCES \
    analogNode.cxx \
    buttonNode.cxx \
    clientAnalogDevice.cxx  \
    clientBase.cxx clientButtonDevice.cxx clientDevice.cxx  \
    clientDialDevice.cxx clientTrackerDevice.cxx  \
    config_device.cxx \
    dialNode.cxx \
    mouseAndKeyboard.cxx \
    trackerData.cxx \
    trackerNode.cxx \
    virtualMouse.cxx

  #define INSTALL_HEADERS \
    analogNode.I analogNode.h \
    buttonNode.I buttonNode.h \
    clientAnalogDevice.I clientAnalogDevice.h \
    clientBase.I clientBase.h \
    clientButtonDevice.I clientButtonDevice.h \
    clientDevice.I clientDevice.h \
    clientDialDevice.I clientDialDevice.h \
    clientTrackerDevice.I clientTrackerDevice.h \
    config_device.h \
    mouseAndKeyboard.h \
    dialNode.I dialNode.h  \
    trackerData.I trackerData.h \
    trackerNode.I trackerNode.h \
    virtualMouse.h

  #define IGATESCAN all

#end lib_target

