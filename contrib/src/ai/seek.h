////////////////////////////////////////////////////////////////////////
// Filename    : seek.h
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

#ifndef _SEEK_H
#define _SEEK_H

#include "aiGlobals.h"
#include "aiCharacter.h"

class AICharacter;

class EXPCL_PANDAAI Seek {

public:
  AICharacter *_ai_char;

  LVecBase3 _seek_position;
  float _seek_weight;
  LVecBase3 _seek_direction;
  bool _seek_done;
  LVecBase3 _seek_accum_force;

  Seek(AICharacter *ai_ch, NodePath target_object, float seek_wt = 1.0);
  Seek(AICharacter *ai_ch, LVecBase3 pos, float seek_wt = 1.0);
  ~Seek();
  LVecBase3 do_seek();
};

#endif
