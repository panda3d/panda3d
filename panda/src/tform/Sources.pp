#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET tform
  #define LOCAL_LIBS \
    dgraph graph linmath sgattrib display event putil gobj gsgbase \
    mathutil sgraph device sgraphutil

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES  \
    buttonThrower.h config_tform.h dataValve.I dataValve.h  \
    driveInterface.I driveInterface.h mouseWatcher.I  \
    mouseWatcher.h mouseWatcherGroup.h \
    mouseWatcherParameter.I mouseWatcherParameter.h  \
    mouseWatcherRegion.I mouseWatcherRegion.h  \
    planarSlider.h trackball.h transform2sg.h  
     
  #define INCLUDED_SOURCES  \
    buttonThrower.cxx config_tform.cxx dataValve.cxx  \
    driveInterface.cxx mouseWatcher.cxx mouseWatcherGroup.cxx \
    mouseWatcherParameter.cxx mouseWatcherRegion.cxx  \
    planarSlider.cxx trackball.cxx transform2sg.cxx 

  #define INSTALL_HEADERS \
    buttonThrower.h dataValve.I dataValve.h \
    driveInterface.I driveInterface.h mouseWatcher.I mouseWatcher.h \
    mouseWatcherGroup.h \
    mouseWatcherParameter.I mouseWatcherParameter.h \
    mouseWatcherRegion.I mouseWatcherRegion.h \
    planarSlider.h trackball.h transform2sg.h

  #define IGATESCAN all

#end lib_target

