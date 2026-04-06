/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTopstrip.h
 * @author drose
 * @date 2001-02-23
 */

#ifndef EGGTOPSTRIP_H
#define EGGTOPSTRIP_H

#include "pandatoolbase.h"

#include "eggCharacterFilter.h"
#include "luse.h"

#include "pvector.h"

class EggCharacterData;
class EggCharacterDb;
class EggJointData;
class EggJointPointer;

/**
 * Reads a character model and/or animations and strips out the animation from
 * one of the top joints from the entire character.  Particularly useful for
 * generating stackable character models from separately-extracted characters.
 */
class EggTopstrip : public EggCharacterFilter {
public:
  EggTopstrip();

  void run();
  void check_transform_channels();

  void strip_anim(EggCharacterData *char_data, EggJointData *joint_data,
                  int from_model, EggCharacterData *from_char,
                  EggJointData *top_joint, EggCharacterDb &db);
  void strip_anim_vertices(EggNode *egg_node, int into_model,
                           int from_model, EggJointData *top_joint,
                           EggCharacterDb &db);

  void adjust_transform(LMatrix4d &mat) const;


  std::string _top_joint_name;
  bool _got_invert_transform;
  bool _invert_transform;
  std::string _transform_channels;
  Filename _channel_filename;
};

#endif
