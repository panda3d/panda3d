#define BUILD_DIRECTORY $[HAVE_MAYA]

#define maya2egg maya2egg
#define mayacopy mayacopy

#if $[UNIX_PLATFORM]
  // On Unix, we need maya2egg to be a script that sets the
  // LD_LIBRARY_PATH variable and then invokes the application.  On
  // Windows, this path seems to get built into the executable so
  // there's no need.  (Don't know why they didn't decide to compile
  // it in also on Unix.)

#set maya2egg maya2egg_bin
#set mayacopy mayacopy_bin

#begin sed_bin_target
  #define TARGET maya2egg

  #define SOURCE mayapath_script
  #define COMMAND s:xxx:$[MAYA_LOCATION]:g;s:yyy:$[TARGET]:g;s+zzz+$[MAYA_LICENSE_FILE]+g;

#end sed_bin_target

#begin sed_bin_target
  #define TARGET mayacopy

  #define SOURCE mayapath_script
  #define COMMAND s:xxx:$[MAYA_LOCATION]:g;s:yyy:$[TARGET]:g;s+zzz+$[MAYA_LICENSE_FILE]+g;

#end sed_bin_target

#endif   // $[UNIX_PLATFORM]

#begin bin_target
  #define USE_PACKAGES maya
  #define TARGET $[maya2egg]
  #define LOCAL_LIBS \
    mayaegg maya eggbase progbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

  // Irix requires this to be named explicitly.
  #define UNIX_SYS_LIBS \
    ExtensionLayer

  #define SOURCES \
    mayaToEgg.cxx mayaToEgg.h

#end bin_target


#begin bin_target
  #define USE_PACKAGES maya
  #define TARGET $[mayacopy]
  #define LOCAL_LIBS cvscopy maya progbase

  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

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
  #define LOCAL_LIBS mayaegg maya
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    framework:m \
    linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

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
  #define USE_PACKAGES maya
  #define TARGET mayaloader
  #define BUILDING_DLL BUILDING_MISC
  #define LOCAL_LIBS \
    mayaegg ptloader converter pandatoolbase
  #define OTHER_LIBS \
    egg2pg:c egg:c pandaegg:m \
    mathutil:c linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolconfig dtool

  #define SOURCES \
    config_mayaloader.cxx

#end lib_target

#begin test_bin_target
  #define USE_PACKAGES maya
  #define TARGET blend_test

  #define SOURCES \
    blend_test.cxx

#end test_bin_target
