#define LOCAL_LIBS dtoolutil dtoolbase prc

#begin lib_target
  #define TARGET dconfig
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx
  
  #define SOURCES \
    configTable.I configTable.h \
    config_dconfig.h config_setup.h \
    dconfig.I dconfig.h \
    serialization.I serialization.h  \
    symbolEnt.I  symbolEnt.h
    
 #define INCLUDED_SOURCES \
    configTable.cxx config_dconfig.cxx dconfig.cxx \
    symbolEnt.cxx 

  #define INSTALL_HEADERS                                               \
    configTable.I configTable.h config_dconfig.h config_setup.h         \
    dconfig.I dconfig.h \
    serialization.I serialization.h symbolEnt.I        \
    symbolEnt.h

#end lib_target

#begin test_bin_target
  #define TARGET test_config
  #define SOURCES test_config.cxx
  #define LOCAL_LIBS dconfig $[LOCAL_LIBS]
#end test_bin_target

#begin test_bin_target
  #define TARGET test_expand
  #define SOURCES test_expand.cxx
  #define LOCAL_LIBS dconfig $[LOCAL_LIBS]
#end test_bin_target

#begin test_bin_target
  #define TARGET test_pfstream
  #define SOURCES test_pfstream.cxx
  #define LOCAL_LIBS dconfig $[LOCAL_LIBS]
#end test_bin_target

#begin test_bin_target
  #define TARGET test_searchpath
  #define SOURCES test_searchpath.cxx
  #define LOCAL_LIBS dconfig $[LOCAL_LIBS]
#end test_bin_target

#begin test_bin_target
  #define TARGET test_serialization
  #define SOURCES test_serialization.cxx
  #define LOCAL_LIBS dconfig $[LOCAL_LIBS]
#end test_bin_target
