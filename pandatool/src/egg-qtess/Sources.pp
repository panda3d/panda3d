#define USE_PACKAGES cg  // from gobj.

#begin bin_target
  #define TARGET egg-qtess
  #define LOCAL_LIBS \
    p3eggbase p3progbase
  #define OTHER_LIBS \
    p3egg2pg:c p3egg:c pandaegg:m \
    p3chan:c p3char:c p3downloader:c p3event:c \
    p3tform:c p3grutil:c p3text:c p3dgraph:c p3display:c p3gsgbase:c \
    p3collide:c p3gobj:c p3cull:c p3device:c \
    p3parametrics:c p3pgraph:c p3pgraphnodes:c p3pipeline:c p3pstatclient:c p3chan:c \
    p3pnmimagetypes:c p3pnmimage:c p3mathutil:c p3linmath:c p3putil:c \
    p3movies:c \
    $[if $[HAVE_FREETYPE],p3pnmtext:c] \
    $[if $[HAVE_NET],p3net:c] $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    $[if $[HAVE_AUDIO],p3audio:c] \
    panda:m \
    p3pandabase:c p3express:c pandaexpress:m \
    p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m p3pystub
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

