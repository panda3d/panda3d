////////////////////////////////////////////////////////////////////////
// Filename    : flee.cxx
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

#include "flee.h"

Flee::Flee(AICharacter *ai_ch, NodePath target_object, double panic_distance,
                                      double relax_distance, float flee_wt){

  _ai_char = ai_ch;

  _flee_position = target_object.get_pos(_ai_char->_window_render);
  _flee_distance = panic_distance;
  _flee_weight = flee_wt;
  _flee_relax_distance = relax_distance;

  _flee_done = false;
  _flee_activate_done = false;
}

Flee::Flee(AICharacter *ai_ch, LVecBase3 pos, double panic_distance,
                                double relax_distance, float flee_wt){

    _ai_char = ai_ch;

  _flee_position = pos;
  _flee_distance = panic_distance;
  _flee_weight = flee_wt;
  _flee_relax_distance = relax_distance;

  _flee_done = false;
  _flee_activate_done = false;
}

Flee::~Flee() {
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function : do_flee
// Description : This function performs the flee and returns a flee force which is used
//                in the calculate_prioritized function.
//                In case the AICharacter is past the (panic + relax) distance,
//                it resets to flee_activate.
//                This function is not to be used by the user.

/////////////////////////////////////////////////////////////////////////////////

LVecBase3 Flee::do_flee() {
  LVecBase3 dirn;
  double distance;
  LVecBase3 desired_force;

  dirn = _ai_char->_ai_char_np.get_pos(_ai_char->_window_render) - _flee_present_pos;
  distance = dirn.length();
  desired_force = _flee_direction * _ai_char->_movt_force;

  if(distance > (_flee_distance + _flee_relax_distance)) {
    if((_ai_char->_steering->_behaviors_flags | _ai_char->_steering->_flee) == _ai_char->_steering->_flee) {
        _ai_char->_steering->_steering_force = LVecBase3(0.0, 0.0, 0.0);
    }
    _flee_done = true;
    _ai_char->_steering->turn_off("flee");
    _ai_char->_steering->turn_on("flee_activate");
    return(LVecBase3(0.0, 0.0, 0.0));
  }
  else {
      return(desired_force);
  }
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function : flee_activate
// Description : This function checks for whether the target is within the panic distance.
//                When this is true, it calls the do_flee function and sets the flee direction.
//                This function is not to be used by the user.

/////////////////////////////////////////////////////////////////////////////////

void Flee::flee_activate() {
  LVecBase3 dirn;
  double distance;

  _flee_activate_done = false;

  dirn = (_ai_char->_ai_char_np.get_pos(_ai_char->_window_render) - _flee_position);
  distance = dirn.length();

  if(distance < _flee_distance) {
      _flee_direction = _ai_char->_ai_char_np.get_pos(_ai_char->_window_render) - _flee_position;
      _flee_direction.normalize();
      _flee_present_pos = _ai_char->_ai_char_np.get_pos(_ai_char->_window_render);
      _ai_char->_steering->turn_off("flee_activate");
      _ai_char->_steering->turn_on("flee");
      _flee_activate_done = true;
  }
}
