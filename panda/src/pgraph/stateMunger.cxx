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
  RenderState::MungedStates &munged_states = state->_munged_states;

  int id = get_gsg()->_id;
  int mi = munged_states.find(id);
  if (mi != -1) {
    if (!munged_states.get_data(mi).was_deleted()) {
      return munged_states.get_data(mi).p();
    } else {
      munged_states.remove_element(mi);
    }
  }

  CPT(RenderState) result = munge_state_impl(state);
  munged_states.store(id, result.p());

  return result;
}

/**
 * Given an input state, returns the munged state.
 */
CPT(RenderState) StateMunger::
munge_state_impl(const RenderState *state) {
  return state;
}
