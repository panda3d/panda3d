// Filename: standardMunger.h
// Created by:  drose (21Mar05)
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

#ifndef STANDARDMUNGER_H
#define STANDARDMUNGER_H

#include "pandabase.h"
#include "qpgeomMunger.h"
#include "graphicsStateGuardian.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "renderModeAttrib.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : StandardMunger
// Description : Performs some generic munging that is appropriate for
//               all GSG types; for instance, applies ColorAttrib and
//               ColorScaleAttrib to the vertices, and checks for
//               hardware-accelerated animation capabilities.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA StandardMunger : public qpGeomMunger {
public:
  StandardMunger(const GraphicsStateGuardianBase *gsg, const RenderState *state,
                 int num_components, NumericType numeric_type,
                 Contents contents);
  virtual ~StandardMunger();

protected:
  virtual CPT(qpGeomVertexData) munge_data_impl(const qpGeomVertexData *data);
  virtual int compare_to_impl(const qpGeomMunger *other) const;
  virtual int geom_compare_to_impl(const qpGeomMunger *other) const;

private:
  int _num_components;
  NumericType _numeric_type;
  Contents _contents;
  CPT(GraphicsStateGuardian) _gsg;
  CPT(ColorAttrib) _color;
  CPT(ColorScaleAttrib) _color_scale;
  CPT(RenderModeAttrib) _render_mode;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    qpGeomMunger::init_type();
    register_type(_type_handle, "StandardMunger",
                  qpGeomMunger::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "standardMunger.I"

#endif

