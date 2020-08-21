/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file flock.h
 * @author Deepak, John, Navin
 * @date 2009-10-24
 */

#ifndef _FLOCK_H
#define _FLOCK_H

#include "aiGlobals.h"
#include "aiCharacter.h"

class AICharacter;

/**
 * This class is used to define the flock attributes and the AI characters
 * which are part of the flock.
 */
class EXPCL_PANDAAI Flock {
private:
  unsigned int _flock_id;

public:
  // Variables which will hold the parameters of the ai character's visibilty
  // cone.
  double _flock_vcone_angle;
  double _flock_vcone_radius;

  // Variables to specify weights of separation, cohesion and alignment
  // behaviors and thus create variable flock behavior.
  unsigned int _separation_wt;
  unsigned int _cohesion_wt;
  unsigned int _alignment_wt;

  // This vector will hold all the ai characters which belong to this flock.
  typedef std::vector<PT(AICharacter)> AICharList;
  AICharList _ai_char_list;

PUBLISHED:
  explicit Flock(unsigned int flock_id, double vcone_angle, double vcone_radius, unsigned int separation_wt = 2,
    unsigned int cohesion_wt = 4, unsigned int alignment_wt = 1);
  ~Flock();

  // Function to add the ai characters to _ai_char_list.
  void add_ai_char(AICharacter *ai_char);

  // Function to access the private member flock_id.
  unsigned int get_id();
};

#endif
