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
#include "vector_string.h"

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
  static bool dispatch_vector_string_pair(const string &opt, const string &arg, void *var);

  void determine_removed_components();
  bool process_joints();
  EggJointData *find_best_parent(EggJointData *joint_data) const;

  bool apply_user_reparents();
  void analyze_joints(EggJointData *joint_data);
  void analyze_sliders(EggCharacterData *char_data);
  void list_joints(EggJointData *joint_data, int indent_level);
  void list_joints_p(EggJointData *joint_data);
  void list_scalars(EggCharacterData *char_data);
  void describe_component(EggComponentData *comp_data, int indent_level);
  void do_reparent();

  bool _list_hierarchy;
  bool _list_hierarchy_p;
  bool _keep_all;

  class StringPair {
  public:
    string _a;
    string _b;
  };
  typedef pvector<StringPair> StringPairs;
  StringPairs _reparent_joints;

  vector_string _keep_components;
  vector_string _expose_components;
};

#endif

