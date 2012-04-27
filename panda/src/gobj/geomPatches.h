// Filename: geomPatches.h
// Created by:  drose (27Apr12)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef GEOMPATCHES_H
#define GEOMPATCHES_H

#include "pandabase.h"
#include "geomPrimitive.h"

////////////////////////////////////////////////////////////////////
//       Class : GeomPatches
// Description : Defines a series of "patches", fixed-size groupings
//               of vertices that must be processed by a tessellation
//               shader.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GOBJ GeomPatches : public GeomPrimitive {
PUBLISHED:
  GeomPatches(int num_vertices_per_patch, UsageHint usage_hint);
  GeomPatches(const GeomPatches &copy);
  virtual ~GeomPatches();
  ALLOC_DELETED_CHAIN(GeomPatches);

public:
  virtual PT(GeomPrimitive) make_copy() const;
  virtual PrimitiveType get_primitive_type() const;

  virtual int get_num_vertices_per_primitive() const;

public:
  virtual bool draw(GraphicsStateGuardianBase *gsg,
                    const GeomPrimitivePipelineReader *reader,
                    bool force) const;

private:
  int _num_vertices_per_patch;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomPrimitive::init_type();
    register_type(_type_handle, "GeomPatches",
                  GeomPrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class Geom;
};

#endif
