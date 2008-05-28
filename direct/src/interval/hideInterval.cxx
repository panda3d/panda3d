// Filename: hideInterval.cxx
// Created by:  drose (27Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "hideInterval.h"

int HideInterval::_unique_index;
TypeHandle HideInterval::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: HideInterval::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
HideInterval::
HideInterval(const NodePath &node, const string &name) :
  CInterval(name, 0.0, true),
  _node(node)
{
  nassertv(!node.is_empty());
  if (_name.empty()) {
    ostringstream name_strm;
    name_strm
      << "HideInterval-" << node.node()->get_name() << "-" << ++_unique_index;
    _name = name_strm.str();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HideInterval::instant
//       Access: Published, Virtual
//  Description: This is called in lieu of priv_initialize() .. priv_step()
//               .. priv_finalize(), when everything is to happen within
//               one frame.  The interval should initialize itself,
//               then leave itself in the final state.
////////////////////////////////////////////////////////////////////
void HideInterval::
priv_instant() {
  check_stopped(get_class_type(), "priv_instant");
  _node.hide();
  _state = S_final;
}

////////////////////////////////////////////////////////////////////
//     Function: HideInterval::reverse_instant
//       Access: Published, Virtual
//  Description: This is called in lieu of priv_reverse_initialize()
//               .. priv_step() .. priv_reverse_finalize(), when everything is
//               to happen within one frame.  The interval should
//               initialize itself, then leave itself in the initial
//               state.
////////////////////////////////////////////////////////////////////
void HideInterval::
priv_reverse_instant() {
  check_stopped(get_class_type(), "priv_reverse_instant");
  _node.show();
  _state = S_initial;
}
