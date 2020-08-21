/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrmlNode.cxx
 * @author drose
 * @date 1999-06-23
 */

#include "vrmlNode.h"
#include "vrmlParser.h"

#include "indent.h"
#include "pnotify.h"

VrmlNode::
VrmlNode(const VrmlNodeType *type) {
  _type = type;
  _use_count = 0;
}

VrmlNode::
~VrmlNode() {
}


const VrmlFieldValue &VrmlNode::
get_value(const char *field_name) const {
  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    if (strcmp((*fi)._type->name, field_name) == 0) {
      return ((*fi)._value);
    }
  }

  // That field was not defined.  Get the default value.
  const VrmlNodeType::NameTypeRec *field = _type->hasField(field_name);
  if (field != nullptr) {
    return field->dflt;
  }

  std::cerr << "No such field defined for type " << _type->getName() << ": "
       << field_name << "\n";
  exit(1);
  // Just to make the compiler happy.
  static VrmlFieldValue zero;
  return zero;
}

void VrmlNode::
output(std::ostream &out, int indent_level) const {
  out << _type->getName() << " {\n";
  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    indent(out, indent_level + 2) << (*fi)._type->name << " ";
    output_value(out, (*fi)._value, (*fi)._type->type, indent_level + 2) << "\n";
  }
  indent(out, indent_level) << "}";
}


void Declaration::
output(std::ostream &out, int indent) const {
  VrmlFieldValue v;
  v._sfnode = _node;
  output_value(out, v, SFNODE, indent);
}

std::ostream &operator << (std::ostream &out, const VrmlScene &scene) {
  VrmlScene::const_iterator si;
  for (si = scene.begin(); si != scene.end(); ++si) {
    out << (*si) << "\n";
  }

  return out;
}
