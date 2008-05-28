// Filename: eggNamedObject.h
// Created by:  drose (16Jan99)
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

#ifndef EGGNAMEDOBJECT_H
#define EGGNAMEDOBJECT_H

#include "pandabase.h"

#include "eggObject.h"
#include "namable.h"
#include "referenceCount.h"

////////////////////////////////////////////////////////////////////
//       Class : EggNamedObject
// Description : This is a fairly low-level base class--any egg
//               object that has a name.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggNamedObject : public EggObject, public Namable {
PUBLISHED:
  INLINE EggNamedObject(const string &name = "");
  INLINE EggNamedObject(const EggNamedObject &copy);
  INLINE EggNamedObject &operator = (const EggNamedObject &copy);

  void output(ostream &out) const;

public:
  void write_header(ostream &out, int indent_level,
                    const char *egg_keyword) const;


  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggObject::init_type();
    Namable::init_type();
    register_type(_type_handle, "EggNamedObject",
                  EggObject::get_class_type(),
                  Namable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const EggNamedObject &n);

#include "eggNamedObject.I"

#endif
