// Filename: eggObject.h
// Created by:  drose (17Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef EGGOBJECT_H
#define EGGOBJECT_H

#include "pandabase.h"
#include "eggUserData.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"
#include "pmap.h"

////////////////////////////////////////////////////////////////////
//       Class : EggObject
// Description : The highest-level base class in the egg directory.
//               (Almost) all things egg inherit from this.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggObject : public TypedReferenceCount {
PUBLISHED:
  INLINE EggObject();
  INLINE EggObject(const EggObject &copy);
  INLINE EggObject &operator = (const EggObject &copy);

  virtual ~EggObject();

  INLINE void set_user_data(EggUserData *user_data);
  INLINE EggUserData *get_user_data() const;
  INLINE EggUserData *get_user_data(TypeHandle type) const;
  INLINE bool has_user_data() const;
  INLINE bool has_user_data(TypeHandle type) const;
  INLINE void clear_user_data();
  INLINE void clear_user_data(TypeHandle type);

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
