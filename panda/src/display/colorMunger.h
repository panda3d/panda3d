// Filename: colorMunger.h
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

#ifndef COLORMUNGER_H
#define COLORMUNGER_H

#include "pandabase.h"
#include "qpgeomMunger.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : ColorMunger
// Description : Applies ColorAttrib and ColorScaleAttrib by munging
//               the vertex data.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorMunger : public qpGeomMunger {
public:
  ColorMunger(const GraphicsStateGuardianBase *gsg, const RenderState *state,
              int num_components,
              qpGeomVertexDataType::NumericType numeric_type);
  virtual ~ColorMunger();

protected:
  virtual CPT(qpGeomVertexData) munge_data_impl(const qpGeomVertexData *data);
  virtual int compare_to_impl(const qpGeomMunger *other) const;
  virtual int geom_compare_to_impl(const qpGeomMunger *other) const;

private:
  int _num_components;
  qpGeomVertexDataType::NumericType _numeric_type;
  CPT(ColorAttrib) _color;
  CPT(ColorScaleAttrib) _color_scale;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    qpGeomMunger::init_type();
    register_type(_type_handle, "ColorMunger",
                  qpGeomMunger::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "colorMunger.I"

#endif

