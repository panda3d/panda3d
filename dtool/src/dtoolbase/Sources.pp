#begin lib_target
  #define TARGET dtoolbase
  
  #define SOURCES \
    dallocator.T dallocator.h \
    dtoolbase.cxx dtoolbase.h dtoolbase_cc.h dtoolsymbols.h \
    fakestringstream.h \
    pallocator.T pallocator.h \
    pdeque.h plist.h pmap.h pset.h pvector.h

  #define INSTALL_HEADERS \
    dallocator.T dallocator.h \
    dtoolbase.h dtoolbase_cc.h dtoolsymbols.h fakestringstream.h \
    pallocator.T pallocator.h \
    pdeque.h plist.h pmap.h pset.h pvector.h
#end lib_target
