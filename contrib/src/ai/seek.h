// Filename: seek.h
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

#ifndef SEEK_H
#define SEEK_H

#include "aiGlobals.h"
#include "aiCharacter.h"

class AICharacter;

////////////////////////////////////////////////////////////////////
//       Class : Seek
// Description : This class handles all calls to the seek behavior
////////////////////////////////////////////////////////////////////
class Seek {

public:
  AICharacter *_ai_char;

  LVecBase3f _seek_position;
  float _seek_weight;
  LVecBase3f _seek_direction;
  bool _seek_done;
  LVecBase3f _seek_accum_force;

  Seek(AICharacter *ai_ch, NodePath target_object, float seek_wt = 1.0);
  Seek(AICharacter *ai_ch, LVecBase3f pos, float seek_wt = 1.0);
  ~Seek();
  LVecBase3f do_seek();
};

#endif
