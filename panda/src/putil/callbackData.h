// Filename: callbackData.h
// Created by:  drose (13Mar09)
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

#ifndef CALLBACKDATA_H
#define CALLBACKDATA_H

#include "pandabase.h"
#include "typedObject.h"

////////////////////////////////////////////////////////////////////
//       Class : CallbackData
// Description : This is a generic data block that is passed along to
//               a CallbackObject when a callback is made.  It
//               contains data specific to the particular callback
//               type in question.
//
//               This is actually an abstract base class and contains
//               no data.  Specializations of this class will contain
//               the actual data relevant to each callback type.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PUTIL CallbackData : public TypedObject {
protected:
  INLINE CallbackData();

PUBLISHED:
  virtual void output(ostream &out) const;

  virtual void upcall();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "CallbackData",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

inline ostream &operator << (ostream &out, const CallbackData &cbd) {
  cbd.output(out);
  return out;
}

#include "callbackData.I"

#endif
