// Filename: cullFaceAttrib.h
// Created by:  drose (27Feb02)
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

#ifndef CULLFACEATTRIB_H
#define CULLFACEATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"

////////////////////////////////////////////////////////////////////
//       Class : CullFaceAttrib
// Description : Indicates which faces should be culled based on their
//               vertex ordering.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullFaceAttrib : public RenderAttrib {
PUBLISHED:
  enum Mode {
    M_cull_none,                // Cull no polygons
    M_cull_clockwise,           // Cull clockwise-oriented polygons
    M_cull_counter_clockwise,   // Cull counter-clockwise-oriented polygons
    M_cull_all,         // Cull all polygons (other primitives are still drawn)
  };

private:
  INLINE CullFaceAttrib(Mode mode = M_cull_clockwise);

PUBLISHED:
  static CPT(RenderAttrib) make(Mode mode);

  INLINE Mode get_mode() const;

public:
  virtual void issue(GraphicsStateGuardianBase *gsg) const;
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  Mode _mode;

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
    RenderAttrib::init_type();
    register_type(_type_handle, "CullFaceAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cullFaceAttrib.I"

#endif

