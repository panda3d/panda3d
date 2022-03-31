/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionLine.h
 * @author drose
 * @date 2005-01-05
 */

#ifndef COLLISIONLINE_H
#define COLLISIONLINE_H

#include "pandabase.h"

#include "collisionRay.h"

/**
 * An infinite line, similar to a CollisionRay, except that it extends in both
 * directions.  It is, however, directional.
 */
class EXPCL_PANDA_COLLIDE CollisionLine : public CollisionRay {
PUBLISHED:
  INLINE CollisionLine();
  INLINE explicit CollisionLine(const LPoint3 &origin, const LVector3 &direction);
  INLINE explicit CollisionLine(PN_stdfloat ox, PN_stdfloat oy, PN_stdfloat oz,
                                PN_stdfloat dx, PN_stdfloat dy, PN_stdfloat dz);

public:
  INLINE CollisionLine(const CollisionLine &copy);
  virtual CollisionSolid *make_copy();

  virtual PT(CollisionEntry)
  test_intersection(const CollisionEntry &entry) const;

  virtual void output(std::ostream &out) const;

protected:
  virtual void fill_viz_geom();

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
    CollisionRay::init_type();
    register_type(_type_handle, "CollisionLine",
                  CollisionRay::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionLine.I"

#endif
