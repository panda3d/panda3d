// Filename: glGeomNodeContext.h
// Created by:  drose (12Jun01)
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

#ifndef GLGEOMNODECONTEXT_H
#define GLGEOMNODECONTEXT_H

#include <pandabase.h>

#ifdef WIN32_VC
// Must include windows.h before gl.h on NT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <GL/gl.h>
#include <geomNodeContext.h>
#include <geomNode.h>
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : GLGeomNodeContext
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAGL GLGeomNodeContext : public GeomNodeContext {
public:
  INLINE GLGeomNodeContext(GeomNode *node);

  // The GL display list index that draws the contents of this
  // GeomNode.
  GLuint _index;

  // A list of the dynamic Geoms within the GeomNode; these aren't
  // part of the above display list.
  typedef pvector<PT(dDrawable) > Geoms;
  Geoms _dynamic_geoms;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomNodeContext::init_type();
    register_type(_type_handle, "GLGeomNodeContext",
                  GeomNodeContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glGeomNodeContext.I"

#endif

