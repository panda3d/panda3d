// Filename: qpgeomSprites.h
// Created by:  drose (05Apr05)
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

#ifndef qpGEOMSPRITES_H
#define qpGEOMSPRITES_H

#include "pandabase.h"
#include "qpgeomPrimitive.h"

////////////////////////////////////////////////////////////////////
//       Class : qpGeomSprites
// Description : Defines a collection of rectangular polygons with the
//               same texture applied to all of them.  This is most
//               useful for particle systems.
//
//               Each sprite is defined by a single vertex, at the
//               center of the polygon.  Each polygon may optionally
//               be rotated by a certain angle, and/or scaled in x and
//               y; if needed, these parameters are specified
//               per-vertex with the optional "rotate", "scale_x", and
//               "scale_y" vertex data column names.
//
//               The overall scale of the sprites is controlled by the
//               current RenderModeAttrib::get_thickness() and
//               get_perspective() parameters.  If present, scale_x
//               and scale_y apply an additional per-sprite scale.
//               The UV range is always (0, 0) in the upper left
//               corner to (1, 1) in the lower right; use a
//               TexMatrixAttrib to adjust this.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomSprites : public qpGeomPrimitive {
PUBLISHED:
  qpGeomSprites(qpGeomUsageHint::UsageHint usage_hint);
  qpGeomSprites(const qpGeomSprites &copy);
  virtual ~qpGeomSprites();

public:
  virtual PT(qpGeomPrimitive) make_copy() const;
  virtual PrimitiveType get_primitive_type() const;

  virtual int get_num_vertices_per_primitive() const;
  virtual int get_min_num_vertices_per_primitive() const;

public:
  virtual void draw(GraphicsStateGuardianBase *gsg) const;

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
    register_type(_type_handle, "qpGeomSprites",
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
