////////////////////////////////////////////////////////////////////////
// Filename    : evade.cxx
// Created by  : Deepak, John, Navin
// Date        :  24 Oct 09
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

#include "evade.h"

Evade::Evade(AICharacter *ai_ch, NodePath target_object, double panic_distance,
                                          double relax_distance, float evade_wt) {
  _ai_char = ai_ch;

  _evade_target = target_object;
  _evade_distance = panic_distance;
  _evade_relax_distance = relax_distance;
  _evade_weight = evade_wt;

  _evade_done = true;
  _evade_activate_done = false;
}

Evade::~Evade() {
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function : do_evade
// Description : This function performs the evade and returns an evade force which is used
//                in the calculate_prioritized function.
//                In case the AICharacter is past the (panic + relax) distance,
//                it resets to evade_activate.
//                This function is not to be used by the user.

/////////////////////////////////////////////////////////////////////////////////

LVecBase3 Evade::do_evade() {
  assert(_evade_target && "evade target not assigned");

  _evade_direction = _ai_char->_ai_char_np.get_pos(_ai_char->_window_render) - _evade_target.get_pos(_ai_char->_window_render);
  double distance = _evade_direction.length();

  _evade_direction.normalize();
  LVecBase3 desired_force = _evade_direction * _ai_char->_movt_force;

  if(distance > (_evade_distance + _evade_relax_distance)) {
    if((_ai_char->_steering->_behaviors_flags | _ai_char->_steering->_evade) == _ai_char->_steering->_evade) {
      _ai_char->_steering->_steering_force = LVecBase3(0.0, 0.0, 0.0);
    }
    _ai_char->_steering->turn_off("evade");
    _ai_char->_steering->turn_on("evade_activate");
    _evade_done = true;
    return(LVecBase3(0.0, 0.0, 0.0));
  }
  else {
      _evade_done = false;
      return(desired_force);
  }
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function : evade_activate
// Description : This function checks for whether the target is within the panic distance.
//                When this is true, it calls the do_evade function and sets the evade direction.
//                This function is not to be used by the user.

/////////////////////////////////////////////////////////////////////////////////

void Evade::evade_activate() {
    _evade_direction = (_ai_char->_ai_char_np.get_pos(_ai_char->_window_render) - _evade_target.get_pos(_ai_char->_window_render));
  double distance = _evade_direction.length();
  _evade_activate_done = false;

  if(distance < _evade_distance) {
      _ai_char->_steering->turn_off("evade_activate");
      _ai_char->_steering->turn_on("evade");
      _evade_activate_done = true;
  }
}
