// Filename: boundingHexahedron.h
// Created by:  drose (03Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef BOUNDINGHEXAHEDRON_H
#define BOUNDINGHEXAHEDRON_H

#include <pandabase.h>

#include "finiteBoundingVolume.h"
#include "frustum.h"
#include "plane.h"

#include <coordinateSystem.h>


///////////////////////////////////////////////////////////////////
// 	 Class : BoundingHexahedron
// Description : This defines a bounding convex hexahedron.  It is
//               typically used to represent a frustum, but may
//               represent any enclosing convex hexahedron.
//
//               This class does not support any of the around() or
//               extend_by() functions, but all other functionality
//               should be well-defined.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BoundingHexahedron : public FiniteBoundingVolume {
public:
  INLINE BoundingHexahedron();
  BoundingHexahedron(const Frustumf &frustum, bool is_ortho,
		     CoordinateSystem cs = CS_default);
  virtual BoundingVolume *make_copy() const;

  virtual LPoint3f get_min() const;
  virtual LPoint3f get_max() const;

  virtual LPoint3f get_approx_center() const;
  virtual void xform(const LMatrix4f &mat);

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

  INLINE int get_num_points() const;
  INLINE LPoint3f get_point(int n) const;
  INLINE int get_num_planes() const;
  INLINE Planef get_plane(int n) const;

protected:
  virtual bool extend_other(BoundingVolume *other) const;
  virtual bool around_other(BoundingVolume *other,
			    const BoundingVolume **first,
			    const BoundingVolume **last) const;
  virtual int contains_other(const BoundingVolume *other) const;


  virtual bool extend_by_point(const LPoint3f &point);
  virtual bool extend_by_sphere(const BoundingSphere *sphere);
  virtual bool extend_by_hexahedron(const BoundingHexahedron *hexahedron);

  virtual bool around_points(const LPoint3f *first,
			     const LPoint3f *last);
  virtual bool around_spheres(const BoundingVolume **first,
			      const BoundingVolume **last);
  virtual bool around_hexahedrons(const BoundingVolume **first,
				  const BoundingVolume **last);

  virtual int contains_point(const LPoint3f &point) const;
  virtual int contains_lineseg(const LPoint3f &a, const LPoint3f &b) const;
  virtual int contains_sphere(const BoundingSphere *sphere) const;
  virtual int contains_hexahedron(const BoundingHexahedron *hexahedron) const;

private:
  void set_planes();
  void set_centroid();

private:
  enum {
    num_points = 8,
    num_planes = 6
  };
  LPoint3f _points[num_points];
  Planef _planes[num_planes];
  LPoint3f _centroid;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FiniteBoundingVolume::init_type();
    register_type(_type_handle, "BoundingHexahedron",
		  FiniteBoundingVolume::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class BoundingSphere;
};

#include "boundingHexahedron.I"

#endif
