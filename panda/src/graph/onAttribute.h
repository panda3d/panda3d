// Filename: onAttribute.h
// Created by:  drose (22Mar00)
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

#ifndef ONATTRIBUTE_H
#define ONATTRIBUTE_H

#include <pandabase.h>

#include "nodeAttribute.h"

class OnTransition;

////////////////////////////////////////////////////////////////////
//       Class : OnAttribute
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA OnAttribute : public NodeAttribute {
protected:
  INLINE_GRAPH OnAttribute() {};
  INLINE_GRAPH OnAttribute(const OnAttribute &copy) : NodeAttribute(copy) {};
  INLINE_GRAPH void operator = (const OnAttribute &copy) {NodeAttribute::operator = (copy);};

public:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeAttribute *other) const;

  virtual void set_value_from(const OnTransition *other)=0;
  virtual int compare_values(const OnAttribute *other) const=0;
  virtual void output_value(ostream &out) const=0;
  virtual void write_value(ostream &out, int indent_level) const=0;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NodeAttribute::init_type();
    register_type(_type_handle, "OnAttribute",
                  NodeAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class OnTransition;
};

#endif
