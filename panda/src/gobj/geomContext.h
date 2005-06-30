// Filename: geomContext.h
// Created by:  drose (19Mar04)
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

#ifndef GEOMCONTEXT_H
#define GEOMCONTEXT_H

#include "pandabase.h"

#include "savedContext.h"
#include "geom.h"

////////////////////////////////////////////////////////////////////
//       Class : GeomContext
// Description : This is a special class object that holds all the
//               information returned by a particular GSG to indicate
//               the geom's internal context identifier.
//
//               Geoms typically have an immediate-mode and a
//               retained-mode operation.  When using geoms in
//               retained-mode (in response to Geom::prepare()),
//               the GSG will create some internal handle for the
//               geom and store it here.  The geom stores all of
//               these handles internally.
//
//               In the case of OpenGL, for example, a GeomContext
//               corresponds to a display list identifier.
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

