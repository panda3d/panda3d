#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3framework
  #define BUILDING_DLL BUILDING_FRAMEWORK
  #define LOCAL_LIBS \
    p3recorder p3pgui p3pgraph p3putil p3collide p3chan p3text \
    p3pnmimage p3pnmimagetypes p3event p3char

#if $[LINK_ALL_STATIC]
  // If we're statically linking, we need to explicitly link with
  // at least one of the available renderers.
  #if $[HAVE_GL]
    #define LOCAL_LIBS pandagl $[LOCAL_LIBS]
  #elif $[HAVE_DX9]
    #define LOCAL_LIBS pandadx9 $[LOCAL_LIBS]
  #elif $[HAVE_TINYDISPLAY]
    #define LOCAL_LIBS p3tinydisplay $[LOCAL_LIBS]
  #endif

  // And we might like to have the p3egg loader available.
  #if $[HAVE_EGG]
    #define LOCAL_LIBS pandaegg $[LOCAL_LIBS]
  #endif
#endif


  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define INCLUDED_SOURCES \
    config_framework.cxx config_framework.h \
    pandaFramework.cxx pandaFramework.I pandaFramework.h \
    windowFramework.cxx windowFramework.I windowFramework.h \
    rock_floor.rgb_src.c shuttle_controls.bam_src.c

  #define INSTALL_HEADERS \
    config_framework.h \
    pandaFramework.I pandaFramework.h \
    windowFramework.I windowFramework.h    

#end lib_target
