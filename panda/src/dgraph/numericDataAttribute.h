// Filename: numericDataAttribute.h
// Created by:  drose (27Mar00)
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

#ifndef NUMERICDATAATTRIBUTE_H
#define NUMERICDATAATTRIBUTE_H

#include <pandabase.h>

#include <indent.h>
#include <nodeAttribute.h>

template<class NumType>
class NumericDataTransition;

////////////////////////////////////////////////////////////////////
//       Class : NumericDataAttribute
// Description :
////////////////////////////////////////////////////////////////////
template<class NumType>
class NumericDataAttribute : public NodeAttribute {
public:
  INLINE NumericDataAttribute();
  INLINE NumericDataAttribute(NumType value);
  INLINE NumericDataAttribute(const NumericDataAttribute<NumType> &copy);
  INLINE void operator = (const NumericDataAttribute<NumType> &copy);

public:
  INLINE void set_value(NumType value);
  INLINE NumType get_value() const;

  virtual TypeHandle get_handle() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeAttribute *other) const;

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
    NodeAttribute::init_type();
    register_type(_type_handle,
                  string("NumericDataAttribute<") +
                  get_type_handle(NumType).get_name() + ">",
                  NodeAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
friend class NumericDataTransition<NumType>;
};

#include "numericDataAttribute.I"

#endif
