// Filename: buttonNode.h
// Created by:  drose (31Dec69)
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

#ifndef BUTTONNODE_H
#define BUTTONNODE_H

#include <pandabase.h>

#include "clientBase.h"
#include "clientButtonDevice.h"

#include <dataNode.h>
#include <nodeAttributes.h>


////////////////////////////////////////////////////////////////////
//       Class : ButtonNode
// Description : This is the primary interface to on/off button
//               devices associated with a ClientBase.  This creates a
//               node that connects to the named button device, if it
//               exists, and provides hooks to the user to read the
//               state of any of the sequentially numbered buttons
//               associated with that device.
//
//               It also can associate an arbitrary ButtonHandle with
//               each button; when buttons are associated with
//               ButtonHandles, this node will put appropriate up and
//               down events on the data graph for each button state
//               change.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ButtonNode : public DataNode {
PUBLISHED:
  ButtonNode(ClientBase *client, const string &device_name);
  virtual ~ButtonNode();

  INLINE bool is_valid() const;

  INLINE int get_num_buttons() const;

  INLINE void set_button_map(int index, ButtonHandle button);
  INLINE ButtonHandle get_button_map(int index) const;

  INLINE bool get_button_state(int index) const;
  INLINE bool is_button_known(int index) const;

public:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

////////////////////////////////////////////////////////////////////
// From parent class DataNode
////////////////////////////////////////////////////////////////////
public:
  virtual void
  transmit_data(NodeAttributes &data);

  NodeAttributes _attrib;
  PT(ButtonEventDataAttribute) _button_events;

  // outputs
  static TypeHandle _button_events_type;

private:
  PT(ClientButtonDevice) _button;

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

#include "buttonNode.I"

#endif
