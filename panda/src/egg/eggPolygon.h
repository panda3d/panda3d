/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggPolygon.h
 * @author drose
 * @date 1999-01-16
 */

#ifndef EGGPOLYGON_H
#define EGGPOLYGON_H

#include "pandabase.h"

#include "eggPrimitive.h"

/**
 * A single polygon.
 */
class EXPCL_PANDA_EGG EggPolygon : public EggPrimitive {
PUBLISHED:
  INLINE explicit EggPolygon(const std::string &name = "");
  INLINE EggPolygon(const EggPolygon &copy);
  INLINE EggPolygon &operator = (const EggPolygon &copy);

  virtual EggPolygon *make_copy() const override;

  virtual bool cleanup() override;

  bool calculate_normal(LNormald &result, CoordinateSystem cs = CS_default) const;
  bool is_planar() const;

  INLINE bool recompute_polygon_normal(CoordinateSystem cs = CS_default);

  INLINE bool triangulate_into(EggGroupNode *container, bool convex_also) const;
  PT(EggPolygon) triangulate_in_place(bool convex_also);

  virtual void write(std::ostream &out, int indent_level) const override;

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
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

};

#include "eggPolygon.I"

#endif
