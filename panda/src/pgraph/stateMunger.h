// Filename: stateMunger.h
// Created by:  drose (04May05)
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

#ifndef STATEMUNGER_H
#define STATEMUNGER_H

#include "pandabase.h"
#include "qpgeomMunger.h"
#include "renderState.h"
#include "weakPointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : StateMunger
// Description : This is just a simple derivative of GeomMunger that
//               adds the ability to munge states.  That functionality
//               can't be declared in the base class, since it doesn't
//               really know about RenderState.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA StateMunger : public qpGeomMunger {
public:
  virtual ~StateMunger();
  CPT(RenderState) munge_state(const RenderState *state);

protected:
  CPT(RenderState) munge_state_impl(const RenderState *state);

  typedef pmap< WCPT(RenderState), WCPT(RenderState) > StateMap;
  StateMap _state_map;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    qpGeomMunger::init_type();
    register_type(_type_handle, "StateMunger",
                  qpGeomMunger::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "stateMunger.I"

#endif

