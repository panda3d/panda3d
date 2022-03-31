/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionParabola.h
 * @author drose
 * @date 2007-10-11
 */

#ifndef COLLISIONPARABOLA_H
#define COLLISIONPARABOLA_H

#include "pandabase.h"

#include "collisionSolid.h"
#include "parabola.h"

class LensNode;

/**
 * This defines a parabolic arc, or subset of an arc, similar to the path of a
 * projectile or falling object.  It is finite, having a specific beginning
 * and end, but it is infinitely thin.
 *
 * Think of it as a wire bending from point t1 to point t2 along the path of a
 * pre-defined parabola.
 */
class EXPCL_PANDA_COLLIDE CollisionParabola : public CollisionSolid {
PUBLISHED:
  INLINE CollisionParabola();
  INLINE explicit CollisionParabola(const LParabola &parabola, PN_stdfloat t1, PN_stdfloat t2);

  virtual LPoint3 get_collision_origin() const;

public:
  INLINE CollisionParabola(const CollisionParabola &copy);
  virtual CollisionSolid *make_copy();

  virtual PT(CollisionEntry)
  test_intersection(const CollisionEntry &entry) const;

  virtual void xform(const LMatrix4 &mat);

  virtual PStatCollector &get_volume_pcollector();
  virtual PStatCollector &get_test_pcollector();

  virtual void output(std::ostream &out) const;

PUBLISHED:
  INLINE void set_parabola(const LParabola &parabola);
  INLINE const LParabola &get_parabola() const;

  INLINE void set_t1(PN_stdfloat t1);
  INLINE PN_stdfloat get_t1() const;

  INLINE void set_t2(PN_stdfloat t2);
  INLINE PN_stdfloat get_t2() const;

PUBLISHED:
  MAKE_PROPERTY(parabola, get_parabola, set_parabola);
  MAKE_PROPERTY(t1, get_t1, set_t1);
  MAKE_PROPERTY(t2, get_t2, set_t2);

protected:
  virtual PT(BoundingVolume) compute_internal_bounds() const;

protected:
  virtual void fill_viz_geom();

private:
  LParabola _parabola;
  PN_stdfloat _t1, _t2;

  static PStatCollector _volume_pcollector;
  static PStatCollector _test_pcollector;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionSolid::init_type();
    register_type(_type_handle, "CollisionParabola",
                  CollisionSolid::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionParabola.I"

#endif
