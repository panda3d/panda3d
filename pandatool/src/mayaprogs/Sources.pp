#define BUILD_DIRECTORY $[HAVE_MAYA]

#define WIN_SYS_LIBS shell32.lib advapi32.lib ws2_32.lib

#begin bin_target
  #define TARGET maya2egg
  #define OTHER_LIBS \
    p3dtoolbase:c p3dtoolutil:c p3dtool:m p3prc:c p3dtoolconfig:m \
    p3express:c pandaexpress:m 
  #define SOURCES \
    mayapath.cxx
#end bin_target

#begin bin_target
  #define TARGET maya2egg_server
  #define OTHER_LIBS \
    p3dtoolbase:c p3dtoolutil:c p3dtool:m p3prc:c p3dtoolconfig:m \
    p3express:c pandaexpress:m 
  #define SOURCES \
    mayapath.cxx
#end bin_target

#begin bin_target
  #define USE_PACKAGES maya
  #define TARGET maya2egg_bin
  #define LOCAL_LIBS \
    mayabase p3mayaegg p3eggbase p3progbase
  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3linmath:c p3putil:c panda:m \
    p3express:c pandaexpress:m \
    p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m $[if $[WINDOWS_PLATFORM],p3pystub,] \
    p3pipeline:c p3pnmimage:c

  // Irix requires this to be named explicitly.
  #define UNIX_SYS_LIBS \
    ExtensionLayer

  #define SOURCES \
    mayaToEgg.cxx mayaToEgg.h

#end bin_target

#begin bin_target
  #define USE_PACKAGES maya
  #define TARGET maya2egg_server_bin
  #define LOCAL_LIBS \
    mayabase p3mayaegg p3eggbase p3progbase
  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3linmath:c p3putil:c panda:m \
    p3express:c pandaexpress:m \
    p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m $[if $[WINDOWS_PLATFORM],p3pystub,] \
    p3pipeline:c p3pnmimage:c

  // Irix requires this to be named explicitly.
  #define UNIX_SYS_LIBS \
    ExtensionLayer

  #define SOURCES \
    mayaToEgg_server.cxx mayaToEgg_server.h

#end bin_target

#begin bin_target
  #define USE_PACKAGES maya
  #define TARGET maya2egg_client
  #define LOCAL_LIBS \
    mayabase p3mayaegg p3eggbase p3progbase
  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3linmath:c p3putil:c panda:m \
    p3express:c pandaexpress:m \
    p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m $[if $[WINDOWS_PLATFORM],p3pystub,] \
    p3pipeline:c p3pnmimage:c

  // Irix requires this to be named explicitly.
  #define UNIX_SYS_LIBS \
    ExtensionLayer

  #define SOURCES \
    mayaToEgg_client.cxx mayaToEgg_client.h

#end bin_target

#begin bin_target
  #define TARGET egg2maya
  #define OTHER_LIBS \
    p3dtoolbase:c p3dtoolutil:c p3dtool:m p3prc:c p3dtoolconfig:m \
    p3express:c pandaexpress:m 
  #define SOURCES \
    mayapath.cxx
#end bin_target

#begin bin_target
  #define USE_PACKAGES maya
  #define TARGET egg2maya_bin
  #define LOCAL_LIBS \
    mayabase p3mayaegg p3eggbase p3progbase
  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3linmath:c p3putil:c panda:m \
    p3express:c pandaexpress:m \
    p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m $[if $[WINDOWS_PLATFORM],p3pystub,] \
    p3pipeline:c p3pnmimage:c

  // Irix requires this to be named explicitly.
  #define UNIX_SYS_LIBS \
    ExtensionLayer

  #define SOURCES \
    eggToMaya.cxx eggToMaya.h

#end bin_target


#begin bin_target
  #define TARGET mayacopy
  #define OTHER_LIBS \
    p3dtoolbase:c p3dtoolutil:c p3dtool:m p3prc:c p3dtoolconfig:m \
    p3express:c pandaexpress:m 
  #define SOURCES \
    mayapath.cxx
#end bin_target

#begin bin_target
  #define USE_PACKAGES maya
  #define TARGET mayacopy_bin
  #define LOCAL_LIBS p3cvscopy mayabase p3progbase

  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3linmath:c panda:m \
    p3express:c pandaexpress:m \
    p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m $[if $[WINDOWS_PLATFORM],p3pystub,] \
    p3putil:c p3pipeline:c p3pnmimage:c

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
  #define LOCAL_LIBS mayabase p3mayaegg
  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3framework:m \
    p3linmath:c p3putil:c panda:m \
    p3express:c pandaexpress:m \
    p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m $[if $[WINDOWS_PLATFORM],p3pystub,] \
    p3pipeline:c

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
  #define LOCAL_LIBS mayabase p3mayaegg
  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3framework:m \
    p3linmath:c p3putil:c panda:m \
    p3express:c pandaexpress:m \
    p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m $[if $[WINDOWS_PLATFORM],p3pystub,] \
    p3pipeline:c

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
    mayabase p3mayaegg p3ptloader p3converter p3pandatoolbase
  #define OTHER_LIBS \
    p3egg2pg:c p3egg:c pandaegg:m \
    p3mathutil:c p3linmath:c p3putil:c panda:m \
    p3express:c pandaexpress:m \
    p3dtoolconfig p3dtool \
    p3event:c p3gobj:c p3chan:c p3pgraph:c p3parametrics:c p3char:c p3prc:c p3dtoolutil:c \
    p3interrogatedb:c p3dtoolbase:c p3collide:c p3pnmimage:c p3dgraph:c p3tform:c \
    p3pipeline:c p3pstatclient:c p3grutil:c p3gsgbase:c p3net:c p3display:c \
    p3cull:c p3text:c p3nativenet:c p3movies:c p3audio:c \
    $[if $[HAVE_FREETYPE],p3pnmtext:c]

  #define SOURCES \
    config_mayaloader.cxx

#end lib_target

#begin test_bin_target
  #define USE_PACKAGES maya
  #define TARGET normal_test

  #define SOURCES \
    normal_test.cxx

#end test_bin_target
