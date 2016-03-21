/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stateMunger.cxx
 * @author drose
 * @date 2005-05-04
 */

#include "stateMunger.h"

TypeHandle StateMunger::_type_handle;

/**
 *
 */
StateMunger::
~StateMunger() {
}

/**
 * Given an input state, returns the munged state.
 */
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

/**
 * Given an input state, returns the munged state.
 */
CPT(RenderState) StateMunger::
munge_state_impl(const RenderState *state) {
  return state;
}
