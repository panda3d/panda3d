// Filename: numericDataTransition.h
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

#ifndef NUMERICDATATRANSITION_H
#define NUMERICDATATRANSITION_H

#include "pandabase.h"

#include "onTransition.h"
#include "indent.h"

////////////////////////////////////////////////////////////////////
//       Class : NumericDataTransition
// Description : A NumericDataTransition is a special data graph
//               attribute that is used to pass around a single
//               number.
////////////////////////////////////////////////////////////////////
template<class NumType>
class NumericDataTransition : public OnTransition {
public:
  INLINE NumericDataTransition(NumType value);
  INLINE NumericDataTransition(const NumericDataTransition<NumType> &copy);
  INLINE void operator = (const NumericDataTransition<NumType> &copy);

public:
  INLINE void set_value(NumType value);
  INLINE NumType get_value() const;

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

private:
  NumType _value;

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
    register_type(_type_handle,
                  string("NumericDataTransition<") +
                  get_type_handle(NumType).get_name() + ">",
                  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "numericDataTransition.I"

#endif
