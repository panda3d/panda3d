////////////////////////////////////////////////////////////////////////
// Filename    : wander.h
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

#ifndef _WANDER_H
#define _WANDER_H

#include "aiCharacter.h"

class AICharacter;

class EXPCL_PANDAAI Wander {
  public:
    AICharacter *_ai_char;
    double _wander_radius;
    LVecBase3 _wander_target;
    float _wander_weight;
    int _flag;
    LVecBase3 _init_pos;
    double _area_of_effect;

    Wander(AICharacter *ai_ch, double wander_radius, int flag, double aoe, float wander_weight);
    LVecBase3 do_wander();
    ~Wander();
};

#endif
