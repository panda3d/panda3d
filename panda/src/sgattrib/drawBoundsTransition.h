// Filename: drawBoundsTransition.h
// Created by:  drose (26Jun00)
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

#ifndef DRAWBOUNDSTRANSITION_H
#define DRAWBOUNDSTRANSITION_H

#include <pandabase.h>

#include <immediateTransition.h>
#include <nodeAttributes.h>

////////////////////////////////////////////////////////////////////
//       Class : DrawBoundsTransition
// Description : This is a special transition that does not change
//               rendering state, but instead causes a representation
//               of the arc's bounding volume to be rendered
//               immediately.  It's intended primarily for debugging
//               purposes; it's not designed to be terribly efficient.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DrawBoundsTransition : public ImmediateTransition {
public:
  DrawBoundsTransition();

public:
  virtual NodeTransition *make_copy() const;

  virtual bool sub_render(NodeRelation *arc,
                          const AllAttributesWrapper &attrib,
                          AllTransitionsWrapper &trans,
                          RenderTraverser *trav);
  virtual bool has_sub_render() const;

  NodeAttributes _outside_attrib;
  NodeAttributes _inside_attrib;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ImmediateTransition::init_type();
    register_type(_type_handle, "DrawBoundsTransition",
                  ImmediateTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class DrawBoundsAttribute;
};

#include "drawBoundsTransition.I"

#endif


