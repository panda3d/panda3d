// Filename: ButtonNode.h
// Created by:  drose (12Mar02)
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

#ifndef BUTTONNODE_H
#define BUTTONNODE_H

#include "pandabase.h"

#include "clientBase.h"
#include "clientButtonDevice.h"
#include "dataNode.h"
#include "buttonEventList.h"


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
class EXPCL_PANDA_DEVICE ButtonNode : public DataNode {
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

private:
  PT(ClientButtonDevice) _button;

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  // outputs
  int _button_events_output;
  PT(ButtonEventList) _button_events;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "ButtonNode",
                  DataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "buttonNode.I"

#endif
