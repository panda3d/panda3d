// Filename: glGeomContext_src.h
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

#include "pandabase.h"
#include "geomContext.h"

////////////////////////////////////////////////////////////////////
//       Class : GLGeomContext
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_GL CLP(GeomContext) : public GeomContext {
public:
  INLINE CLP(GeomContext)(Geom *geom);

  // This is the GL display list index.
  GLuint _index;

  // The number of vertices encoded in the display list, for stats
  // reporting.
#ifdef DO_PSTATS
  int _num_verts;
#endif

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "GeomContext",
                  GeomContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glGeomContext_src.I"

