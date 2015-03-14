////////////////////////////////////////////////////////////////////////
// Filename    : seek.cxx
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

#include "seek.h"

Seek::Seek(AICharacter *ai_ch, NodePath target_object, float seek_wt) {
  _ai_char = ai_ch;

  _seek_position = target_object.get_pos(_ai_char->_window_render);
  _seek_weight = seek_wt;

  _seek_direction = _seek_position - _ai_char->_ai_char_np.get_pos(_ai_char->_window_render);
  _seek_direction.normalize();

  _seek_done = false;
}

Seek::Seek(AICharacter *ai_ch, LVecBase3 pos, float seek_wt) {
      _ai_char = ai_ch;

  _seek_position = pos;
  _seek_weight = seek_wt;

  _seek_direction = _seek_position - _ai_char->_ai_char_np.get_pos(_ai_char->_window_render);
  _seek_direction.normalize();

  _seek_done = false;
}

Seek::~Seek() {
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function : do_seek
// Description : This function performs the seek and returns a seek force which is used
//                in the calculate_prioritized function.
//                This function is not to be used by the user.

/////////////////////////////////////////////////////////////////////////////////

LVecBase3 Seek::do_seek() {
  double target_distance = (_seek_position - _ai_char->_ai_char_np.get_pos(_ai_char->_window_render)).length();

    if(int(target_distance) == 0) {
        _seek_done = true;
    _ai_char->_steering->_steering_force = LVecBase3(0.0, 0.0, 0.0);
    _ai_char->_steering->turn_off("seek");
    return(LVecBase3(0.0, 0.0, 0.0));
  }

  LVecBase3 desired_force = _seek_direction * _ai_char->_movt_force;
  return(desired_force);
}
