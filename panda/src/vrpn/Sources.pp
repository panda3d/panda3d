#define DIRECTORY_IF_VRPN yes

#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET pandavrpn
  #define LOCAL_LIBS \
    dgraph

  #define SOURCES \
    config_vrpn.cxx config_vrpn.h vrpnClient.I vrpnClient.cxx \
    vrpnClient.h

  #define INSTALL_HEADERS \
    config_vrpn.h vrpnClient.I vrpnClient.h

#end lib_target
