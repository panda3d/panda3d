// Filename: tinyGeomMunger.h
// Created by:  drose (29Apr08)
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

#ifndef TINYGEOMMUNGER_H
#define TINYGEOMMUNGER_H

#include "pandabase.h"
#include "standardMunger.h"
#include "graphicsStateGuardian.h"
#include "renderState.h"

////////////////////////////////////////////////////////////////////
//       Class : TinyGeomMunger
// Description : This specialization on GeomMunger finesses vertices
//               for TinyGL rendering.  In particular, it makes sure
//               colors aren't stored in DirectX's packed_argb format.
////////////////////////////////////////////////////////////////////
class TinyGeomMunger : public StandardMunger {
public:
  TinyGeomMunger(GraphicsStateGuardian *gsg, const RenderState *state);
  virtual ~TinyGeomMunger();
  ALLOC_DELETED_CHAIN_DECL(TinyGeomMunger);

protected:
  virtual CPT(GeomVertexFormat) munge_format_impl(const GeomVertexFormat *orig,
                                                  const GeomVertexAnimationSpec &animation);
  virtual CPT(GeomVertexFormat) premunge_format_impl(const GeomVertexFormat *orig);

  virtual int compare_to_impl(const GeomMunger *other) const;
  virtual int geom_compare_to_impl(const GeomMunger *other) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    StandardMunger::init_type();
    register_type(_type_handle, "TinyGeomMunger",
                  StandardMunger::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinyGeomMunger.I"

#endif
