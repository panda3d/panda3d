#define C++FLAGS -DWITHIN_PANDA

#begin lib_target
  #define BUILD_TARGET $[HAVE_PYTHON]
  #define USE_PACKAGES ssl nspr

  #define TARGET distributed
  #define LOCAL_LIBS \
    directbase dcparser
  #define OTHER_LIBS \
    downloader:c panda:m express:c pandaexpress:m \
    interrogatedb:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m
  #if $[and $[HAVE_NET],$[HAVE_NSPR]] \
    #define OTHER_LIBS net:c $[OTHER_LIBS]
  #endif

  #define SOURCES \
    config_distributed.cxx config_distributed.h \
    cConnectionRepository.cxx cConnectionRepository.I \
    cConnectionRepository.h

  #define IGATESCAN all
#end lib_target
