// Filename: geomContext.h
// Created by:  drose (11Jun01)
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

#ifndef GEOMCONTEXT_H
#define GEOMCONTEXT_H

#include "pandabase.h"

#include "savedContext.h"

class Geom;

////////////////////////////////////////////////////////////////////
//       Class : GeomContext
// Description : This is a special class object, similar to a
//               TextureContext, that holds all the information
//               returned by a particular GSG to cache the rendering
//               information associated with one or more Geoms.  This
//               is similar to, but different from, a GeomNode
//               context, which is associated with the containing
//               GeomNode class; a GSG might prefer to associate data
//               with either the Geom or the GeomNode or both.
//
//               This allows the GSG to precompute some information
//               necessary for drawing the Geoms as quickly as
//               possible and reuse that information across multiple
//               frames.  Typically, only static Geoms
//               (e.g. nonindexed) will be assigned GeomContexts.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomContext : public SavedContext {
public:
  INLINE GeomContext(Geom *geom);

  // This cannot be a PT(Geom), because the geom and the GSG
  // both own their GeomContexts!  That would create a circular
  // reference count.
  Geom *_geom;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    SavedContext::init_type();
    register_type(_type_handle, "GeomContext",
                  SavedContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "geomContext.I"

#endif

