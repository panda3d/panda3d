#begin lib_target
  #define TARGET dtoolbase
  
  #define SOURCES \
    dtoolbase.cxx dtoolbase.h dtoolbase_cc.h dtoolsymbols.h \
    fakestringstream.h

  #define INSTALL_HEADERS \
    dtoolbase.h dtoolbase_cc.h dtoolsymbols.h fakestringstream.h
#end lib_target
