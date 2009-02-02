// Filename: tinyGeomMunger.h
// Created by:  drose (29Apr08)
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

#ifndef TINYGEOMMUNGER_H
#define TINYGEOMMUNGER_H

#include "pandabase.h"
#include "standardMunger.h"
#include "graphicsStateGuardian.h"
#include "renderState.h"

////////////////////////////////////////////////////////////////////
//       Class : TinyGeomMunger
// Description : This specialization on GeomMunger finesses vertices
//               for TinyPanda rendering.  This actually doesn't have
//               to do very much, since TinyPanda is not that
//               particular.
////////////////////////////////////////////////////////////////////
class EXPCL_TINYDISPLAY TinyGeomMunger : public StandardMunger {
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
