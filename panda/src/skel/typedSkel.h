// Filename: typedSkel.h
// Created by: jyelon (31Jan07)
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

#ifndef TYPEDSKEL_H
#define TYPEDSKEL_H

#include "pandabase.h"
#include "typedObject.h"

////////////////////////////////////////////////////////////////////
//       Class : TypedSkel
// Description : Skeleton object that inherits from TypedObject.
//               Stores an integer, and will return it on request.
//
//               The skeleton classes are intended to help you learn
//               how to add C++ classes to panda.  See also the manual,
//               "Adding C++ Classes to Panda."
////////////////////////////////////////////////////////////////////
class EXPCL_PANDASKEL TypedSkel : public TypedObject {
PUBLISHED:
  INLINE TypedSkel();
  INLINE ~TypedSkel();

  // These inline functions allow you to get and set _value.
  INLINE void set_value(int n);
  INLINE int  get_value();
  
  // These do the same thing as the functions above.
  void set_value_alt(int n);
  int  get_value_alt(); 

private:
  int _value;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "TypedSkel",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;

};

#include "typedSkel.I"

#endif

