#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET loader
  #define LOCAL_LIBS \
    event graph ipc putil express downloader

  #define SOURCES \
    bamFile.I bamFile.cxx bamFile.h config_loader.cxx config_loader.h \
    loader.I loader.cxx loader.h loaderFileType.cxx loaderFileType.h \
    loaderFileTypeBam.cxx loaderFileTypeBam.h \
    loaderFileTypeRegistry.cxx loaderFileTypeRegistry.h modelPool.I \
    modelPool.cxx modelPool.h

  #define INSTALL_HEADERS \
    bamFile.I bamFile.h loader.I loader.h loaderFileType.h \
    loaderFileTypeBam.h loaderFileTypeRegistry.h modelPool.I \
    modelPool.h

  #define IGATESCAN all

#end lib_target

