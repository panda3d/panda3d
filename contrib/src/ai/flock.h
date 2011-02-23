////////////////////////////////////////////////////////////////////////
// Filename    : flock.h
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

#ifndef _FLOCK_H
#define _FLOCK_H

#include "aiGlobals.h"
#include "aiCharacter.h"

class AICharacter;

///////////////////////////////////////////////////////////////////////////////////////
//
// Class : Flock
// Description : This class is used to define the flock attributes and the AI characters
//                which are part of the flock.

///////////////////////////////////////////////////////////////////////////////////////

class EXPCL_PANDAAI Flock {
private:
  unsigned int _flock_id;

public:
  // Variables which will hold the parameters of the ai character's visibilty cone.
  double _flock_vcone_angle;
  double _flock_vcone_radius;

  // Variables to specify weights of separation, cohesion and alignment behaviors and thus
  // create variable flock behavior.
  unsigned int _separation_wt;
  unsigned int _cohesion_wt;
  unsigned int _alignment_wt;

  // This vector will hold all the ai characters which belong to this flock.
  typedef std::vector<AICharacter*> AICharList;
  AICharList _ai_char_list;

PUBLISHED:
  Flock(unsigned int flock_id, double vcone_angle, double vcone_radius, unsigned int separation_wt = 2,
    unsigned int cohesion_wt = 4, unsigned int alignment_wt = 1);
  ~Flock();

  // Function to add the ai characters to _ai_char_list.
  void add_ai_char(AICharacter *ai_char);

  // Function to access the private member flock_id.
  unsigned int get_id();
};

#endif
