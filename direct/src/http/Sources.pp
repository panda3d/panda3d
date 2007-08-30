#define BUILD_DIRECTORY $[WANT_NATIVE_NET]

#define OTHER_LIBS \
    express:c pandaexpress:m \
    pstatclient:c pipeline:c panda:m \
    interrogatedb:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m prc:c pandabase:c \
    $[if $[HAVE_NET],net:c] $[if $[WANT_NATIVE_NET],nativenet:c] \
    linmath:c putil:c

#define LOCAL_LIBS \
    directbase
#define C++FLAGS -DWITHIN_PANDA
#define UNIX_SYS_LIBS m
#define USE_PACKAGES python

#begin lib_target
  #define TARGET  http

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx  


  #define SOURCES \
     http_connection.h  \
     http_request.h


  #define INCLUDED_SOURCES \
     http_connection.cxx	\
     parsedhttprequest.cxx  \
     http_request.cxx


  #define IGATESCAN all
#end lib_target
