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

#include <pandabase.h>

#include <dataNode.h>
#include <modifierButtons.h>

////////////////////////////////////////////////////////////////////
//       Class : ButtonThrower
// Description : Throws Panda Events for button down/up events
//               generated within the data graph.
//
//               This is a DataNode which is intended to be parented
//               to the data graph below a device which is generating
//               a sequence of button events as stored in an
//               ButtonEventDataAttributes data member.  It simply
//               takes each button it finds and throws a corresponding
//               event based on the button name via the throw_event()
//               call.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ButtonThrower : public DataNode {
PUBLISHED:
  ButtonThrower(const string &name = "");

  void set_prefix(const string &prefix);
  bool has_prefix() const;
  string get_prefix() const;

  const ModifierButtons &get_modifier_buttons() const;
  void set_modifier_buttons(const ModifierButtons &mods);

protected:
  string _prefix;
  ModifierButtons _mods;

////////////////////////////////////////////////////////////////////
// From parent class DataNode
////////////////////////////////////////////////////////////////////
public:

  virtual void
  transmit_data(NodeAttributes &data);

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
