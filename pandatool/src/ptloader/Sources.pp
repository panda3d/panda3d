#begin lib_target
  #define TARGET ptloader
  #define BUILDING_DLL BUILDING_PTLOADER
  #define LOCAL_LIBS xfile fltegg flt lwoegg lwo converter pandatoolbase
  #define OTHER_LIBS \
    egg2sg:c builder:c egg:c pandaegg:m \
    mathutil:c linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolconfig dtool
  #define UNIX_SYS_LIBS \
    m

  #if $[HAVE_DX]
    // To link in xfile, we need to also link in the DX libraries.
    #define WIN_SYS_LIBS d3dxof.lib
    #define USE_DX yes
  #endif

  #define SOURCES \
    config_ptloader.cxx config_ptloader.h \
    loaderFileTypePandatool.cxx loaderFileTypePandatool.h

  #define INSTALL_HEADERS \
    config_ptloader.h loaderFileTypePandatool.h

#end lib_target
