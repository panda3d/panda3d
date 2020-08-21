/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file maya_funcs.T
 * @author drose
 * @date 2000-02-16
 */

/**
 * A generic function to extract an attribute of some type from an MObject.
 * This is used to implement get_bool_attribute(), etc.
 */
template<class ValueType>
bool
get_maya_attribute(MObject &node, const std::string &attribute_name,
                   ValueType &value) {
  bool status = false;

  MPlug plug;
  if (get_maya_plug(node, attribute_name, plug)) {
    status = plug.getValue(value);
  }

  return status;
}

/**
 * A generic function to set an attribute of some type on an MObject.  This is
 * used to implement set_bool_attribute(), etc.
 */
template<class ValueType>
bool
set_maya_attribute(MObject &node, const std::string &attribute_name,
                   ValueType &value) {
  bool status = false;

  MPlug plug;
  if (get_maya_plug(node, attribute_name, plug)) {
    status = plug.setValue(value);
  }

  return status;
}
