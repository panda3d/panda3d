#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET chat
  #define LOCAL_LIBS \
    putil dgraph display text event graph

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES \
    chatHelpers.h chatInput.I chatInput.h config_chat.h
    
  #define INCLUDED_SOURCES \
    chatHelpers.cxx chatInput.cxx config_chat.cxx

  #define INSTALL_HEADERS \
    chatHelpers.h chatInput.I chatInput.h

  #define IGATESCAN all

#end lib_target
