#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET grutil
  #define LOCAL_LIBS \
    sgraphutil sgattrib sgraph gobj linmath putil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    cardMaker.I cardMaker.h \
    qpcardMaker.I qpcardMaker.h \
    config_grutil.h \
    lineSegs.I lineSegs.h \
    qplineSegs.I qplineSegs.h
    
  #define INCLUDED_SOURCES \
    cardMaker.cxx \
    qpcardMaker.cxx \
    config_grutil.cxx \
    qplineSegs.cxx \
    lineSegs.cxx

  #define INSTALL_HEADERS \
    cardMaker.I cardMaker.h \
    qpcardMaker.I qpcardMaker.h \
    lineSegs.I lineSegs.h \
    qplineSegs.I qplineSegs.h

  #define IGATESCAN all

#end lib_target

