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

#include <pandabase.h>

#include <nodeTransition.h>
#include <indent.h>

////////////////////////////////////////////////////////////////////
//       Class : NumericDataTransition
// Description : A NumericDataAttribute is a special data graph
//               attribute that is used to pass around a single
//               number.  A NumericDataTransition isn't often used,
//               but when it is it may transform the number by a scale
//               and/or offset.
////////////////////////////////////////////////////////////////////
template<class NumType>
class NumericDataTransition : public NodeTransition {
public:
  INLINE NumericDataTransition();
  INLINE NumericDataTransition(NumType scale, NumType offset);
  INLINE NumericDataTransition(const NumericDataTransition<NumType> &copy);
  INLINE void operator = (const NumericDataTransition<NumType> &copy);

public:

  INLINE bool is_identity() const;

  INLINE void set_scale(NumType scale);
  INLINE NumType get_scale() const;

  INLINE void set_offset(NumType offset);
  INLINE NumType get_offset() const;

  virtual NodeTransition *compose(const NodeTransition *other) const;
  virtual NodeTransition *invert() const;
  virtual NodeAttribute *apply(const NodeAttribute *attrib) const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeTransition *other) const;

private:
  NumType _scale;
  NumType _offset;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NodeTransition::init_type();
    register_type(_type_handle,
                  string("NumericDataTransition<") +
                  get_type_handle(NumType).get_name() + ">",
                  NodeTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "numericDataTransition.I"

#endif
