#define LOCAL_LIBS dtoolutil dtoolbase

#begin lib_target
  #define TARGET dconfig
  
  #define SOURCES \
    configTable.I configTable.cxx configTable.h config_dconfig.cxx        \
    config_dconfig.h config_notify.cxx config_notify.h config_setup.h    \
    dconfig.I dconfig.cxx dconfig.h expand.I expand.h notify.I           \
    notify.cxx notify.h notifyCategory.I notifyCategory.cxx               \
    notifyCategory.h notifyCategoryProxy.I notifyCategoryProxy.h        \
    notifySeverity.cxx notifySeverity.h serialization.I serialization.h  \
    symbolEnt.I symbolEnt.cxx symbolEnt.h

  #define INSTALL_HEADERS						\
    configTable.I configTable.h config_dconfig.h config_setup.h		\
    dconfig.I dconfig.h							\
    expand.I expand.h notify.I notify.h notifyCategory.I		\
    notifyCategory.h notifyCategoryProxy.I notifyCategoryProxy.h	\
    notifySeverity.h serialization.I serialization.h symbolEnt.I	\
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
