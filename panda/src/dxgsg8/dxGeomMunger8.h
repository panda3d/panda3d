// Filename: dxGeomMunger8.h
// Created by:  drose (11Mar05)
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

#ifndef DXGEOMMUNGER8_H
#define DXGEOMMUNGER8_H

#include "pandabase.h"
#include "colorMunger.h"
#include "graphicsStateGuardian.h"

////////////////////////////////////////////////////////////////////
//       Class : DXGeomMunger8
// Description : This specialization on GeomMunger finesses vertices
//               for DirectX rendering.  In particular, it makes sure
//               colors are stored in DirectX's packed_argb format,
//               and that all relevant components are packed into a
//               single array, in the correct order.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXGeomMunger8 : public ColorMunger {
public:
  INLINE DXGeomMunger8(GraphicsStateGuardian *gsg, const RenderState *state);

protected:
  virtual CPT(qpGeomVertexFormat) munge_format_impl(const qpGeomVertexFormat *orig);
  virtual void munge_geom_impl(CPT(qpGeom) &geom, CPT(qpGeomVertexData) &data);
  virtual int compare_to_impl(const qpGeomMunger *other) const;

public:
  INLINE void *operator new(size_t size);

private:
  static qpGeomMunger *_deleted_chain;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ColorMunger::init_type();
    register_type(_type_handle, "DXGeomMunger8",
                  ColorMunger::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxGeomMunger8.I"

#endif
