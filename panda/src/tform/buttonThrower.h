// Filename: buttonThrower.h
// Created by:  drose (09Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef BUTTONTHROWER_H
#define BUTTONTHROWER_H

#include "pandabase.h"

#include "dataNode.h"
#include "modifierButtons.h"
#include "pvector.h"
#include "pmap.h"

////////////////////////////////////////////////////////////////////
//       Class : ButtonThrower
// Description : Throws Panda Events for button down/up events
//               generated within the data graph.
//
//               This is a DataNode which is intended to be parented
//               to the data graph below a device which is generating
//               a sequence of button events as stored in an
//               ButtonEventDataTransitions data member.  It simply
//               takes each button it finds and throws a corresponding
//               event based on the button name via the throw_event()
//               call.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ButtonThrower : public DataNode {
PUBLISHED:
  ButtonThrower(const string &name = "");
  ~ButtonThrower();

  void set_prefix(const string &prefix);
  bool has_prefix() const;
  string get_prefix() const;

  const ModifierButtons &get_modifier_buttons() const;
  void set_modifier_buttons(const ModifierButtons &mods);

  void set_throw_buttons_active(bool flag);
  bool get_throw_buttons_active() const;

  bool add_throw_button(const ModifierButtons &mods, const ButtonHandle &button);
  bool remove_throw_button(const ModifierButtons &mods, const ButtonHandle &button);
  bool has_throw_button(const ModifierButtons &mods, const ButtonHandle &button) const;
  bool has_throw_button(const ButtonHandle &button) const;
  void clear_throw_buttons();

public:
  virtual void write(ostream &out, int indent_level = 0) const;

private:
  string _prefix;
  ModifierButtons _mods;

  typedef pvector<ModifierButtons> ThrowButtonDef;
  typedef pmap<ButtonHandle, ThrowButtonDef> ThrowButtons;
  ThrowButtons _throw_buttons;
  bool _throw_buttons_active;

////////////////////////////////////////////////////////////////////
// From parent class DataNode
////////////////////////////////////////////////////////////////////
public:
  virtual void
  transmit_data(AllTransitionsWrapper &data);

  // inputs
  static TypeHandle _button_events_type;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#endif
