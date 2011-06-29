// Filename: awWebCore.h
// Created by:  rurbino (12Oct09)
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
#ifndef AWWEBKEYBOARDMOUSE_H
#define AWWEBKEYBOARDMOUSE_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "luse.h"

#include "mouseAndKeyboard.h"


////////////////////////////////////////////////////////////////////
//       Class : AwMouseAndKeyboard
// Description : Thin wrappings arround WebCore.h
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAAWESOMIUM AwMouseAndKeyboard : public DataNode {
//member data data
protected:  
  // inputs adn output indices... initialized in constructor
  int _button_events_input;
  int _button_events_output;

PUBLISHED:
  AwMouseAndKeyboard(const string &name);

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MouseAndKeyboard::init_type();
    register_type(_type_handle, "AwMouseAndKeyboard",
                  DataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
