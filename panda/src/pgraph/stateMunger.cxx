// Filename: stateMunger.cxx
// Created by:  drose (04May05)
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

#include "stateMunger.h"

TypeHandle StateMunger::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: StateMunger::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
StateMunger::
~StateMunger() {
}

////////////////////////////////////////////////////////////////////
//     Function: StateMunger::munge_state
//       Access: Public
//  Description: Given an input state, returns the munged state.
////////////////////////////////////////////////////////////////////
CPT(RenderState) StateMunger::
munge_state(const RenderState *state) {
  int mi = _state_map.find(state);
  if (mi != -1) {
    if (!_state_map.get_data(mi).was_deleted()) {
      return _state_map.get_data(mi).p();
    }
  }

  CPT(RenderState) result = munge_state_impl(state);
  _state_map.store(state, result.p());

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: StateMunger::munge_state_impl
//       Access: Protected, Virtual
//  Description: Given an input state, returns the munged state.
////////////////////////////////////////////////////////////////////
CPT(RenderState) StateMunger::
munge_state_impl(const RenderState *state) {
  return state;
}
