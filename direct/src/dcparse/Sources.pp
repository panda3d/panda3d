#define LOCAL_LIBS \
  dcparser
#define OTHER_LIBS \
  express:c \
  interrogatedb:c dconfig:c dtoolconfig:m \
  dtoolutil:c dtoolbase:c dtool:m
#define C++FLAGS -DWITHIN_PANDA

#begin bin_target
  #define TARGET dcparse
  #define LOCAL_LIBS dcparser
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    dcparse.cxx
#end bin_target

