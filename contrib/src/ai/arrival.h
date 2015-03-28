////////////////////////////////////////////////////////////////////////
// Filename    : arrival.h
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

#ifndef _ARRIVAL_H
#define _ARRIVAL_H

#include "aiGlobals.h"
#include "aiCharacter.h"

class AICharacter;

class EXPCL_PANDAAI Arrival {

public:
  AICharacter *_ai_char;

  NodePath _arrival_target;
  LVecBase3 _arrival_target_pos;
  double _arrival_distance;
  LVecBase3 _arrival_direction;
  bool _arrival_done;

  // This flag specifies if the arrival behavior is being used with seek or pursue behavior.
  // True = used with pursue.
  // False = used with seek.
  bool _arrival_type;

  Arrival(AICharacter *ai_ch, double distance = 10.0);
  ~Arrival();
  LVecBase3 do_arrival();
  void arrival_activate();
};

#endif
