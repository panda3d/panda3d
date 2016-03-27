/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggOptcharUserData.h
 * @author drose
 * @date 2003-07-18
 */

#ifndef EGGOPTCHARUSERDATA_H
#define EGGOPTCHARUSERDATA_H

#include "pandatoolbase.h"
#include "eggUserData.h"
#include "luse.h"

/**
 * This class contains extra user data which is piggybacked onto EggGroup
 * objects for the purpose of the maya converter.
 */
class EggOptcharUserData : public EggUserData {
public:
  INLINE EggOptcharUserData();
  INLINE EggOptcharUserData(const EggOptcharUserData &copy);
  INLINE void operator = (const EggOptcharUserData &copy);

  INLINE bool is_static() const;
  INLINE bool is_identity() const;
  INLINE bool is_empty() const;
  INLINE bool is_top() const;

  enum Flags {
    F_static   = 0x0001,
    F_identity = 0x0002,
    F_empty    = 0x0004,
    F_top      = 0x0008,
    F_remove   = 0x0010,
    F_expose   = 0x0020,
    F_suppress = 0x0040,
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
