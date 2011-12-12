#define C++FLAGS -DWITHIN_PANDA

#begin lib_target
  #define BUILD_TARGET $[HAVE_PYTHON]
  #define USE_PACKAGES openssl native_net net

  #define TARGET p3distributed
  #define LOCAL_LIBS \
    p3directbase p3dcparser
  #define OTHER_LIBS \
    p3event:c p3downloader:c panda:m p3express:c pandaexpress:m \
    p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    p3prc:c p3pstatclient:c p3pandabase:c p3linmath:c p3putil:c \
    p3pipeline:c $[if $[HAVE_NET],p3net:c] $[if $[WANT_NATIVE_NET],p3nativenet:c]

  #define SOURCES \
    config_distributed.cxx config_distributed.h \
    cConnectionRepository.cxx cConnectionRepository.I \
    cConnectionRepository.h \
    cDistributedSmoothNodeBase.cxx cDistributedSmoothNodeBase.I \
    cDistributedSmoothNodeBase.h

  #define IGATESCAN all
#end lib_target
