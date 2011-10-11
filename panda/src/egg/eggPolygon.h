// Filename: eggPolygon.h
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

#ifndef EGGPOLYGON_H
#define EGGPOLYGON_H

#include "pandabase.h"

#include "eggPrimitive.h"

////////////////////////////////////////////////////////////////////
//       Class : EggPolygon
// Description : A single polygon.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggPolygon : public EggPrimitive {
PUBLISHED:
  INLINE EggPolygon(const string &name = "");
  INLINE EggPolygon(const EggPolygon &copy);
  INLINE EggPolygon &operator = (const EggPolygon &copy);

  virtual bool cleanup();

  bool calculate_normal(LNormald &result, CoordinateSystem cs = CS_default) const;
  bool is_planar() const;

  INLINE bool recompute_polygon_normal(CoordinateSystem cs = CS_default);

  INLINE bool triangulate_into(EggGroupNode *container, bool convex_also) const;
  PT(EggPolygon) triangulate_in_place(bool convex_also);

  virtual void write(ostream &out, int indent_level) const;

private:
  bool decomp_concave(EggGroupNode *container, int asum, int x, int y) const;
  bool triangulate_poly(EggGroupNode *container, bool convex_also);

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggPrimitive::init_type();
    register_type(_type_handle, "EggPolygon",
                  EggPrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "eggPolygon.I"

#endif
