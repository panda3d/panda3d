#define DIRECTORY_IF_VRPN yes

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET pvrpn
  #define LOCAL_LIBS \
    device dgraph
  #define USE_VRPN yes

  #define SOURCES \
    config_vrpn.cxx config_vrpn.h vrpnClient.I vrpnClient.cxx \
    vrpnClient.h

  #define INSTALL_HEADERS \
    config_vrpn.h vrpnClient.I vrpnClient.h

  #define IGATESCAN all

#end lib_target
