#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3gsgbase
  #define LOCAL_LIBS \
    p3putil p3linmath
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx     

  #define SOURCES \
    config_gsgbase.h \
    displayRegionBase.I displayRegionBase.h \
    graphicsOutputBase.I graphicsOutputBase.h \
    graphicsStateGuardianBase.h    

  #define INCLUDED_SOURCES \
    config_gsgbase.cxx \
    displayRegionBase.cxx \
    graphicsOutputBase.cxx \
    graphicsStateGuardianBase.cxx

  #define INSTALL_HEADERS \
    displayRegionBase.I displayRegionBase.h \
    graphicsOutputBase.I graphicsOutputBase.h \
    graphicsStateGuardianBase.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_gsgbase
  #define LOCAL_LIBS \
    p3gsgbase

  #define SOURCES \
    test_gsgbase.cxx

#end test_bin_target

