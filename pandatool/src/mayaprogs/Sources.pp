#define DIRECTORY_IF_MAYA yes

#define binary_name maya2egg

#if $[UNIX_PLATFORM]
  // On Unix, we need maya2egg to be a script that sets the
  // LD_LIBRARY_PATH variable and then invokes the application.  On
  // Windows, this path seems to get built into the executable so
  // there's no need.  (Don't know why they didn't decide to compile
  // it in also on Unix.)

#set binary_name maya2egg_bin

#begin sed_bin_target
  #define TARGET maya2egg

  #define SOURCE maya2egg_script
  #define COMMAND s:xxx:$[MAYA_LOCATION]:g

#end sed_bin_target

#endif   // $[UNIX_PLATFORM]

#begin bin_target
  #define USE_MAYA yes
  #define TARGET $[binary_name]
  #define LOCAL_LIBS \
    mayaegg eggbase progbase
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub

  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    mayaToEgg.cxx mayaToEgg.h

#end bin_target

