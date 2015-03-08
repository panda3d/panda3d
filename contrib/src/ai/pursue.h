////////////////////////////////////////////////////////////////////////
// Filename    : pursue.h
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

#ifndef _PURSUE_H
#define _PURSUE_H

#include "aiGlobals.h"
#include "aiCharacter.h"

class AICharacter;

class EXPCL_PANDAAI Pursue {

public:
  AICharacter *_ai_char;

  NodePath _pursue_target;
  float _pursue_weight;
  LVecBase3 _pursue_direction;
  bool _pursue_done;

  Pursue(AICharacter *ai_ch, NodePath target_object, float pursue_wt);
  ~Pursue();
  LVecBase3 do_pursue();
};

#endif
