#define INSTALL_CONFIG \
  dtool.pth

#begin lib_target
  #define TARGET dtoolbase
  
  #define SOURCES \
    dtoolbase.cxx dtoolbase.h dtoolbase_cc.h dtoolsymbols.h \
    fakestringstream.h \
    pallocator.T pallocator.h \
    pdeque.h plist.h pmap.h pset.h pvector.h

  #define INSTALL_HEADERS \
    dtoolbase.h dtoolbase_cc.h dtoolsymbols.h fakestringstream.h \
    pallocator.T pallocator.h \
    pdeque.h plist.h pmap.h pset.h pvector.h
#end lib_target
