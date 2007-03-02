#define C++FLAGS -DWITHIN_PANDA

#begin lib_target
  #define BUILD_TARGET $[HAVE_PYTHON]
  #define USE_PACKAGES openssl nspr native_net

  #define TARGET distributed
  #define LOCAL_LIBS \
    directbase dcparser
  #define OTHER_LIBS \
    event:c downloader:c panda:m express:c pandaexpress:m \
    interrogatedb:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m \
    prc:c pstatclient:c pandabase:c linmath:c putil:c \
    pipeline:c $[if $[HAVE_NET],net:c] $[if $[WANT_NATIVE_NET],nativenet:c]

  #define SOURCES \
    config_distributed.cxx config_distributed.h \
    cConnectionRepository.cxx cConnectionRepository.I \
    cConnectionRepository.h \
    cDistributedSmoothNodeBase.cxx cDistributedSmoothNodeBase.I \
    cDistributedSmoothNodeBase.h

  #define IGATESCAN all
#end lib_target
