/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionSolid.h
 * @author drose
 * @date 2000-04-24
 */

#ifndef COLLISIONSOLID_H
#define COLLISIONSOLID_H

#include "pandabase.h"

#include "config_collide.h"
#include "copyOnWriteObject.h"
#include "luse.h"
#include "pointerTo.h"
#include "renderState.h"
#include "geomNode.h"
#include "lightMutex.h"
#include "lightMutexHolder.h"
#include "pStatCollector.h"

class CollisionHandler;
class CollisionEntry;
class CollisionSphere;
class GeomNode;
class CollisionNode;
class CullTraverserData;

/**
 * The abstract base class for all things that can collide with other things
 * in the world, and all the things they can collide with (except geometry).
 *
 * This class and its derivatives really work very similarly to the way
 * BoundingVolume and all of its derivatives work.  There's a different
 * subclass for each basic shape of solid, and double-dispatch function calls
 * handle the subset of the N*N intersection tests that we care about.
 */
class EXPCL_PANDA_COLLIDE CollisionSolid : public CopyOnWriteObject {
public:
  CollisionSolid();
  CollisionSolid(const CollisionSolid &copy);
  virtual ~CollisionSolid();

  virtual CollisionSolid *make_copy()=0;
protected:
  virtual PT(CopyOnWriteObject) make_cow_copy();

PUBLISHED:
  virtual LPoint3 get_collision_origin() const=0;
  MAKE_PROPERTY(collision_origin, get_collision_origin);

  INLINE void set_tangible(bool tangible);
  INLINE bool is_tangible() const;
  MAKE_PROPERTY(tangible, is_tangible, set_tangible);

  INLINE void set_effective_normal(const LVector3 &effective_normal);
  INLINE void clear_effective_normal();
  INLINE bool has_effective_normal() const;
  INLINE const LVector3 &get_effective_normal() const;

  INLINE void set_respect_effective_normal(bool respect_effective_normal);
  INLINE bool get_respect_effective_normal() const;
  MAKE_PROPERTY(respect_effective_normal,
            get_respect_effective_normal,
            set_respect_effective_normal);

  CPT(BoundingVolume) get_bounds() const;
  void set_bounds(const BoundingVolume &bounding_volume);
  MAKE_PROPERTY(bounds, get_bounds, set_bounds);

public:
  virtual PT(CollisionEntry)
  test_intersection(const CollisionEntry &entry) const;

  virtual void xform(const LMatrix4 &mat);

  virtual PT(PandaNode) get_viz(const CullTraverser *trav,
                                const CullTraverserData &data,
                                bool bounds_only) const;

  virtual PStatCollector &get_volume_pcollector();
  virtual PStatCollector &get_test_pcollector();

PUBLISHED:
  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

protected:
  INLINE bool do_is_tangible() const;
  INLINE bool do_has_effective_normal() const;

  INLINE void mark_internal_bounds_stale();
  virtual PT(BoundingVolume) compute_internal_bounds() const;

  virtual PT(CollisionEntry)
  test_intersection_from_sphere(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_line(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_ray(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_segment(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_capsule(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_parabola(const CollisionEntry &entry) const;
  virtual PT(CollisionEntry)
  test_intersection_from_box(const CollisionEntry &entry) const;

  static void report_undefined_intersection_test(TypeHandle from_type,
                                                 TypeHandle into_type);
  static void report_undefined_from_intersection(TypeHandle from_type);

  INLINE void mark_viz_stale();
  virtual void fill_viz_geom();

  CPT(RenderState) get_solid_viz_state();
  CPT(RenderState) get_wireframe_viz_state();
  CPT(RenderState) get_other_viz_state();
  CPT(RenderState) get_solid_bounds_viz_state();
  CPT(RenderState) get_wireframe_bounds_viz_state();
  CPT(RenderState) get_other_bounds_viz_state();

  PT(GeomNode) _viz_geom;
  PT(GeomNode) _bounds_viz_geom;

private:
  LVector3 _effective_normal;
  PT(BoundingVolume) _internal_bounds;

  // Be careful reordering these bits, since they are written to a bam file.
  enum Flags {
    F_tangible                  = 0x01,
    F_effective_normal          = 0x02,
    F_viz_geom_stale            = 0x04,
    F_ignore_effective_normal   = 0x08,
    F_internal_bounds_stale     = 0x10,
  };
  int _flags;

  LightMutex _lock;

  static PStatCollector _volume_pcollector;
  static PStatCollector _test_pcollector;

public:
  virtual void write_datagram(BamWriter* manager, Datagram &me);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CopyOnWriteObject::init_type();
    register_type(_type_handle, "CollisionSolid",
                  CopyOnWriteObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CollisionSphere;
  friend class CollisionLine;
  friend class CollisionRay;
  friend class CollisionSegment;
  friend class CollisionCapsule;
  friend class CollisionParabola;
  friend class CollisionHandlerFluidPusher;
  friend class CollisionBox;
};

INLINE std::ostream &operator << (std::ostream &out, const CollisionSolid &cs) {
  cs.output(out);
  return out;
}

#include "collisionSolid.I"

#endif
