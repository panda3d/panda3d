// Filename: perspectiveProjection.h
// Created by:  drose (18Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef PERSPECTIVEPROJECTION_H
#define PERSPECTIVEPROJECTION_H

#include <pandabase.h>

#include "projection.h"
#include <frustum.h>


////////////////////////////////////////////////////////////////////
//       Class : PerspectiveProjection
// Description : A perspective-type projection, with a frustum.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PerspectiveProjection : public Projection {
PUBLISHED:
  INLINE PerspectiveProjection(const Frustumf &frustum);

public:
  virtual Projection *make_copy() const;
  virtual LMatrix4f get_projection_mat(CoordinateSystem cs = CS_default) const;

  virtual Geom* make_geometry(const Colorf &color = Colorf(0.0, 1.0, 0.0, 1.0),
			      CoordinateSystem cs = CS_default) const;

  virtual BoundingVolume *make_bounds(CoordinateSystem cs = CS_default) const;

  virtual bool extrude(const LPoint2f &point2d,
		       LPoint3f &origin, LVector3f &direction,
		       CoordinateSystem cs = CS_default) const;

  INLINE const Frustumf &get_frustum() const;
  INLINE void set_frustum(const Frustumf &frust);

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
    register_type(_type_handle, "PerspectiveProjection",
		  Projection::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "perspectiveProjection.I"

#endif


