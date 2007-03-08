#begin bin_target
  #define TARGET egg-qtess
  #define LOCAL_LIBS \
    eggbase progbase
  #define OTHER_LIBS \
    egg2pg:c egg:c pandaegg:m \
    chan:c char:c downloader:c event:c lerp:c \
    tform:c grutil:c text:c dgraph:c display:c gsgbase:c \
    collide:c gobj:c cull:c device:c pnmtext:c \
    parametrics:c pgraph:c pipeline:c pstatclient:c chan:c \
    pnmimagetypes:c pnmimage:c mathutil:c linmath:c putil:c \
    $[if $[HAVE_NET],net:c] $[if $[WANT_NATIVE_NET],nativenet:c] \
    panda:m \
    pandabase:c express:c pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub
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

