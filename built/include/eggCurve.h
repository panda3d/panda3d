/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggCurve.h
 * @author drose
 * @date 2000-02-15
 */

#ifndef EGGCURVE_H
#define EGGCURVE_H

#include "pandabase.h"

#include "eggPrimitive.h"

/**
 * A parametric curve of some kind.  See EggNurbsCurve.
 */
class EXPCL_PANDA_EGG EggCurve : public EggPrimitive {
PUBLISHED:
  INLINE explicit EggCurve(const std::string &name = "");
  INLINE EggCurve(const EggCurve &copy);
  INLINE EggCurve &operator = (const EggCurve &copy);

  enum CurveType {
    CT_none,
    CT_xyz,
    CT_hpr,
    CT_t
  };

  INLINE void set_subdiv(int subdiv);
  INLINE int get_subdiv() const;

  INLINE void set_curve_type(CurveType type);
  INLINE CurveType get_curve_type() const;

  static CurveType string_curve_type(const std::string &string);

private:
  int _subdiv;
  CurveType _type;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggPrimitive::init_type();
    register_type(_type_handle, "EggCurve",
                  EggPrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

std::ostream &operator << (std::ostream &out, EggCurve::CurveType t);

#include "eggCurve.I"

#endif
