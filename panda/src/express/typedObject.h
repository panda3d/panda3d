// Filename: typedObject.h
// Created by:  drose (11May01)
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

#ifndef TYPEDOBJECT_H
#define TYPEDOBJECT_H

#include "pandabase.h"

#include "typeHandle.h"
#include "register_type.h"


////////////////////////////////////////////////////////////////////
//       Class : TypedObject
// Description : This is an abstract class that all classes which
//               use TypeHandle, and also provide virtual functions to
//               support polymorphism, should inherit from.  Each
//               derived class should define get_type(), which should
//               return the specific type of the derived class.
//               Inheriting from this automatically provides support
//               for is_of_type() and is_exact_type().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS TypedObject {
public:
  INLINE TypedObject();
  INLINE TypedObject(const TypedObject &copy);
  INLINE void operator = (const TypedObject &copy);

PUBLISHED:
  // A virtual destructor is just a good idea.
  virtual ~TypedObject();

  // Derived classes should override this function to return
  // get_class_type().
  virtual TypeHandle get_type() const=0;

  INLINE int get_type_index() const;
  INLINE bool is_of_type(TypeHandle handle) const;
  INLINE bool is_exact_type(TypeHandle handle) const;

public:
  // Derived classes should override this function to call
  // init_type().  It will only be called in error situations when the
  // type was for some reason not properly initialized.
  virtual TypeHandle force_init_type()=0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "TypedObject");
  }

private:
  static TypeHandle _type_handle;
};

#include "typedObject.I"

#endif
