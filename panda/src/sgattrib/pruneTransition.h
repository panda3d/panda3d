// Filename: pruneTransition.h
// Created by:  drose (26Apr00)
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

#ifndef PRUNETRANSITION_H
#define PRUNETRANSITION_H

#include <pandabase.h>

#include <immediateTransition.h>
#include <luse.h>

////////////////////////////////////////////////////////////////////
//       Class : PruneTransition
// Description : This transition, when encountered in the scene graph,
//               causes rendering to stop at this point and not
//               traverse anything below.  In effect, it causes all
//               the geometry at this level and below to become
//               invisible.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PruneTransition : public ImmediateTransition {
PUBLISHED:
  INLINE PruneTransition();

public:
  virtual NodeTransition *make_copy() const;

  virtual bool sub_render(NodeRelation *arc,
                          const AllTransitionsWrapper &input_trans,
                          AllTransitionsWrapper &modify_trans,
                          RenderTraverser *trav);
  virtual bool has_sub_render() const;

public:
  static void register_with_read_factory(void);
  static TypedWritable *make_PruneTransition(const FactoryParams &params);

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
    register_type(_type_handle, "PruneTransition",
                  ImmediateTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "pruneTransition.I"

#endif
