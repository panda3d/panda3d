/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file evade.h
 * @author Deepak, John, Navin
 * @date 2009-10-24
 */

#ifndef _EVADE_H
#define _EVADE_H

#include "aiGlobals.h"
#include "aiCharacter.h"

class AICharacter;

class EXPCL_PANDAAI Evade {
public:
  AICharacter *_ai_char;

  NodePath _evade_target;
  float _evade_weight;
  LVecBase3 _evade_direction;
  double _evade_distance;
  double _evade_relax_distance;
  bool _evade_done;
  bool _evade_activate_done;

  Evade(AICharacter *ai_ch, NodePath target_object, double panic_distance,
                                          double relax_distance, float evade_wt);

  ~Evade();
  LVecBase3 do_evade();
  void evade_activate();
};

#endif
