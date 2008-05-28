// Filename: eggPoint.h
// Created by:  drose (15Dec99)
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

#ifndef EGGPOINT_H
#define EGGPOINT_H

#include "pandabase.h"

#include "eggPrimitive.h"

////////////////////////////////////////////////////////////////////
//       Class : EggPoint
// Description : A single point, or a collection of points as defined
//               by a single <PointLight> entry.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggPoint : public EggPrimitive {
PUBLISHED:
  INLINE EggPoint(const string &name = "");
  INLINE EggPoint(const EggPoint &copy);
  INLINE EggPoint &operator = (const EggPoint &copy);

  INLINE bool has_thick() const;
  INLINE double get_thick() const;
  INLINE void set_thick(double thick);
  INLINE void clear_thick();

  INLINE bool has_perspective() const;
  INLINE bool get_perspective() const;
  INLINE void set_perspective(bool perspective);
  INLINE void clear_perspective();

  virtual bool cleanup();

  virtual void write(ostream &out, int indent_level) const;

private:
  enum Flags {
    F_has_thick       = 0x0001,
    F_perspective     = 0x0002,
    F_has_perspective = 0x0004,
  };

  int _flags;
  double _thick;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggPrimitive::init_type();
    register_type(_type_handle, "EggPoint",
                  EggPrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "eggPoint.I"

#endif
