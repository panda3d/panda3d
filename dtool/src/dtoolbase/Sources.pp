#begin lib_target
  #define TARGET dtoolbase
  
  #define SOURCES \
    cmath.I cmath.h \
    dallocator.T dallocator.h \
    dtoolbase.cxx dtoolbase.h dtoolbase_cc.h dtoolsymbols.h \
    fakestringstream.h \
    nearly_zero.h \
    stl_compares.I stl_compares.h \
    pallocator.T pallocator.h \
    pdeque.h plist.h pmap.h pset.h pvector.h

  #define INSTALL_HEADERS \
    cmath.I cmath.h \
    dallocator.T dallocator.h \
    dtoolbase.h dtoolbase_cc.h dtoolsymbols.h fakestringstream.h \
    nearly_zero.h \
    stl_compares.I stl_compares.h \
    pallocator.T pallocator.h \
    pdeque.h plist.h pmap.h pset.h pvector.h
#end lib_target
