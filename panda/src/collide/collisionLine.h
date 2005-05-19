// Filename: collisionLine.h
// Created by:  drose (05Jan05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONLINE_H
#define COLLISIONLINE_H

#include "pandabase.h"

#include "collisionRay.h"

////////////////////////////////////////////////////////////////////
//       Class : CollisionLine
// Description : An infinite line, similar to a CollisionRay, except
//               that it extends in both directions.  It is, however,
//               directional.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionLine : public CollisionRay {
PUBLISHED:
  INLINE CollisionLine();
  INLINE CollisionLine(const LPoint3f &origin, const LVector3f &direction);
  INLINE CollisionLine(float ox, float oy, float oz,
                       float dx, float dy, float dz);

public:
  INLINE CollisionLine(const CollisionLine &copy);
  virtual CollisionSolid *make_copy();

  virtual PT(CollisionEntry)
  test_intersection(const CollisionEntry &entry) const;

  virtual void output(ostream &out) const;

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


