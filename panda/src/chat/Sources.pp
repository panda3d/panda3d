#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET chat
  #define LOCAL_LIBS \
    putil dgraph display text event graph

  #define SOURCES \
    chatHelpers.cxx chatHelpers.h chatInput.I chatInput.cxx chatInput.h \
    config_chat.cxx config_chat.h

  #define INSTALL_HEADERS \
    chatHelpers.h chatInput.I chatInput.h

  #define IGATESCAN all

#end lib_target

