// Filename: eggOptchar.h
// Created by:  drose (18Jul03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef EGGOPTCHAR_H
#define EGGOPTCHAR_H

#include "pandatoolbase.h"

#include "eggCharacterFilter.h"
#include "luse.h"

#include "pvector.h"

class EggCharacterData;
class EggComponentData;
class EggJointData;
class EggSliderData;

////////////////////////////////////////////////////////////////////
//       Class : EggOptchar
// Description : Performs basic optimizations of a character model and
//               its associated animations, by analyzing the animation
//               tables and removing unneeded joints and/or morphs.
//               Can also be used to restructure the character
//               hierarchy.
////////////////////////////////////////////////////////////////////
class EggOptchar : public EggCharacterFilter {
public:
  EggOptchar();

  void run();

protected:
  virtual bool handle_args(Args &args);

private:
  void analyze_joints(EggJointData *joint_data);
  void analyze_sliders(EggCharacterData *char_data);
  void list_joints(EggJointData *joint_data, int indent_level);
  void list_scalars(EggCharacterData *char_data);
  void describe_component(EggComponentData *comp_data, int indent_level);

  bool _list_hierarchy;
};

#endif

