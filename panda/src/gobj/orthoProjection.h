// Filename: orthoProjection.h
// Created by:  mike (18Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef ORTHOPROJECTION_H
#define ORTHOPROJECTION_H

#include <pandabase.h>

#include "projection.h"
#include <frustum.h>


////////////////////////////////////////////////////////////////////
//       Class : OrthoProjection
// Description : An orthographic-type projection, with a frustum.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA OrthoProjection : public Projection {
PUBLISHED:
  INLINE OrthoProjection(const Frustumf &frustum);

public:
  virtual Projection *make_copy() const;
  virtual LMatrix4f get_projection_mat(CoordinateSystem cs = CS_default) const;

  virtual Geom* make_geometry(const Colorf &color = Colorf(0.0, 1.0, 0.0, 1.0),
                              CoordinateSystem cs = CS_default) const;

  virtual BoundingVolume *make_bounds(CoordinateSystem cs = CS_default) const;

  INLINE const Frustumf &get_frustum() const;


protected:
  Frustumf _frustum;
 
public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Projection::init_type();
    register_type(_type_handle, "OrthoProjection",
                  Projection::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "orthoProjection.I"

#endif


