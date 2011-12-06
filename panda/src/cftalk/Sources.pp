#define BUILDING_DLL BUILDING_CFTALK

#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c 

#begin lib_target
  #define TARGET p3cftalk
  #define LOCAL_LIBS \
    p3gsgbase p3gobj p3display \
    p3putil p3linmath p3mathutil p3pnmimage

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
    config_cftalk.h \
    cfChannel.h cfChannel.I \
    cfCommand.h cfCommand.I

  #define INCLUDED_SOURCES \
    cfChannel.cxx \
    cfCommand.cxx

#end lib_target

