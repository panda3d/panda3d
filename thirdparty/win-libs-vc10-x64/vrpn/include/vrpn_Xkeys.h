#ifndef VRPN_XKEYS_H
#define VRPN_XKEYS_H

#include "vrpn_HumanInterface.h"
#include "vrpn_Button.h"
#include "vrpn_Dial.h"
#include "vrpn_Analog.h"

#if defined(VRPN_USE_HID)

// Device drivers for the X-Keys USB line of products from P.I. Engineering
// Currently supported: X-Keys Desktop, X-Keys Jog & Shuttle Pro.
// Theoretically working but untested: X-Keys Professional, X-Keys Joystick Pro.
//
// Exposes three major VRPN device classes: Button, Analog, Dial (as appropriate).
// All models expose Buttons for the keys on the device.
// Button 0 is the programming switch; it is set if the switch is in the "red" position.
//
// For the X-Keys Jog & Shuttle:
// Analog channel 0 is the shuttle position (-1 to 1). There are 15 levels.
// Analog channel 1 is the dial orientation (0 to 255).
// Dial channel 0 sends deltas on the dial.
//
// For the X-Keys Joystick Pro:
// Analog channel 0 is the joystick X axis (-1 to 1).
// Analog channel 1 is the joystick Y axis (-1 to 1).
// Analog channel 2 is the joystick RZ axis (-1 to 1).

class vrpn_Xkeys: public vrpn_BaseClass, protected vrpn_HidInterface {
public:
  vrpn_Xkeys(vrpn_HidAcceptor *filter, const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_Xkeys();

  virtual void mainloop() = 0;

  virtual void reconnect();

protected:
  // Set up message handlers, etc.
  void init_hid();
  void on_data_received(size_t bytes, vrpn_uint8 *buffer);

  static int VRPN_CALLBACK on_connect(void *thisPtr, vrpn_HANDLERPARAM p);
  static int VRPN_CALLBACK on_last_disconnect(void *thisPtr, vrpn_HANDLERPARAM p);

  virtual void decodePacket(size_t bytes, vrpn_uint8 *buffer) = 0;	
  struct timeval _timestamp;
  vrpn_HidAcceptor *_filter;

  // No actual types to register, derived classes will be buttons, analogs, and/or dials
  int register_types(void) { return 0; }
};

class vrpn_Xkeys_Desktop: protected vrpn_Xkeys, public vrpn_Button {
public:
  vrpn_Xkeys_Desktop(const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_Xkeys_Desktop() {};

  virtual void mainloop();

protected:
  // Send report iff changed
  void report_changes (void);
  // Send report whether or not changed
  void report (void);

  void decodePacket(size_t bytes, vrpn_uint8 *buffer);
};

class vrpn_Xkeys_Pro: protected vrpn_Xkeys, public vrpn_Button {
public:
  vrpn_Xkeys_Pro(const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_Xkeys_Pro() {};

  virtual void mainloop();

protected:
  // Send report iff changed
  void report_changes (void);
  // Send report whether or not changed
  void report (void);

  void decodePacket(size_t bytes, vrpn_uint8 *buffer);
};

class vrpn_Xkeys_Joystick: protected vrpn_Xkeys, public vrpn_Analog, public vrpn_Button {
public:
  vrpn_Xkeys_Joystick(const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_Xkeys_Joystick() {};

  virtual void mainloop();

protected:
  // Send report iff changed
  void report_changes (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
  // Send report whether or not changed
  void report (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
  // NOTE:  class_of_service is only applied to vrpn_Analog
  //  values, not vrpn_Button or vrpn_Dial

  void decodePacket(size_t bytes, vrpn_uint8 *buffer);
};

class vrpn_Xkeys_Jog_And_Shuttle: protected vrpn_Xkeys, public vrpn_Analog, public vrpn_Button, public vrpn_Dial {
public:
  vrpn_Xkeys_Jog_And_Shuttle(const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_Xkeys_Jog_And_Shuttle() {};

  virtual void mainloop();

protected:
  // Send report iff changed
  void report_changes (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
  // Send report whether or not changed
  void report (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
  // NOTE:  class_of_service is only applied to vrpn_Analog
  //  values, not vrpn_Button or vrpn_Dial

  void decodePacket(size_t bytes, vrpn_uint8 *buffer);

  // Previous dial value, used to determine delta to send when it changes.
  vrpn_uint8 _lastDial;
};

// end of _WIN32
#endif

// end of VRPN_XKEYS_H
#endif

