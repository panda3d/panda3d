#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET grutil
  #define LOCAL_LIBS \
    pgraph gobj linmath putil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    qpcardMaker.I qpcardMaker.h \
    config_grutil.h \
    qplineSegs.I qplineSegs.h
    
  #define INCLUDED_SOURCES \
    qpcardMaker.cxx \
    config_grutil.cxx \
    qplineSegs.cxx

  #define INSTALL_HEADERS \
    qpcardMaker.I qpcardMaker.h \
    qplineSegs.I qplineSegs.h

  #define IGATESCAN all

#end lib_target

