#begin lib_target
  #define BUILD_TARGET $[HAVE_PYTHON]

  #define TARGET distributed
  #define LOCAL_LIBS \
    directbase
  #define OTHER_LIBS \
    downloader:c net:c panda:m express:c pandaexpress:m \
    interrogatedb:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m

  #define SOURCES \
    config_distributed.cxx config_distributed.h \
    cConnectionRepository.cxx cConnectionRepository.I \
    cConnectionRepository.h

  #define IGATESCAN all
#end lib_target
