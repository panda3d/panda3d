// Filename: glGeomMunger_src.h
// Created by:  drose (10Mar05)
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

#include "pandabase.h"
#include "qpgeomMunger.h"
#include "graphicsStateGuardian.h"

////////////////////////////////////////////////////////////////////
//       Class : GLGeomMunger
// Description : This specialization on GeomMunger finesses vertices
//               for OpenGL rendering.  In particular, it makes sure
//               colors aren't stored in DirectX's packed_argb format.
////////////////////////////////////////////////////////////////////
class EXPCL_GL CLP(GeomMunger) : public qpGeomMunger {
public:
  INLINE CLP(GeomMunger)(GraphicsStateGuardian *gsg, const RenderState *state);

protected:
  virtual CPT(qpGeomVertexFormat) munge_format_impl(const qpGeomVertexFormat *orig);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    qpGeomMunger::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "GeomMunger",
                  qpGeomMunger::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glGeomMunger_src.I"

