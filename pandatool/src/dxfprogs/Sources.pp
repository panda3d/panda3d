#define UNIX_SYS_LIBS m

#begin bin_target
  #define TARGET dxf-points
  #define LOCAL_LIBS \
    progbase dxf
  #define OTHER_LIBS \
    linmath:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    dxfPoints.cxx dxfPoints.h

#end bin_target
