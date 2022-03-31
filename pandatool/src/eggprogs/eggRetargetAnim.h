/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggRetargetAnim.h
 * @author drose
 * @date 2005-05-05
 */

#ifndef EGGRETARGETANIM_H
#define EGGRETARGETANIM_H

#include "pandatoolbase.h"

#include "eggCharacterFilter.h"
#include "luse.h"
#include "pvector.h"
#include "pset.h"

class EggCharacterData;
class EggJointData;
class EggCharacterDb;

/**
 * Retargets one or more animation files from one particular skeleton to a
 * similar, but differently scaled skeleton by preserving the rotation
 * information but discarding translation and/or scale.
 */
class EggRetargetAnim : public EggCharacterFilter {
public:
  EggRetargetAnim();

  void run();

  void retarget_anim(EggCharacterData *char_data, EggJointData *joint_data,
                     int reference_model, const pset<std::string> &keep_names,
                     EggCharacterDb &db);

  Filename _reference_filename;
  vector_string _keep_joints;
};

#endif
