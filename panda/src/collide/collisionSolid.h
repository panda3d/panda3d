// Filename: collisionSolid.h
// Created by:  drose (24Apr00)
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

#ifndef COLLISIONSOLID_H
#define COLLISIONSOLID_H

#include "pandabase.h"

#include "typedWritableReferenceCount.h"
#include "boundedObject.h"
#include "luse.h"
#include "pointerTo.h"
#include "renderState.h"
#include "geomNode.h"

class CollisionHandler;
class CollisionEntry;
class CollisionSphere;
class GeomNode;
class CollisionNode;
class CullTraverserData;

///////////////////////////////////////////////////////////////////
//       Class : CollisionSolid
// Description : The abstract base class for all things that can
//               collide with other things in the world, and all the
//               things they can collide with (except geometry).
//
//               This class and its derivatives really work very
//               similarly to the way BoundingVolume and all of its
//               derivatives work.  There's a different subclass for
//               each basic shape of solid, and double-dispatch
//               function calls handle the subset of the N*N
//               intersection tests that we care about.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionSolid :
  public TypedWritableReferenceCount, public BoundedObject {
public:
  CollisionSolid();
  CollisionSolid(const CollisionSolid &copy);
  virtual ~CollisionSolid();

  virtual CollisionSolid *make_copy()=0;
  virtual LPoint3f get_collision_origin() const=0;

PUBLISHED:
  INLINE void set_tangible(bool tangible);
  INLINE bool is_tangible() const;

  INLINE void set_effective_normal(const LVector3f &effective_normal);
  INLINE void clear_effective_normal();
  INLINE bool has_effective_normal() const;
  INLINE const LVector3f &get_effective_normal() const;

public:
  virtual PT(CollisionEntry)
  test_intersection(const CollisionEntry &entry) const;

  virtual void xform(const LMatrix4f &mat);

  virtual PT(PandaNode) get_viz(const CullTraverserData &data) const;

PUBLISHED:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual PT(CollisionEntry)
  test_intersection_from_sphere(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_ray(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_segment(const CollisionEntry &entry) const;

  static void report_undefined_intersection_test(TypeHandle from_type,
                                                 TypeHandle into_type);
  static void report_undefined_from_intersection(TypeHandle from_type);

  INLINE void mark_viz_stale();
  virtual void fill_viz_geom();

  CPT(RenderState) get_solid_viz_state();
  CPT(RenderState) get_wireframe_viz_state();
  CPT(RenderState) get_other_viz_state();

  PT(GeomNode) _viz_geom;

private:
  LVector3f _effective_normal;

  enum Flags {
    F_tangible         = 0x01,
    F_effective_normal = 0x02,
    F_viz_geom_stale   = 0x04,
  };
  int _flags;

public:
  virtual void write_datagram(BamWriter* manager, Datagram &me);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    BoundedObject::init_type();
    register_type(_type_handle, "CollisionSolid",
                  TypedWritableReferenceCount::get_class_type(),
                  BoundedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CollisionSphere;
  friend class CollisionRay;
  friend class CollisionSegment;
};

INLINE ostream &operator << (ostream &out, const CollisionSolid &cs) {
  cs.output(out);
  return out;
}

#include "collisionSolid.I"

#endif

