// Filename: inputDevice.h
// Created by:  drose (24May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H

#include "pandabase.h"

#include "buttonEvent.h"
#include "buttonEventList.h"
#include "pointerEvent.h"
#include "pointerEventList.h"
#include "mouseData.h"
#include "trackerData.h"
#include "clockObject.h"

#include "pdeque.h"
#include "pvector.h"
#include "lightMutex.h"
#include "lightMutexHolder.h"

typedef MouseData PointerData;

////////////////////////////////////////////////////////////////////
//       Class : InputDevice
// Description : This is a structure representing a single input
//               device.  Input devices may have zero or more
//               buttons, pointers, or controls associated with them,
//               and optionally a tracker.
//
//               These devices are brought under a common interface
//               because there is such a large range of devices out
//               there that may support any number of these types of
//               controls, we couldn't even begin to cover them with
//               type-specific subclasses.
//
//               Use the various has_() and get_num_() methods to
//               determine information about the device capabilities.
//               For instance, has_keyboard() will give an indication
//               that you can receive keystroke events from this
//               device, and get_num_buttons() will tell you that
//               the device may send button events.
//
//               There is the DeviceType enumeration, however, which
//               will (if known) contain identification about the
//               general category of devices this fits in, such as
//               keyboard, mouse, gamepad, or flight stick.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DEVICE InputDevice : public TypedReferenceCount {
PUBLISHED:
  // This enum contains information that can be used to identify the
  // type of input device.
  enum DeviceClass {
    // It is not known what type of device this is.
    DC_unknown,

    // This means that the device doesn't correspond to a physical
    // device, but rather to a dynamic source of input events.
    DC_virtual,

    // A physical, alphabetical keyboard.
    DC_keyboard,

    DC_mouse,
    DC_touch,

    // A gamepad with action buttons, a D-pad, and thumbsticks.
    DC_gamepad,

    DC_flight_stick,
    DC_steering_wheel,
  };

protected:
  InputDevice(const string &name, DeviceClass dev_class, int flags);

public:
  InputDevice();
  InputDevice(const InputDevice &copy);
  void operator = (const InputDevice &copy);
  ~InputDevice();

PUBLISHED:
  enum ControlAxis {
    C_none,

    // Gamepad
    C_left_x,
    C_left_y,
    C_left_trigger,
    C_right_x,
    C_right_y,
    C_right_trigger,

    // Flight stick
    C_x,
    C_y,
    C_trigger,
    C_throttle,
  };

  INLINE string get_name() const;
  INLINE bool is_connected() const;
  INLINE DeviceClass get_device_class() const;

  // The human-readable name of this input device.
  MAKE_PROPERTY(name, get_name);

  // This is false if we know that the device is not currently connected.
  // May report false positives if we can't know this with certainty.
  MAKE_PROPERTY(connected, is_connected);

  // This contains an identification of the general type of device.  If
  // this could not be determined, it is set to DC_unknown.
  MAKE_PROPERTY(device_class, get_device_class);

  INLINE bool has_pointer() const;
  INLINE bool has_keyboard() const;
  INLINE bool has_tracker() const;
  INLINE bool has_battery() const;

  INLINE PointerData get_pointer() const;
  INLINE TrackerData get_tracker() const;

  INLINE short get_battery_level() const;
  INLINE short get_max_battery_level() const;

  INLINE int get_num_buttons() const;
  INLINE void set_button_map(int index, ButtonHandle button);
  INLINE ButtonHandle get_button_map(int index) const;
  INLINE bool get_button_state(int index) const;
  INLINE bool is_button_known(int index) const;

  INLINE int get_num_controls() const;
  INLINE void set_control_map(int index, ControlAxis axis);
  INLINE ControlAxis get_control_map(int index) const;
  INLINE double get_control_state(int index) const;
  INLINE bool is_control_known(int index) const;

  INLINE void enable_pointer_events();
  INLINE void disable_pointer_events();

  void poll();

  bool has_button_event() const;
  PT(ButtonEventList) get_button_events();
  bool has_pointer_event() const;
  PT(PointerEventList) get_pointer_events();

  virtual void output(ostream &out) const;

protected:
  void set_pointer(bool inwin, double x, double y, double time);
  void set_pointer_out_of_window(double time);
  void set_button_state(int index, bool down);
  void set_control_state(int index, double state);
  void set_tracker(const LPoint3 &pos, const LOrientation &orient, double time);

  virtual void do_poll();

public:
  INLINE void set_connected(bool connected);

  // We need these methods to make VC++ happy when we try to
  // instantiate a pvector<InputDevice>.  They don't do
  // anything useful.
  INLINE bool operator == (const InputDevice &other) const;
  INLINE bool operator != (const InputDevice &other) const;
  INLINE bool operator < (const InputDevice &other) const;

  void output_buttons(ostream &out) const;
  void write_buttons(ostream &out, int indent_level) const;
  void write_controls(ostream &out, int indent_level) const;

protected:
  enum InputDeviceFlags {
    // The device provides absolute screen coordinates.
    IDF_has_pointer    = 0x01,

    // The device has an interface for providing text input.
    IDF_has_keyboard   = 0x02,

    // The device has a motion tracker, such as an HMD.
    IDF_has_tracker    = 0x04,

    // The device can produce force feedback.
    IDF_has_vibration  = 0x08,

    // The device provides information about battery life.
    IDF_has_battery    = 0x10,
  };

protected:
  typedef pdeque<ButtonEvent> ButtonEvents;

  LightMutex _lock;

  string _name;
  string _serial_number;
  string _manufacturer;
  DeviceClass _device_class;
  int _flags;
  int _event_sequence;
  short _vendor_id;
  short _product_id;
  bool _is_connected;
  bool _enable_pointer_events;
  PointerData _pointer_data;
  PT(ButtonEventList) _button_events;
  PT(PointerEventList) _pointer_events;

public:
  enum State {
    S_unknown,
    S_up,
    S_down
  };

  class ButtonState {
  public:
    INLINE ButtonState();

    ButtonHandle _handle;
    State _state;
  };
  typedef pvector<ButtonState> Buttons;
  Buttons _buttons;

  class AnalogState {
  public:
    INLINE AnalogState();

    ControlAxis _axis;
    double _state;
    bool _known;
  };
  typedef pvector<AnalogState> Controls;
  Controls _controls;

  short _battery_level;
  short _max_battery_level;

  TrackerData _tracker_data;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "InputDevice",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const InputDevice &device) {
  device.output(out);
  return out;
}

ostream &operator << (ostream &out, InputDevice::DeviceClass dc);
ostream &operator << (ostream &out, InputDevice::ControlAxis axis);

#include "inputDevice.I"

#endif
