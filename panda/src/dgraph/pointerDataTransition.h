// Filename: pointerDataTransition.h
// Created by:  jason (07Aug00)
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

#ifndef POINTERDATATRANSITION_H
#define POINTERDATATRANSITION_H

#include <pandabase.h>

#include <nodeTransition.h>
#include <indent.h>

////////////////////////////////////////////////////////////////////
//       Class : PointerDataTransition
// Description : A PointerDataAttribute is a special data graph
//               attribute that is used to pass around a single
//               pointer.
////////////////////////////////////////////////////////////////////
template<class PtrType>
class PointerDataTransition : public NodeTransition {
public:
  INLINE PointerDataTransition();
  INLINE PointerDataTransition(PtrType* ptr);
  INLINE PointerDataTransition(const PointerDataTransition<PtrType> &copy);
  INLINE void operator = (const PointerDataTransition<PtrType> &copy);

public:
  INLINE void set_value(PtrType* ptr);
  INLINE PtrType* get_value() const;

  virtual NodeTransition *compose(const NodeTransition *other) const;
  virtual NodeTransition *invert() const;
  virtual NodeAttribute *apply(const NodeAttribute *attrib) const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeTransition *other) const;

private:
  PtrType* _ptr;

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
                  string("PointerDataTransition<") +
                  get_type_handle(PtrType).get_name() + ">",
                  NodeTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "pointerDataTransition.I"

#endif
