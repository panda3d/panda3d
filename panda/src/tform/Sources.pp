#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET tform
  #define LOCAL_LIBS \
    dgraph graph linmath sgattrib display event putil gobj gsgbase \
    mathutil sgraph device sgraphutil

  #define SOURCES \
    buttonThrower.cxx buttonThrower.h config_tform.cxx config_tform.h \
    dataValve.I dataValve.cxx dataValve.h driveInterface.I \
    driveInterface.cxx driveInterface.h mouseWatcher.I mouseWatcher.cxx \
    mouseWatcher.h mouseWatcherRegion.I mouseWatcherRegion.cxx \
    mouseWatcherRegion.h planarSlider.cxx planarSlider.h trackball.cxx \
    trackball.h \
    transform2sg.cxx transform2sg.h

  #define INSTALL_HEADERS \
    buttonThrower.h dataValve.I dataValve.h \
    driveInterface.I driveInterface.h mouseWatcher.I mouseWatcher.h \
    mouseWatcherRegion.I mouseWatcherRegion.h planarSlider.h \
    trackball.h transform2sg.h

  #define IGATESCAN all

#end lib_target

