#ifndef VRPN_DREAMCHEEKY_H
#define VRPN_DREAMCHEEKY_H

#include "vrpn_HumanInterface.h"
#include "vrpn_Button.h"

// Device drivers for the Dream Cheeky USB Roll-Up Drum Kit; done in such a
// way that any other USB devices from this vendow should be easy to add.
// Based on the X-Keys driver.
//
// For the X-Keys Joystick Pro:
// Button 0 is the upper-left triangle
// Button 1 is hte upper-right triangle
// Button 2 is the upper center round pad
// Button 3 is the lower-right round pad
// Button 4 is the lower-center round pad
// Button 5 is the lower-left round pad

#if defined(VRPN_USE_HID)

class vrpn_DreamCheeky: public vrpn_BaseClass, protected vrpn_HidInterface {
public:
  vrpn_DreamCheeky(vrpn_HidAcceptor *filter, const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_DreamCheeky();

  virtual void mainloop() = 0;

  virtual void reconnect();

protected:
  // Set up message handlers, etc.
  void on_data_received(size_t bytes, vrpn_uint8 *buffer);

  virtual void decodePacket(size_t bytes, vrpn_uint8 *buffer) = 0;	
  struct timeval _timestamp;
  vrpn_HidAcceptor *_filter;

  // No actual types to register, derived classes will be buttons, analogs, and/or dials
  int register_types(void) { return 0; }
};

class vrpn_DreamCheeky_Drum_Kit: protected vrpn_DreamCheeky, public vrpn_Button {
public:
  // The sensors "bounce" a lot when the buttons are pressed and released,
  // causing spurious readings of press/release.  Debouncing looks at ensembles
  // of events to make sure that the buttons have settled before reporting
  // events.
  vrpn_DreamCheeky_Drum_Kit(const char *name, vrpn_Connection *c = 0, bool debounce = true);
  virtual ~vrpn_DreamCheeky_Drum_Kit() {};

  virtual void mainloop();

protected:
  // Do we try to debounce the buttons?
  bool d_debounce;

  // Send report iff changed
  void report_changes (void);
  // Send report whether or not changed
  void report (void);

  void decodePacket(size_t bytes, vrpn_uint8 *buffer);
};

// End of Windows/Cygwin/Apple
#endif

// end of VRPN_DREAMCHEEKY_H
#endif

