#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3tform
  #define LOCAL_LIBS \
    p3grutil p3dgraph p3pgraph p3linmath p3display p3event p3putil p3gobj p3gsgbase \
    p3mathutil p3device

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES  \
    buttonThrower.I buttonThrower.h \
    config_tform.h \
    driveInterface.I driveInterface.h \
    mouseInterfaceNode.I mouseInterfaceNode.h \
    mouseSubregion.I mouseSubregion.h \
    mouseWatcher.I mouseWatcher.h \
    mouseWatcherBase.h mouseWatcherGroup.h \
    mouseWatcherParameter.I mouseWatcherParameter.h \
    mouseWatcherRegion.I mouseWatcherRegion.h \
    trackball.h \
    transform2sg.h

  #define INCLUDED_SOURCES  \
    buttonThrower.cxx \
    config_tform.cxx \
    driveInterface.cxx \
    mouseInterfaceNode.cxx \
    mouseSubregion.cxx \
    mouseWatcher.cxx \
    mouseWatcherBase.cxx \
    mouseWatcherGroup.cxx \
    mouseWatcherParameter.cxx \
    mouseWatcherRegion.cxx \
    trackball.cxx \
    transform2sg.cxx

  #define INSTALL_HEADERS \
    buttonThrower.I buttonThrower.h \
    driveInterface.I driveInterface.h \
    mouseInterfaceNode.I mouseInterfaceNode.h \
    mouseSubregion.I mouseSubregion.h \
    mouseWatcher.I mouseWatcher.h \
    mouseWatcherBase.h mouseWatcherGroup.h \
    mouseWatcherParameter.I mouseWatcherParameter.h \
    mouseWatcherRegion.I mouseWatcherRegion.h \
    trackball.h \
    transform2sg.h

  #define IGATESCAN all

#end lib_target

