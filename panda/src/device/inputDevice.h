/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file inputDevice.h
 * @author rdb
 * @date 2015-12-11
 */

#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H

#include "pandabase.h"

#include "buttonEvent.h"
#include "buttonEventList.h"
#include "pointerEvent.h"
#include "pointerEventList.h"
#include "pointerData.h"
#include "trackerData.h"
#include "clockObject.h"

#include "pdeque.h"
#include "pvector.h"
#include "lightMutex.h"
#include "lightMutexHolder.h"

/**
 * This is a structure representing a single input device.  Input devices may
 * have zero or more buttons, pointers, or axes associated with them, and
 * optionally a motion tracker.
 *
 * These devices are brought under a common interface because there is such a
 * large range of devices out there that may support any number of these types
 * of axes, we couldn't even begin to cover them with type-specific
 * subclasses.
 *
 * Use the various has_() and get_num_() methods to determine information about
 * the device capabilities. For instance, has_keyboard() will give an
 * indication that you can receive keystroke events from this device, and
 * get_num_buttons() will tell you that the device may send button events.
 *
 * There is the DeviceType enumeration, however, which will (if known) contain
 * identification of the general category of devices this fits in, such as
 * keyboard, mouse, gamepad, or flight stick.
 *
 * @since 1.10.0
 */
class EXPCL_PANDA_DEVICE InputDevice : public TypedReferenceCount {
PUBLISHED:
  // This enum contains information that can be used to identify the
  // type of input device.
  enum class DeviceClass {
    // It is not known what type of device this is.
    unknown,

    // This means that the device doesn't correspond to a physical
    // device, but rather to a dynamic source of input events.
    virtual_device,

    // A physical, alphabetical keyboard.
    keyboard,

    mouse,
    touch,

    // A gamepad with action buttons, a D-pad, and thumbsticks.
    gamepad,

    flight_stick,
    steering_wheel,
    dance_pad,

    // Head-mounted display.
    hmd,

    // 3D mouse, such as produced by 3Dconnexion.
    spatial_mouse,
  };

  enum class Feature {
    // The device provides absolute screen coordinates.
    pointer,

    // The device has an interface for providing text input.
    keyboard,

    // The device has a motion tracker, such as an HMD.
    tracker,

    // The device can produce force feedback.
    vibration,

    // The device provides information about battery life.
    battery,
  };

  enum class Axis {
    none,

    // Generic translational axes
    x,
    y,
    z,

    // Generic rotational axes, used by joysticks and 3D mice
    yaw,
    pitch,
    roll,

    // Gamepad
    left_x,
    left_y,
    left_trigger,
    right_x,
    right_y,
    right_trigger,

    // Flight stick specific
    throttle,
    rudder, // When available separately from yaw

    // Steering wheel / pedals
    wheel,
    accelerator,
    brake,
  };

  enum State {
    S_unknown,
    S_up,
    S_down
  };

  class ButtonState {
  public:
    constexpr ButtonState() = default;
    INLINE ButtonState(ButtonHandle handle);
    ALWAYS_INLINE bool is_known() const;
    ALWAYS_INLINE bool is_pressed() const;

  PUBLISHED:
    operator bool() { return _state != S_unknown; }

    MAKE_PROPERTY(known, is_known);
    MAKE_PROPERTY(pressed, is_pressed);

    ButtonHandle handle = ButtonHandle::none();

  public:
    State _state = S_unknown;
  };

  class AxisState {
  public:
    constexpr AxisState() = default;

  PUBLISHED:
    operator bool() { return known && value != 0.0; }

    Axis axis = Axis::none;
    double value = 0.0;
    bool known = false;

  public:
    double _scale = 1.0;
    double _bias = 0.0;
  };

  class BatteryData {
  PUBLISHED:
    // Ranges from 0 through max_level.
    short level = -1;

    // Maximum value of 'level' field.
    short max_level = -1;
  };

protected:
  InputDevice(const std::string &name, DeviceClass dev_class);

public:
  InputDevice();
  InputDevice(const InputDevice &copy) = delete;
  InputDevice &operator = (const InputDevice &copy) = delete;
  virtual ~InputDevice();

  INLINE std::string get_name() const;
  INLINE std::string get_manufacturer() const;
  INLINE std::string get_serial_number() const;
  INLINE unsigned short get_vendor_id() const;
  INLINE unsigned short get_product_id() const;
  INLINE bool is_connected() const;
  INLINE DeviceClass get_device_class() const;

  INLINE bool has_pointer() const;
  INLINE bool has_keyboard() const;
  INLINE bool has_tracker() const;
  INLINE bool has_vibration() const;
  INLINE bool has_battery() const;

  INLINE TrackerData get_tracker() const;
  INLINE BatteryData get_battery() const;

