// Filename: switchNodeOne.h
// Created by:  drose (15May01)
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

#ifndef SWITCHNODEONE_H
#define SWITCHNODEONE_H

#include <pandabase.h>

#include <switchNode.h>
#include "config_switchnode.h"

class RenderTraverser;
class ArcChain;

////////////////////////////////////////////////////////////////////
//       Class : SwitchNodeOne
// Description : A specialization on SwitchNode for the common kind of
//               switching node that will only select one child of its
//               several available.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SwitchNodeOne : public SwitchNode {
PUBLISHED:
  INLINE SwitchNodeOne(const string &name = "");
  INLINE SwitchNodeOne(const SwitchNodeOne &copy);
  INLINE void operator = (const SwitchNodeOne &copy);

public:
  virtual bool is_child_visible(TypeHandle type, int index);

protected:
  INLINE void select_child(int index);

private:
  int _selected_child_index;

public:
  static TypeHandle get_class_type() {
      return _type_handle;
  }
  static void init_type() {
    SwitchNode::init_type();
    register_type( _type_handle, "SwitchNodeOne",
                SwitchNode::get_class_type() );
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle             _type_handle;
};

#include "switchNodeOne.I"

#endif
