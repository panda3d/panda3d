// Filename: vectorDataTransition.h
// Created by:  drose (25Jan99)
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

#ifndef VECTORDATATRANSITION_H
#define VECTORDATATRANSITION_H

#include "pandabase.h"

#include "onTransition.h"
#include "luse.h"
#include "indent.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : VectorDataTransition
// Description : A VectorDataTransition is a special data graph
//               attribute that is used to pass around a complex
//               number like a vector or a matrix.  Matrices do not
//               accumulate; each matrix replaces the one before.
////////////////////////////////////////////////////////////////////
template<class VecType>
class VectorDataTransition : public OnTransition {
public:
  INLINE VectorDataTransition(const VecType &value);
  INLINE VectorDataTransition(const VectorDataTransition<VecType> &copy);
  INLINE void operator = (const VectorDataTransition<VecType> &copy);

public:
  INLINE void set_value(const VecType &value);
  INLINE const VecType &get_value() const;

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

private:
  VecType _value;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OnTransition::init_type();
    do_init_type(VecType);
    register_type(_type_handle,
                  string("VectorDataTransition<") +
                  get_type_handle(VecType).get_name() + ">",
                  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "vectorDataTransition.I"

#endif
