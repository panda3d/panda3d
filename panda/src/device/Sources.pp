#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET device
  #define LOCAL_LIBS \
    dgraph display gobj sgraph graph gsgbase ipc mathutil linmath putil

  #define SOURCES \
    adinputNode.cxx adinputNode.h analogData.I analogData.cxx \
    analogData.h buttonData.I buttonData.cxx buttonData.h \
    clientBase.cxx clientBase.h config_device.cxx config_device.h \
    dialData.I dialData.cxx dialData.h mouse.cxx mouse.h trackerData.I \
    trackerData.cxx trackerData.h trackerNode.cxx trackerNode.h

  #define INSTALL_HEADERS \
    adinputNode.h analogData.I analogData.h buttonData.I buttonData.h \
    clientBase.h dialData.I dialData.h mouse.h trackerData.I \
    trackerData.h trackerNode.h

  #define IGATESCAN all

#end lib_target

