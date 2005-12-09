// Filename: dxGeomMunger9.h
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

#ifndef DXGEOMMUNGER9_H
#define DXGEOMMUNGER9_H

#include "pandabase.h"
#include "standardMunger.h"
#include "graphicsStateGuardian.h"

////////////////////////////////////////////////////////////////////
//       Class : DXGeomMunger9
// Description : This specialization on GeomMunger finesses vertices
//               for DirectX rendering.  In particular, it makes sure
//               colors are stored in DirectX's packed_argb format,
//               and that all relevant components are packed into a
//               single array, in the correct order.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXGeomMunger9 : public StandardMunger {
public:
  INLINE DXGeomMunger9(GraphicsStateGuardian *gsg, const RenderState *state);

protected:
  virtual CPT(GeomVertexFormat) munge_format_impl(const GeomVertexFormat *orig,
                                                    const GeomVertexAnimationSpec &animation);

  virtual int compare_to_impl(const GeomMunger *other) const;
  virtual int geom_compare_to_impl(const GeomMunger *other) const;

public:
  INLINE void *operator new(size_t size);

private:
  CPT(TextureAttrib) _texture;
  CPT(TexGenAttrib) _tex_gen;

  static GeomMunger *_deleted_chain;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    StandardMunger::init_type();
    register_type(_type_handle, "DXGeomMunger9",
                  StandardMunger::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxGeomMunger9.I"

#endif

