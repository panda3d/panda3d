#define BUILD_DIRECTORY $[WANT_NATIVE_NET]
#define USE_PACKAGES native_net

#define OTHER_LIBS \
    p3express:c pandaexpress:m \
    p3pstatclient:c p3pipeline:c panda:m \
    p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c p3pandabase:c \
    $[if $[HAVE_NET],p3net:c] $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    p3linmath:c p3putil:c

#define LOCAL_LIBS \
    p3directbase
#define C++FLAGS -DWITHIN_PANDA
#define UNIX_SYS_LIBS m
#define USE_PACKAGES python

#begin lib_target
  #define TARGET  p3http

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx  


  #define SOURCES \
     config_http.h \
     http_connection.h  \
     http_request.h


  #define INCLUDED_SOURCES \
     config_http.cxx \
     http_connection.cxx \
     parsedhttprequest.cxx  \
     http_request.cxx


  #define IGATESCAN all
#end lib_target
