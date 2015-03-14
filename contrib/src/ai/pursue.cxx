////////////////////////////////////////////////////////////////////////
// Filename    : pursue.cxx
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

#include "pursue.h"

Pursue::Pursue(AICharacter *ai_ch, NodePath target_object, float pursue_wt) {
  _ai_char = ai_ch;

  _pursue_target = target_object;
  _pursue_weight = pursue_wt;

  _pursue_done = false;
}

Pursue::~Pursue() {
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function : do_pursue
// Description : This function performs the pursue and returns a pursue force which is used
//                in the calculate_prioritized function.
//                In case the target has been reached it resets the forces to 0 so that the character stops.
//                This function is not to be used by the user.

/////////////////////////////////////////////////////////////////////////////////

LVecBase3 Pursue::do_pursue() {
  assert(_pursue_target && "pursue target not assigned");

  LVecBase3 present_pos = _ai_char->_ai_char_np.get_pos(_ai_char->_window_render);
  double target_distance = (_pursue_target.get_pos(_ai_char->_window_render) - present_pos).length();

  if(int(target_distance) == 0) {
    _pursue_done = true;
    _ai_char->_steering->_steering_force = LVecBase3(0.0, 0.0, 0.0);
    _ai_char->_steering->_pursue_force = LVecBase3(0.0, 0.0, 0.0);
    return(LVecBase3(0.0, 0.0, 0.0));
  }
  else {
    _pursue_done = false;
  }

  _pursue_direction = _pursue_target.get_pos(_ai_char->_window_render) - present_pos;
  _pursue_direction.normalize();

  LVecBase3 desired_force = _pursue_direction * _ai_char->_movt_force;
  return(desired_force);
}
