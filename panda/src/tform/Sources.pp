#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET tform
  #define LOCAL_LIBS \
    dgraph pgraph linmath display event putil gobj gsgbase \
    mathutil device

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES  \
    qpbuttonThrower.h \
    config_tform.h \
    qpdriveInterface.I qpdriveInterface.h \
    qpmouseWatcher.I qpmouseWatcher.h \
    mouseWatcherGroup.h \
    mouseWatcherParameter.I mouseWatcherParameter.h \
    mouseWatcherRegion.I mouseWatcherRegion.h \
    qptrackball.h \
    qptransform2sg.h  
     
  #define INCLUDED_SOURCES  \
    qpbuttonThrower.cxx \
    config_tform.cxx \
    qpdriveInterface.cxx \
    qpmouseWatcher.cxx \
    mouseWatcherGroup.cxx \
    mouseWatcherParameter.cxx mouseWatcherRegion.cxx  \
    qptrackball.cxx \
    qptransform2sg.cxx 

  #define INSTALL_HEADERS \
    qpbuttonThrower.h \
    qpdriveInterface.I qpdriveInterface.h \
    qpmouseWatcher.I qpmouseWatcher.h \
    mouseWatcherGroup.h \
    mouseWatcherParameter.I mouseWatcherParameter.h \
    mouseWatcherRegion.I mouseWatcherRegion.h \
    qptrackball.h \
    qptransform2sg.h

  #define IGATESCAN all

#end lib_target

