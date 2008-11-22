#define BUILD_DIRECTORY $[HAVE_MAYA]

#begin bin_target
  #define TARGET maya2egg
  #define OTHER_LIBS \
    dtoolbase:c dtoolutil:c dtool:m
  #define SOURCES \
    mayapath.cxx
#end bin_target

#begin bin_target
  #define USE_PACKAGES maya
  #define TARGET maya2egg_bin
  #define LOCAL_LIBS \
    mayabase mayaegg eggbase progbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m $[if $[WINDOWS_PLATFORM],pystub,] \
    pipeline:c pnmimage:c

  // Irix requires this to be named explicitly.
  #define UNIX_SYS_LIBS \
    ExtensionLayer

  #define SOURCES \
    mayaToEgg.cxx mayaToEgg.h

#end bin_target

#begin bin_target
  #define TARGET egg2maya
  #define OTHER_LIBS \
    dtoolbase:c dtoolutil:c dtool:m
  #define SOURCES \
    mayapath.cxx
#end bin_target

#begin bin_target
  #define USE_PACKAGES maya
  #define TARGET egg2maya_bin
  #define LOCAL_LIBS \
    mayabase mayaegg eggbase progbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m $[if $[WINDOWS_PLATFORM],pystub,] \
    pipeline:c pnmimage:c

  // Irix requires this to be named explicitly.
  #define UNIX_SYS_LIBS \
    ExtensionLayer

  #define SOURCES \
    eggToMaya.cxx eggToMaya.h

#end bin_target


#begin bin_target
  #define TARGET mayacopy
  #define OTHER_LIBS \
    dtoolbase:c dtoolutil:c dtool:m
  #define SOURCES \
    mayapath.cxx
#end bin_target

#begin bin_target
  #define USE_PACKAGES maya
  #define TARGET mayacopy_bin
  #define LOCAL_LIBS cvscopy mayabase progbase

  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c panda:m \
    express:c pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m $[if $[WINDOWS_PLATFORM],pystub,] \
    putil:c pipeline:c pnmimage:c

  // Irix requires this to be named explicitly.
  #define UNIX_SYS_LIBS \
    ExtensionLayer

  #define SOURCES \
    mayaCopy.cxx mayaCopy.h

#end bin_target


#begin lib_target
  #define BUILD_TARGET $[not $[LINK_ALL_STATIC]]
  #define USE_PACKAGES maya
  #define TARGET mayapview
  #define LOCAL_LIBS mayabase mayaegg
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    framework:m \
    linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m $[if $[WINDOWS_PLATFORM],pystub,] \
    pipeline:c

  #define BUILDING_DLL BUILDING_MISC

  #if $[WINDOWS_PLATFORM]
    // On Windows, Maya expects its plugins to be named with a .mll
    // extension, but it's a perfectly normal dll otherwise.  This
    // ppremake hack achieves that filename.
    #define dlllib mll
  #endif

  #define SOURCES \
    mayaPview.cxx mayaPview.h

#end lib_target

#begin lib_target
  #define BUILD_TARGET $[not $[LINK_ALL_STATIC]]
  #define USE_PACKAGES maya
  #define TARGET mayasavepview

  #if $[WINDOWS_PLATFORM]
    #define dlllib mll
  #endif

  #define SOURCES \
    mayaSavePview.cxx mayaSavePview.h
#end lib_target

#begin lib_target
  #define BUILD_TARGET $[not $[LINK_ALL_STATIC]]
  #define USE_PACKAGES maya
  #define TARGET mayaEggImport
  #define LOCAL_LIBS mayabase mayaegg
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    framework:m \
    linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m $[if $[WINDOWS_PLATFORM],pystub,] \
    pipeline:c

  #define BUILDING_DLL BUILDING_MISC

  #if $[WINDOWS_PLATFORM]
    #define dlllib mll
  #endif

  #define SOURCES \
    mayaEggImport.cxx

#end lib_target

#begin lib_target
  #define USE_PACKAGES maya
  #define TARGET mayaloader
  #define BUILDING_DLL BUILDING_MISC
  #define LOCAL_LIBS \
    mayabase mayaegg ptloader converter pandatoolbase
  #define OTHER_LIBS \
    egg2pg:c egg:c pandaegg:m \
    mathutil:c linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolconfig dtool \
    event:c gobj:c chan:c pgraph:c parametrics:c char:c prc:c dtoolutil:c \
    interrogatedb:c dtoolbase:c collide:c pnmimage:c dgraph:c tform:c \
    pipeline:c pstatclient:c grutil:c gsgbase:c net:c lerp:c display:c \
    cull:c text:c nativenet:c movies:c audio:c \
    $[if $[HAVE_FREETYPE],pnmtext:c]

  #define SOURCES \
    config_mayaloader.cxx

#end lib_target

#begin test_bin_target
  #define USE_PACKAGES maya
  #define TARGET normal_test

  #define SOURCES \
    normal_test.cxx

#end test_bin_target
