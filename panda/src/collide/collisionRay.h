// Filename: collisionRay.h
// Created by:  drose (22Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONRAY_H
#define COLLISIONRAY_H

#include <pandabase.h>

#include "collisionSolid.h"

class ProjectionNode;

///////////////////////////////////////////////////////////////////
//       Class : CollisionRay
// Description : An infinite ray, with a specific origin and
//               direction.  It begins at its origin and continues in
//               one direction to infinity, and it has no radius.
//               Useful for picking from a window, or for gravity
//               effects.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionRay : public CollisionSolid {
PUBLISHED:
  INLINE CollisionRay();
  INLINE CollisionRay(const LPoint3f &origin, const LVector3f &direction);
  INLINE CollisionRay(float ox, float oy, float oz, 
                      float dx, float dy, float dz);

public:
  INLINE CollisionRay(const CollisionRay &copy);
  virtual CollisionSolid *make_copy();

  virtual int
  test_intersection(CollisionHandler *record,
                    const CollisionEntry &entry,
                    const CollisionSolid *into) const;

  virtual void xform(const LMatrix4f &mat);
  virtual LPoint3f get_collision_origin() const;

  virtual void output(ostream &out) const;

PUBLISHED:
  INLINE void set_origin(const LPoint3f &origin);
  INLINE void set_origin(float x, float y, float z);
  INLINE const LPoint3f &get_origin() const;

  INLINE void set_direction(const LVector3f &direction);
  INLINE void set_direction(float x, float y, float z);
  INLINE const LVector3f &get_direction() const;

  bool set_projection(ProjectionNode *camera, const LPoint2f &point);
  INLINE bool set_projection(ProjectionNode *camera, float px, float py);

protected:
  virtual void recompute_bound();

protected:
  virtual void recompute_viz(Node *parent);

private:
  LPoint3f _origin;
  LVector3f _direction;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionSolid::init_type();
    register_type(_type_handle, "CollisionRay",
                  CollisionSolid::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionRay.I"

#endif


