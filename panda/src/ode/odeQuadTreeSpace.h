// Filename: odeQuadTreeSpace.h
// Created by:  joswilso (27Dec06)
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

#ifndef ODEQUADTREESPACE_H
#define ODEQUADTREESPACE_H

#include "pandabase.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeSpace.h"


////////////////////////////////////////////////////////////////////
//       Class : OdeQuadTreeSpace
// Description : 
////////////////////////////////////////////////////////////////////c
class EXPCL_PANDAODE OdeQuadTreeSpace : public OdeSpace {
  friend class OdeSpace;

private:
  OdeQuadTreeSpace(dSpaceID id);

PUBLISHED:
  OdeQuadTreeSpace(const LPoint3f &center,
                   const LVecBase3f &extents,
                   const int depth);
  OdeQuadTreeSpace(OdeSpace &space,
                   const LPoint3f &center,
                   const LVecBase3f &extents,
                   const int depth);
  virtual ~OdeQuadTreeSpace();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeSpace::init_type();
    register_type(_type_handle, "OdeQuadTreeSpace",
		  OdeSpace::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeQuadTreeSpace.I"

#endif

