// Filename: buttonThrower.h
// Created by:  drose (12Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef BUTTONTHROWER_H
#define BUTTONTHROWER_H

#include "pandabase.h"

#include "dataNode.h"
#include "modifierButtons.h"
#include "buttonEventList.h"
#include "pvector.h"
#include "pmap.h"
#include "eventParameter.h"

////////////////////////////////////////////////////////////////////
//       Class : ButtonThrower
// Description : Throws Panda Events for button down/up events
//               generated within the data graph.
//
//               This is a DataNode which is intended to be parented
//               to the data graph below a device which is generating
//               a sequence of button events, like a MouseAndKeyboard
//               device.  It simply takes each button it finds and
//               throws a corresponding event based on the button name
//               via the throw_event() call.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ButtonThrower : public DataNode {
PUBLISHED:
  ButtonThrower(const string &name);
  ~ButtonThrower();

  INLINE void set_prefix(const string &prefix);
  INLINE bool has_prefix() const;
  INLINE string get_prefix() const;

  INLINE void set_time_flag(bool time_flag);
  INLINE bool get_time_flag() const;

  void add_parameter(const EventParameter &obj);
  int get_num_parameters() const;
  EventParameter get_parameter(int n) const;

  INLINE const ModifierButtons &get_modifier_buttons() const;
  INLINE void set_modifier_buttons(const ModifierButtons &mods);

  INLINE void set_throw_buttons_active(bool flag);
  INLINE bool get_throw_buttons_active() const;

  bool add_throw_button(const ModifierButtons &mods, const ButtonHandle &button);
  bool remove_throw_button(const ModifierButtons &mods, const ButtonHandle &button);
  bool has_throw_button(const ModifierButtons &mods, const ButtonHandle &button) const;
  bool has_throw_button(const ButtonHandle &button) const;
  void clear_throw_buttons();

public:
  virtual void write(ostream &out, int indent_level = 0) const;

private:
  void do_throw_event(const string &event_name, double time);

private:
  string _prefix;
  ModifierButtons _mods;

  bool _time_flag;

  typedef pvector<EventParameter> ParameterList;
  ParameterList _parameters;

  typedef pvector<ModifierButtons> ThrowButtonDef;
  typedef pmap<ButtonHandle, ThrowButtonDef> ThrowButtons;
  ThrowButtons _throw_buttons;
  bool _throw_buttons_active;

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  // inputs
  int _button_events_input;

  // outputs
  int _button_events_output;
  PT(ButtonEventList) _button_events;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "ButtonThrower",
                  DataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "buttonThrower.I"

#endif