  INLINE size_t get_num_buttons() const;
  INLINE ButtonHandle get_button_map(size_t index) const;
  INLINE bool is_button_pressed(size_t index) const;
  INLINE bool is_button_known(size_t index) const;
  INLINE ButtonState get_button(size_t index) const;

  INLINE size_t get_num_axes() const;
  INLINE double get_axis_value(size_t index) const;
  INLINE bool is_axis_known(size_t index) const;
  INLINE AxisState get_axis(size_t index) const;

PUBLISHED:
  // The human-readable name of this input device.
  MAKE_PROPERTY(name, get_name);

  // The device's manufacturer, or the empty string if not known.
  MAKE_PROPERTY(manufacturer, get_manufacturer);

  // The device's serial number, or the empty string if not known.
  MAKE_PROPERTY(serial_number, get_serial_number);

  // USB vendor ID of the device, or 0 if not known.
  MAKE_PROPERTY(vendor_id, get_vendor_id);

  // USB product ID of the device, or 0 if not known.
  MAKE_PROPERTY(product_id, get_product_id);

  // This is false if we know that the device is not currently connected.
  // May report false positives if we can't know this with certainty.
  MAKE_PROPERTY(connected, is_connected);

  // This contains an identification of the general type of device.  If
  // this could not be determined, it is set to DC_unknown.
  MAKE_PROPERTY(device_class, get_device_class);

  // Determine supported features
  INLINE bool has_feature(Feature feature) const;

  // Getters for the various types of device data.
  MAKE_PROPERTY2(tracker, has_tracker, get_tracker);
  MAKE_PROPERTY2(battery, has_battery, get_battery);

  // Make device buttons and axes iterable
  MAKE_SEQ_PROPERTY(buttons, get_num_buttons, get_button);
  MAKE_SEQ_PROPERTY(axes, get_num_axes, get_axis);

  // Associate buttons/axes with symbolic handles.
  INLINE void map_button(size_t index, ButtonHandle handle);
  INLINE void map_axis(size_t index, Axis axis);
  INLINE ButtonState find_button(ButtonHandle handle) const;
  INLINE AxisState find_axis(Axis axis) const;

  // Enable rumble force-feedback effects
  INLINE void set_vibration(double strong, double weak);

  INLINE void enable_pointer_events();
  INLINE void disable_pointer_events();

  void poll();

  bool has_button_event() const;
  PT(ButtonEventList) get_button_events();
  bool has_pointer_event() const;
  PT(PointerEventList) get_pointer_events();

  virtual void output(std::ostream &out) const;

public:
  static std::string format_device_class(DeviceClass dc);
  static std::string format_axis(Axis axis);

protected:
  INLINE void enable_feature(Feature feature);

  // Called during the constructor to add new axes or buttons
  int add_button(ButtonHandle handle);
  int add_axis(Axis axis, int minimum, int maximum, bool centered);
  int add_axis(Axis axis, int minimum, int maximum);

  int add_pointer(PointerType type, int id, bool primary = false);
  void remove_pointer(int id);
  void update_pointer(PointerData data, double time);
  void pointer_moved(int id, double x, double y, double time);
  void button_changed(int index, bool down);
  void axis_changed(int index, int value);
  void set_axis_value(int index, double state);

  void tracker_changed(const LPoint3 &pos, const LOrientation &orient, double time);

  virtual void do_set_vibration(double low, double high);
  virtual void do_poll();

public:
  INLINE void set_connected(bool connected);

  void output_buttons(std::ostream &out) const;
  void write_buttons(std::ostream &out, int indent_level) const;
  void write_axes(std::ostream &out, int indent_level) const;

protected:
  typedef pdeque<ButtonEvent> ButtonEvents;

  LightMutex _lock {"InputDevice"};

  std::string _name;
  std::string _serial_number;
  std::string _manufacturer;
  DeviceClass _device_class = DeviceClass::unknown;
  unsigned int _features = 0;
  int _event_sequence = 0;
  unsigned short _vendor_id = 0;
  unsigned short _product_id = 0;
  bool _is_connected = false;
  bool _enable_pointer_events = false;
  PT(ButtonEventList) _button_events;
  PT(PointerEventList) _pointer_events;

  size_t _num_pointers = 0;
  typedef pvector<PointerData> Pointers;
  Pointers _pointers;

PUBLISHED:
  typedef pvector<ButtonState> Buttons;
  typedef pvector<AxisState> Axes;
  Buttons _buttons;
  Axes _axes;

  PointerData _pointer_data;
  BatteryData _battery_data;
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

INLINE std::ostream &operator << (std::ostream &out, const InputDevice &device) {
  device.output(out);
  return out;
}

EXPCL_PANDA_DEVICE std::ostream &operator << (std::ostream &out, InputDevice::DeviceClass dc);
EXPCL_PANDA_DEVICE std::ostream &operator << (std::ostream &out, InputDevice::Axis axis);

#include "inputDevice.I"

#endif
