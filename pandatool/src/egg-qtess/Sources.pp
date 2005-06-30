#begin bin_target
  #define TARGET egg-qtess
  #define LOCAL_LIBS \
    eggbase progbase
  #define OTHER_LIBS \
    chan:c char:c downloader:c egg2pg:c event:c lerp:c \
    tform:c grutil:c text:c dgraph:c display:c gsgbase:c collide:c gobj:c \
    parametrics:c pgraph:c egg:c pandaegg:m \
    pnmimagetypes:c pnmimage:c mathutil:c linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub
  #define UNIX_SYS_LIBS m

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx     
  
  #define SOURCES \
     config_egg_qtess.h \
     eggQtess.h \
     isoPlacer.I isoPlacer.h \
     qtessGlobals.h \
     qtessInputEntry.I qtessInputEntry.h \
     qtessInputFile.I qtessInputFile.h \
     qtessSurface.I qtessSurface.h \
     subdivSegment.I subdivSegment.h
  
  #define INCLUDED_SOURCES \
     config_egg_qtess.cxx \
     eggQtess.cxx \
     isoPlacer.cxx \
     qtessGlobals.cxx \
     qtessInputEntry.cxx \
     qtessInputFile.cxx \
     qtessSurface.cxx \
     subdivSegment.cxx

#end bin_target

