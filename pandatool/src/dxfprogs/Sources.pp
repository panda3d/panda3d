#define UNIX_SYS_LIBS m

#begin bin_target
  #define TARGET dxf-points
  #define LOCAL_LIBS \
    progbase dxf

  #define SOURCES \
    dxfPoints.cxx dxfPoints.h

#end bin_target
