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
    vrpnAnalog.I vrpnAnalog.cxx vrpnAnalog.h \
    vrpnAnalogDevice.I vrpnAnalogDevice.cxx vrpnAnalogDevice.h \
    vrpnButton.I vrpnButton.cxx vrpnButton.h \
    vrpnButtonDevice.I vrpnButtonDevice.cxx vrpnButtonDevice.h \
    vrpnClient.h \
    vrpnTracker.I vrpnTracker.cxx vrpnTracker.h \
    vrpnTrackerDevice.I vrpnTrackerDevice.cxx vrpnTrackerDevice.h \
    vrpn_interface.h

  #define INSTALL_HEADERS \
    config_vrpn.h \
    vrpnClient.I vrpnClient.h

  #define IGATESCAN all

#end lib_target
