// Filename: collisionSphere.h
// Created by:  drose (24Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONSPHERE_H
#define COLLISIONSPHERE_H

#include <pandabase.h>

#include "collisionSolid.h"

///////////////////////////////////////////////////////////////////
// 	 Class : CollisionSphere
// Description : A spherical collision volume or object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionSphere : public CollisionSolid {
PUBLISHED:
  INLINE CollisionSphere(const LPoint3f &center, float radius);
  INLINE CollisionSphere(float cx, float cy, float cz, float radius);

public:
  INLINE CollisionSphere(const CollisionSphere &copy);
  virtual CollisionSolid *make_copy();

  virtual int
  test_intersection(CollisionHandler *record,
		    const CollisionEntry &entry,
		    const CollisionSolid *into) const;

  virtual void xform(const LMatrix4f &mat);

  virtual void output(ostream &out) const;

PUBLISHED:
  INLINE void set_center(const LPoint3f &center);
  INLINE void set_center(float x, float y, float z);
  INLINE const LPoint3f &get_center() const;

  INLINE void set_radius(float radius);
  INLINE float get_radius() const;

protected:
  INLINE CollisionSphere(void);
  virtual void recompute_bound();

protected:
  virtual int
  test_intersection_from_sphere(CollisionHandler *record,
				const CollisionEntry &entry) const;
  virtual int
  test_intersection_from_ray(CollisionHandler *record,
			     const CollisionEntry &entry) const;
  virtual int
  test_intersection_from_segment(CollisionHandler *record,
				 const CollisionEntry &entry) const;

  virtual void recompute_viz(Node *parent);

  bool intersects_line(double &t1, double &t2,
		       const LPoint3f &from, const LVector3f &delta) const;

private:
  LPoint3f _center;
  float _radius;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  

  static TypedWriteable *make_CollisionSphere(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionSolid::init_type();
    register_type(_type_handle, "CollisionSphere",
		  CollisionSolid::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionSphere.I"

#endif


