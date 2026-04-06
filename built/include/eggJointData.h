/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggJointData.h
 * @author drose
 * @date 2001-02-23
 */

#ifndef EGGJOINTDATA_H
#define EGGJOINTDATA_H

#include "pandatoolbase.h"
#include "eggComponentData.h"
#include "eggGroup.h"
#include "luse.h"
#include "pset.h"

class EggCharacterDb;

/**
 * This is one node of a hierarchy of EggJointData nodes, each of which
 * represents a single joint of the character hierarchy across all loaded
 * files: the various models, the LOD's of each model, and the various
 * animation channel files.
 */
class EggJointData : public EggComponentData {
public:
  EggJointData(EggCharacterCollection *collection,
               EggCharacterData *char_data);

  INLINE EggJointData *get_parent() const;
  INLINE int get_num_children() const;
  INLINE EggJointData *get_child(int n) const;
  INLINE EggJointData *find_joint(const std::string &name);

  LMatrix4d get_frame(int model_index, int n) const;
  LMatrix4d get_net_frame(int model_index, int n, EggCharacterDb &db) const;
  LMatrix4d get_net_frame_inv(int model_index, int n, EggCharacterDb &db) const;

  INLINE bool has_rest_frame() const;
  INLINE bool rest_frames_differ() const;
  INLINE const LMatrix4d &get_rest_frame() const;
  void force_initial_rest_frame();

  INLINE void reparent_to(EggJointData *new_parent);
  void move_vertices_to(EggJointData *new_owner);
  int score_reparent_to(EggJointData *new_parent, EggCharacterDb &db);

  bool do_rebuild_all(EggCharacterDb &db);
  void optimize();
  void expose(EggGroup::DCSType dcs_type = EggGroup::DC_default);
  void zero_channels(const std::string &components);
  void quantize_channels(const std::string &components, double quantum);
  void apply_default_pose(int source_model, int frame);

  virtual void add_back_pointer(int model_index, EggObject *egg_object);
  virtual void write(std::ostream &out, int indent_level = 0) const;

protected:
  void do_begin_reparent();
  bool calc_new_parent_depth(pset<EggJointData *> &chain);
  void do_begin_compute_reparent();
  bool do_compute_reparent(int model_index, int n, EggCharacterDb &db);
  bool do_joint_rebuild(int model_index, EggCharacterDb &db);
  void do_finish_reparent();

private:
  EggJointData *make_new_joint(const std::string &name);
  EggJointData *find_joint_exact(const std::string &name);
  EggJointData *find_joint_matches(const std::string &name);

  bool is_new_ancestor(EggJointData *child) const;
  const LMatrix4d &get_new_net_frame(int model_index, int n, EggCharacterDb &db);
  const LMatrix4d &get_new_net_frame_inv(int model_index, int n, EggCharacterDb &db);
  LMatrix4d get_new_frame(int model_index, int n, EggCharacterDb &db);

  bool _has_rest_frame;
  bool _rest_frames_differ;
  LMatrix4d _rest_frame;

  // These are used to cache the above results for optimizing
  // do_compute_reparent().
  LMatrix4d _new_net_frame, _new_net_frame_inv;
  bool _got_new_net_frame, _got_new_net_frame_inv;
  bool _computed_reparent;
  bool _computed_ok;

protected:
  typedef pvector<EggJointData *> Children;
  Children _children;
  EggJointData *_parent;
  EggJointData *_new_parent;
  int _new_parent_depth;
  bool _got_new_parent_depth;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggComponentData::init_type();
    register_type(_type_handle, "EggJointData",
                  EggComponentData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class EggCharacterCollection;
  friend class EggCharacterData;
  friend class OrderJointsByNewDepth;
};

#include "eggJointData.I"

#endif
