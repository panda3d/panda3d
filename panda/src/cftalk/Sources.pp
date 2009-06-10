#define BUILDING_DLL BUILDING_CFTALK

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c 

#begin lib_target
  #define TARGET cftalk
  #define LOCAL_LIBS \
    gsgbase gobj display \
    putil linmath mathutil pnmimage

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
    config_cftalk.h \
    cfChannel.h cfChannel.I \
    cfCommand.h cfCommand.I

  #define INCLUDED_SOURCES \
    cfChannel.cxx \
    cfCommand.cxx

#end lib_target

