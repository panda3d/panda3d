#begin lib_target
  #define TARGET ptloader
  #define BUILDING_DLL BUILDING_PTLOADER
  #define LOCAL_LIBS \
    fltegg flt lwoegg lwo mayaegg converter pandatoolbase
  #define OTHER_LIBS \
    egg2pg:c builder:c egg:c pandaegg:m \
    mathutil:c linmath:c putil:c panda:m \
    express:c pandaexpress:m \
    dtoolconfig dtool
  #define UNIX_SYS_LIBS \
    m

  // If we've got Maya, link in the Maya libraries.
  // On second thought, don't.  It takes forever to load.
  //#define USE_PACKAGES maya

  #define SOURCES \
    config_ptloader.cxx config_ptloader.h \
    loaderFileTypePandatool.cxx loaderFileTypePandatool.h

  #define INSTALL_HEADERS \
    config_ptloader.h loaderFileTypePandatool.h

#end lib_target
