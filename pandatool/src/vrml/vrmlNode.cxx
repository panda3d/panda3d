// Filename: vrmlNode.cxx
// Created by:  drose (23Jun99)
// 
////////////////////////////////////////////////////////////////////
// Copyright (C) 1992,93,94,95,96,97  Walt Disney Imagineering, Inc.
// 
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////

#include "vrmlNode.h"
#include "vrmlParser.h"

#include "indent.h"
#include "notify.h"

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
  if (field != NULL) {
    return field->dflt;
  }

  cerr << "No such field defined for type " << _type->getName() << ": "
       << field_name << "\n";
  exit(1);
  // Just to make the compiler happy.
  static VrmlFieldValue zero;
  return zero;
}

void VrmlNode::
output(ostream &out, int indent_level) const {
  out << _type->getName() << " {\n";
  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    indent(out, indent_level + 2) << (*fi)._type->name << " ";
    output_value(out, (*fi)._value, (*fi)._type->type, indent_level + 2) << "\n";
  }
  indent(out, indent_level) << "}";
}


void Declaration::
output(ostream &out, int indent) const {
  VrmlFieldValue v;
  v._sfnode = _node;
  output_value(out, v, SFNODE, indent);
}

ostream &operator << (ostream &out, const VrmlScene &scene) {
  VrmlScene::const_iterator si;
  for (si = scene.begin(); si != scene.end(); ++si) {
    out << (*si) << "\n";
  }

  return out;
}
