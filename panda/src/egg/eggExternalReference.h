// Filename: eggExternalReference.h
// Created by:  drose (11Feb99)
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

#ifndef EGGEXTERNALREFERENCE_H
#define EGGEXTERNALREFERENCE_H

#include "pandabase.h"

#include "eggFilenameNode.h"

////////////////////////////////////////////////////////////////////
//       Class : EggExternalReference
// Description : Defines a reference to another egg file which should
//               be inserted at this point.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggExternalReference : public EggFilenameNode {
PUBLISHED:
  EggExternalReference(const string &node_name, const string &filename);
  EggExternalReference(const EggExternalReference &copy);
  EggExternalReference &operator = (const EggExternalReference &copy);

  virtual void write(ostream &out, int indent_level) const;

  virtual string get_default_extension() const;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggFilenameNode::init_type();
    register_type(_type_handle, "EggExternalReference",
                  EggFilenameNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggExternalReference.I"

#endif
