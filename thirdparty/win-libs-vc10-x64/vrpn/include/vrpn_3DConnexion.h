#ifndef VRPN_3DCONNEXION_H
#define VRPN_3DCONNEXION_H

#include "vrpn_HumanInterface.h"
#include "vrpn_Button.h"
#include "vrpn_Analog.h"

// Device drivers for the 3DConnexion SpaceNavigator and SpaceTraveler
// SpaceExplorer, SpaceMouse, Spaceball5000
// devices, connecting to them as HID devices (USB).

// Exposes two VRPN device classes: Button and Analog.
// Analogs are mapped to the six channels, each in the range (-1..1).

// This is the base driver for the devices.  The Navigator has
// only two buttons and has product ID 50726, the traveler has 8
// buttons and ID 50723.  The derived classes just construct with
// the appropriate number of buttons and an acceptor for the proper
// product ID; the baseclass does all the work.

#if defined(VRPN_USE_HID)
class VRPN_API vrpn_3DConnexion: public vrpn_Button, public vrpn_Analog, protected vrpn_HidInterface {
public:
  vrpn_3DConnexion(vrpn_HidAcceptor *filter, unsigned num_buttons,
                   const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_3DConnexion();

  virtual void mainloop();

  virtual void reconnect();

protected:
  // Set up message handlers, etc.
  void on_data_received(size_t bytes, vrpn_uint8 *buffer);

  virtual void decodePacket(size_t bytes, vrpn_uint8 *buffer);
  struct timeval _timestamp;
  vrpn_HidAcceptor *_filter;

  // Send report iff changed
  void report_changes (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
  // Send report whether or not changed
  void report (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
  // NOTE:  class_of_service is only applied to vrpn_Analog
  //  values, not vrpn_Button or vrpn_Dial
};
#else   // not _WIN32
class VRPN_API vrpn_3DConnexion: public vrpn_Button, public vrpn_Analog {
public:
  vrpn_3DConnexion(vrpn_HidAcceptor *filter, unsigned num_buttons,
                   const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_3DConnexion();

  virtual void mainloop();

protected:
  struct timeval _timestamp;
  vrpn_HidAcceptor *_filter;
  int fd;

  // Send report iff changed
  void report_changes(vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
  // Send report whether or not changed
  void report(vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
  // NOTE:  class_of_service is only applied to vrpn_Analog
  //  values, not vrpn_Button or vrpn_Dial

// There is a non-HID Linux-based driver for this device that has a capability
// not implemented in the HID interface.
#if defined(linux) && !defined(VRPN_USE_HID)
  int set_led(int led_state);
#endif
};
#endif  // not _WIN32

class VRPN_API vrpn_3DConnexion_Navigator: public vrpn_3DConnexion {
public:
  vrpn_3DConnexion_Navigator(const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_3DConnexion_Navigator() {};

protected:
};

class VRPN_API vrpn_3DConnexion_Traveler: public vrpn_3DConnexion {
public:
  vrpn_3DConnexion_Traveler(const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_3DConnexion_Traveler() {};

protected:
};

class VRPN_API vrpn_3DConnexion_SpaceMouse: public vrpn_3DConnexion {
public:
  vrpn_3DConnexion_SpaceMouse(const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_3DConnexion_SpaceMouse() {};

protected:
};

class VRPN_API vrpn_3DConnexion_SpaceExplorer: public vrpn_3DConnexion {
public:
  vrpn_3DConnexion_SpaceExplorer(const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_3DConnexion_SpaceExplorer() {};

protected:
};

class VRPN_API vrpn_3DConnexion_SpaceBall5000: public vrpn_3DConnexion {
public:
  vrpn_3DConnexion_SpaceBall5000(const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_3DConnexion_SpaceBall5000() {};

protected:
};


// end of VRPN_3DCONNEXION_H
#endif

