// Filename: graphicsWindowInputDevice.h
// Created by:  drose (24May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GRAPHICSWINDOWINPUTDEVICE_H
#define GRAPHICSWINDOWINPUTDEVICE_H

#include <pandabase.h>

#include <buttonEvent.h>
#include <mouseData.h>
#include <modifierButtons.h>

#include <deque>
#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : GraphicsWindowInputDevice
// Description : This is a structure representing a single input
//               device that may be associated with a window.
//               Typically this will be a keyboard/mouse pair, and
//               there will be exactly one of these associated with
//               each window, but other variants are possible.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsWindowInputDevice {
private:
  GraphicsWindowInputDevice(const string &name, int flags);

public:
  static GraphicsWindowInputDevice pointer_only(const string &name);
  static GraphicsWindowInputDevice keyboard_only(const string &name);
  static GraphicsWindowInputDevice pointer_and_keyboard(const string &name);

  GraphicsWindowInputDevice(const GraphicsWindowInputDevice &copy);
  void operator = (const GraphicsWindowInputDevice &copy);
  ~GraphicsWindowInputDevice();

  INLINE string get_name() const;
  INLINE bool has_pointer() const;
  INLINE bool has_keyboard() const;

  INLINE const MouseData &get_mouse_data() const;
  INLINE const ModifierButtons &get_modifier_buttons() const;
  INLINE void set_modifier_buttons(const ModifierButtons &mods);

  bool has_button_event() const;
  ButtonEvent get_button_event();

public:
  // The following interface is for the various kinds of
  // GraphicsWindows to record the data incoming on the device.
  void button_down(ButtonHandle button);
  void button_up(ButtonHandle button);
  INLINE void set_pointer_in_window(int x, int y);
  INLINE void set_pointer_out_of_window();

public:
  // We need these methods to make VC++ happy when we try to
  // instantiate a vector<GraphicsWindowInputDevice>.  They don't do
  // anything useful.
  INLINE bool operator == (const GraphicsWindowInputDevice &other) const;
  INLINE bool operator != (const GraphicsWindowInputDevice &other) const;
  INLINE bool operator < (const GraphicsWindowInputDevice &other) const;

private:
  enum InputDeviceFlags {
    IDF_has_pointer    = 0x01,
    IDF_has_keyboard   = 0x02
  };
  typedef deque<ButtonEvent> ButtonEvents;
  
  string _name;
  int _flags;
  MouseData _mouse_data;
  ModifierButtons _mods;
  ButtonEvents _button_events;
};

#include "graphicsWindowInputDevice.I"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, std::vector<GraphicsWindowInputDevice>)
typedef vector<GraphicsWindowInputDevice> vector_GraphicsWindowInputDevice;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
