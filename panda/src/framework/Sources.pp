#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#begin lib_target
  #define TARGET framework
  #define BUILDING_DLL BUILDING_FRAMEWORK
  #define LOCAL_LIBS \
    recorder pgui pgraph putil collide chan text \
    pnmimage pnmimagetypes event

#if $[LINK_ALL_STATIC]
  // If we're statically linking, we need to explicitly link with
  // at least one of the available renderers.
  #if $[HAVE_GL]
    #define LOCAL_LIBS pandagl $[LOCAL_LIBS]
  #elif $[HAVE_DX9]
    #define LOCAL_LIBS pandadx9 $[LOCAL_LIBS]
  #elif $[HAVE_DX8]
    #define LOCAL_LIBS pandadx8 $[LOCAL_LIBS]
  #elif $[HAVE_TINYDISPLAY]
    #define LOCAL_LIBS tinydisplay $[LOCAL_LIBS]
  #endif

  // And we might like to have the egg loader available.
  #define LOCAL_LIBS pandaegg $[LOCAL_LIBS]
#endif


  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define INCLUDED_SOURCES \
    config_framework.cxx config_framework.h \
    pandaFramework.cxx pandaFramework.I pandaFramework.h \
    windowFramework.cxx windowFramework.I windowFramework.h \
    shuttle_controls.bam.c

  #define INSTALL_HEADERS \
    pandaFramework.I pandaFramework.h \
    windowFramework.I windowFramework.h    

#end lib_target
