// Filename: eggObject.h
// Created by:  drose (17Jan99)
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

#ifndef EGGOBJECT_H
#define EGGOBJECT_H

#include "pandabase.h"
#include "eggUserData.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"
#include "pmap.h"

class EggTransform;

////////////////////////////////////////////////////////////////////
//       Class : EggObject
// Description : The highest-level base class in the egg directory.
//               (Almost) all things egg inherit from this.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggObject : public TypedReferenceCount {
PUBLISHED:
  EggObject();
  EggObject(const EggObject &copy);
  EggObject &operator = (const EggObject &copy);

  virtual ~EggObject();

  void set_user_data(EggUserData *user_data);
  EggUserData *get_user_data() const;
  EggUserData *get_user_data(TypeHandle type) const;
  bool has_user_data() const;
  bool has_user_data(TypeHandle type) const;
  void clear_user_data();
  void clear_user_data(TypeHandle type);

public:
  virtual EggTransform *as_transform();

private:
  typedef pmap<TypeHandle, PT(EggUserData) > UserData;
  UserData _user_data;
  PT(EggUserData) _default_user_data;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "EggObject",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggObject.I"

#endif
