////////////////////////////////////////////////////////////////////////
// Filename    : flock.cxx
// Created by  : Deepak, John, Navin
// Date        :  12 Oct 09
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

#include "flock.h"

Flock::Flock(unsigned int flock_id, double vcone_angle, double vcone_radius, unsigned int separation_wt,
      unsigned int cohesion_wt, unsigned int alignment_wt) {
        _flock_id = flock_id;
        _flock_vcone_angle = vcone_angle;
        _flock_vcone_radius = vcone_radius;
        _separation_wt = separation_wt;
        _cohesion_wt = cohesion_wt;
        _alignment_wt = alignment_wt;
}

Flock::~Flock() {
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Function : add_ai_char
// Description : This function adds AI characters to the flock.

/////////////////////////////////////////////////////////////////////////////////////////

void Flock::add_ai_char(AICharacter *ai_char) {
  ai_char->_ai_char_flock_id = _flock_id;
  ai_char->_steering->_flock_group = this;
  _ai_char_list.push_back(ai_char);
}

unsigned int Flock::get_id() {
  return _flock_id;
}
