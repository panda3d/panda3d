// Filename: eggOptcharUserData.h
// Created by:  drose (18Jul03)
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

#ifndef EGGOPTCHARUSERDATA_H
#define EGGOPTCHARUSERDATA_H

#include "pandatoolbase.h"
#include "eggUserData.h"
#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : EggOptcharUserData
// Description : This class contains extra user data which is
//               piggybacked onto EggGroup objects for the purpose of
//               the maya converter.
////////////////////////////////////////////////////////////////////
class EggOptcharUserData : public EggUserData {
public:
  INLINE EggOptcharUserData();
  INLINE EggOptcharUserData(const EggOptcharUserData &copy);
  INLINE void operator = (const EggOptcharUserData &copy);

  INLINE bool is_static() const;
  INLINE bool is_identity() const;
  INLINE bool is_empty() const;

  enum Flags {
    F_static   = 0x0001,
    F_identity = 0x0002,
    F_empty    = 0x0004,
    F_remove   = 0x0008
  };
  int _flags;
  LMatrix4d _static_mat;
  double _static_value;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggUserData::init_type();
    register_type(_type_handle, "EggOptcharUserData",
                  EggUserData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggOptcharUserData.I"

#endif
