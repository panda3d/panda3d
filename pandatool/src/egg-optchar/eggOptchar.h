/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggOptchar.h
 * @author drose
 * @date 2003-07-18
 */

#ifndef EGGOPTCHAR_H
#define EGGOPTCHAR_H

#include "pandatoolbase.h"

#include "eggCharacterFilter.h"
#include "luse.h"

#include "pvector.h"
#include "vector_string.h"
#include "globPattern.h"

class EggCharacterData;
class EggComponentData;
class EggJointData;
class EggSliderData;
class EggGroupNode;

/**
 * Performs basic optimizations of a character model and its associated
 * animations, by analyzing the animation tables and removing unneeded joints
 * and/or morphs.  Can also be used to restructure the character hierarchy.
 */
class EggOptchar : public EggCharacterFilter {
public:
  EggOptchar();

  void run();

protected:
  virtual bool handle_args(Args &args);

private:
  static bool dispatch_vector_string_pair(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_name_components(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_double_components(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_flag_groups(const std::string &opt, const std::string &arg, void *var);

  void determine_removed_components();
  void move_vertices();
  bool process_joints();
  EggJointData *find_best_parent(EggJointData *joint_data) const;
  EggJointData *find_best_vertex_joint(EggJointData *joint_data) const;

  bool apply_user_reparents();
  bool zero_channels();
  bool quantize_channels();
  void analyze_joints(EggJointData *joint_data, int level);
  void analyze_sliders(EggCharacterData *char_data);
  void list_joints(EggJointData *joint_data, int indent_level, bool verbose);
  void list_joints_p(EggJointData *joint_data, int &col);
  void list_scalars(EggCharacterData *char_data, bool verbose);
  void describe_component(EggComponentData *comp_data, int indent_level,
                          bool verbose);
  void do_reparent();

  void quantize_vertices();
  void quantize_vertices(EggNode *egg_node);
  void quantize_vertex(EggVertex *egg_vertex);

  void do_flag_groups(EggGroupNode *egg_group);
  void rename_joints();
  void rename_primitives(EggGroupNode *egg_group, const std::string &name);
  void change_dart_type(EggGroupNode *egg_group, const std::string &new_dart_type);
  void do_preload();
  void do_defpose();

  bool _list_hierarchy;
  bool _list_hierarchy_v;
  bool _list_hierarchy_p;
  bool _preload;
  bool _keep_all;

  class StringPair {
  public:
    std::string _a;
    std::string _b;
  };
  typedef pvector<StringPair> StringPairs;
  StringPairs _new_joints;
  StringPairs _reparent_joints;
  StringPairs _zero_channels;
  StringPairs _rename_joints;

  vector_string _keep_components;
  vector_string _drop_components;
  vector_string _expose_components;
  vector_string _suppress_components;

  std::string _dart_type;

  class DoubleString {
  public:
    double _a;
    std::string _b;
  };
  typedef pvector<DoubleString> DoubleStrings;
  DoubleStrings _quantize_anims;

  typedef pvector<GlobPattern> Globs;

  class FlagGroupsEntry {
  public:
    Globs _groups;
    std::string _name;
  };
  typedef pvector<FlagGroupsEntry> FlagGroups;
  FlagGroups _flag_groups;

  std::string _defpose;

  bool _optimal_hierarchy;
  double _vref_quantum;
};

#endif
