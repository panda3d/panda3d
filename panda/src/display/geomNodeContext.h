// Filename: geomNodeContext.h
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

#ifndef GEOMNODECONTEXT_H
#define GEOMNODECONTEXT_H

#include "pandabase.h"

#include "savedContext.h"

class GeomNode;

////////////////////////////////////////////////////////////////////
//       Class : GeomNodeContext
// Description : This is a special class object, similar to a
//               TextureContext, that holds all the information
//               returned by a particular GSG to cache the rendering
//               information associated with one or more GeomNodes.
//               This is similar to, but different from, a Geom
//               context, which is associated with the containing Geom
//               class; a GSG might prefer to associate data with
//               either the Geom or the GeomNode or both.
//
//               This allows the GSG to precompute some information
//               necessary for drawing the Geoms as quickly as
//               possible and reuse that information across multiple
//               frames.  Typically, only static Geoms
//               (e.g. nonindexed) will be assigned GeomContexts.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomNodeContext : public SavedContext {
public:
  INLINE GeomNodeContext(GeomNode *node);

  // This cannot be a PT(GeomNode), because the geomNode and the GSG
  // both own their GeomNodeContexts!  That would create a circular
  // reference count.
  GeomNode *_node;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    SavedContext::init_type();
    register_type(_type_handle, "GeomNodeContext",
                  SavedContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "geomNodeContext.I"

#endif

