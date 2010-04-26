// Filename: wander.h
// Created by:  Deepak, John, Navin (24Oct09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised
// BSD license.  You should have received a copy of this license
// along with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////
#ifndef WANDER_H
#define WANDER_H

#include "aiCharacter.h"

class AICharacter;

////////////////////////////////////////////////////////////////////
//       Class : Wander
// Description : This class handles all calls to the wander behavior
////////////////////////////////////////////////////////////////////
class Wander {
public:
  AICharacter *_ai_char;
  double _wander_radius;
  LVecBase3f _wander_target;
  float _wander_weight;
  int _flag;
  LVecBase3f _init_pos;
  double _area_of_effect;

  Wander(AICharacter *ai_ch, double wander_radius, int flag, double aoe, float wander_weight);
  LVecBase3f do_wander();
  ~Wander();
};

#endif
