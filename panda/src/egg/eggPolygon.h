// Filename: eggPolygon.h
// Created by:  drose (16Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGPOLYGON_H
#define EGGPOLYGON_H

#include <pandabase.h>

#include "eggPrimitive.h"

////////////////////////////////////////////////////////////////////
// 	 Class : EggPolygon
// Description : A single polygon.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggPolygon : public EggPrimitive {
public:
  INLINE EggPolygon(const string &name = "");
  INLINE EggPolygon(const EggPolygon &copy);
  INLINE EggPolygon &operator = (const EggPolygon &copy);

  virtual bool cleanup();

  bool calculate_normal(Normald &result, CoordinateSystem cs = CS_default) const;
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
