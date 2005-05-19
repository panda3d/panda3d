// Filename: eggRetargetAnim.h
// Created by:  drose (05May05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef EGGRETARGETANIM_H
#define EGGRETARGETANIM_H

#include "pandatoolbase.h"

#include "eggCharacterFilter.h"
#include "luse.h"
#include "pvector.h"
#include "pset.h"

class EggCharacterData;
class EggJointData;

////////////////////////////////////////////////////////////////////
//       Class : EggRetargetAnim
// Description : Retargets one or more animation files from one
//               particular skeleton to a similar, but differently
//               scaled skeleton by preserving the rotation
//               information but discarding translation and/or scale.
////////////////////////////////////////////////////////////////////
class EggRetargetAnim : public EggCharacterFilter {
public:
  EggRetargetAnim();

  void run();

  void retarget_anim(EggCharacterData *char_data, EggJointData *joint_data,
                     int reference_model, const pset<string> &keep_names);

  Filename _reference_filename;
  vector_string _keep_joints;
};

#endif

