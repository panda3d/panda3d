// Filename: eggNurbsCurve.h
// Created by:  drose (15Feb00)
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

#ifndef EGGNURBSCURVE_H
#define EGGNURBSCURVE_H

#include "pandabase.h"

#include "eggCurve.h"

#include "vector_double.h"

////////////////////////////////////////////////////////////////////
//       Class : EggNurbsCurve
// Description : A parametric NURBS curve.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggNurbsCurve : public EggCurve {
PUBLISHED:
  INLINE EggNurbsCurve(const string &name = "");
  INLINE EggNurbsCurve(const EggNurbsCurve &copy);
  INLINE EggNurbsCurve &operator = (const EggNurbsCurve &copy);

  void setup(int order, int num_knots);

  INLINE void set_order(int order);
  void set_num_knots(int num);

  INLINE void set_knot(int k, double value);

  bool is_valid() const;

  INLINE int get_order() const;
  INLINE int get_degree() const;
  INLINE int get_num_knots() const;
  INLINE int get_num_cvs() const;

  bool is_closed() const;

  INLINE double get_knot(int k) const;

  virtual void write(ostream &out, int indent_level) const;

private:
  typedef vector_double Knots;
  Knots _knots;
  int _order;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggPrimitive::init_type();
    register_type(_type_handle, "EggNurbsCurve",
                  EggPrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "eggNurbsCurve.I"

#endif
