#define LOCAL_LIBS p3dtoolutil p3dtoolbase p3prc

#begin lib_target
  #define TARGET p3dconfig
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx
  
  #define SOURCES \
    config_dconfig.h \
    dconfig.I dconfig.h
    
 #define INCLUDED_SOURCES \
    config_dconfig.cxx dconfig.cxx

  #define INSTALL_HEADERS \
    config_dconfig.h \
    dconfig.I dconfig.h

#end lib_target

#begin test_bin_target
  #define TARGET test_config
  #define SOURCES test_config.cxx
  #define LOCAL_LIBS p3dconfig $[LOCAL_LIBS]
#end test_bin_target

#begin test_bin_target
  #define TARGET test_expand
  #define SOURCES test_expand.cxx
  #define LOCAL_LIBS p3dconfig $[LOCAL_LIBS]
#end test_bin_target

#begin test_bin_target
  #define TARGET test_pfstream
  #define SOURCES test_pfstream.cxx
  #define LOCAL_LIBS p3dconfig $[LOCAL_LIBS]
#end test_bin_target

#begin test_bin_target
  #define TARGET test_searchpath
  #define SOURCES test_searchpath.cxx
  #define LOCAL_LIBS p3dconfig $[LOCAL_LIBS]
#end test_bin_target
