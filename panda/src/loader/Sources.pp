#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET loader
  #define LOCAL_LIBS \
    event graph ipc putil express downloader
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
     bamFile.I bamFile.h config_loader.h loader.I loader.h  \
     loaderFileType.h loaderFileTypeBam.h  \
     loaderFileTypeRegistry.h modelPool.I modelPool.h  
     
  #define INCLUDED_SOURCES  \
     bamFile.cxx config_loader.cxx loader.cxx loaderFileType.cxx  \
     loaderFileTypeBam.cxx loaderFileTypeRegistry.cxx  \
     modelPool.cxx 

  #define INSTALL_HEADERS \
    bamFile.I bamFile.h loader.I loader.h loaderFileType.h \
    loaderFileTypeBam.h loaderFileTypeRegistry.h modelPool.I \
    modelPool.h

  #define IGATESCAN all

#end lib_target

