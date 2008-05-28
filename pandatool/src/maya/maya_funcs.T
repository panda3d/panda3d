// Filename: maya_funcs.I
// Created by:  drose (16Feb00)
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


////////////////////////////////////////////////////////////////////
//     Function: get_maya_attribute
//  Description: A generic function to extract an attribute of some
//               type from an MObject.  This is used to implement
//               get_bool_attribute(), etc.
////////////////////////////////////////////////////////////////////
template<class ValueType>
bool
get_maya_attribute(MObject &node, const string &attribute_name,
                   ValueType &value) {
  bool status = false;

  MPlug plug;
  if (get_maya_plug(node, attribute_name, plug)) {
    status = plug.getValue(value);
  }

  return status;
}

////////////////////////////////////////////////////////////////////
//     Function: set_maya_attribute
//  Description: A generic function to set an attribute of some
//               type on an MObject.  This is used to implement
//               set_bool_attribute(), etc.
////////////////////////////////////////////////////////////////////
template<class ValueType>
bool
set_maya_attribute(MObject &node, const string &attribute_name,
                   ValueType &value) {
  bool status = false;

  MPlug plug;
  if (get_maya_plug(node, attribute_name, plug)) {
    status = plug.setValue(value);
  }

  return status;
}
