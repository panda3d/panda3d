// Filename: eggTopstrip.h
// Created by:  drose (23Feb01)
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

#ifndef EGGTOPSTRIP_H
#define EGGTOPSTRIP_H

#include "pandatoolbase.h"

#include "eggCharacterFilter.h"
#include "luse.h"

#include "pvector.h"

class EggCharacterData;
class EggJointData;
class EggJointPointer;

////////////////////////////////////////////////////////////////////
//       Class : EggTopstrip
// Description : Reads a character model and/or animations and strips
//               out the animation from one of the top joints from the
//               entire character.  Particularly useful for generating
//               stackable character models from separately-extracted
//               characters.
////////////////////////////////////////////////////////////////////
class EggTopstrip : public EggCharacterFilter {
public:
  EggTopstrip();

  void run();
  void check_transform_channels();

  void strip_anim(EggCharacterData *char_data, EggJointData *joint_data,
                  int from_model, EggCharacterData *from_char,
                  EggJointData *top_joint);
  void strip_anim_vertices(EggNode *egg_node, int into_model,
                           int from_model, EggJointData *top_joint);

  void adjust_transform(LMatrix4d &mat) const;


  string _top_joint_name;
  bool _got_invert_transform;
  bool _invert_transform;
  string _transform_channels;
  Filename _channel_filename;
};

#endif

