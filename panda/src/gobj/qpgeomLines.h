// Filename: qpgeomLines.h
// Created by:  drose (22Mar05)
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

#ifndef qpGEOMLINES_H
#define qpGEOMLINES_H

#include "pandabase.h"
#include "qpgeomPrimitive.h"

////////////////////////////////////////////////////////////////////
//       Class : qpGeomLines
// Description : Defines a series of disconnected line segments.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomLines : public qpGeomPrimitive {
PUBLISHED:
  qpGeomLines(qpGeomUsageHint::UsageHint usage_hint);
  qpGeomLines(const qpGeomLines &copy);
  virtual ~qpGeomLines();

public:
  virtual PT(qpGeomPrimitive) make_copy() const;
  virtual PrimitiveType get_primitive_type() const;

  virtual int get_num_vertices_per_primitive() const;

public:
  virtual void draw(GraphicsStateGuardianBase *gsg) const;

protected:
  virtual CPTA_ushort rotate_impl() const;

public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    qpGeomPrimitive::init_type();
    register_type(_type_handle, "qpGeomLines",
                  qpGeomPrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class qpGeom;
};

#endif
