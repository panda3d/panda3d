/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggCharacterData.h
 * @author drose
 * @date 2001-02-23
 */

#ifndef EGGCHARACTERDATA_H
#define EGGCHARACTERDATA_H

#include "pandatoolbase.h"

#include "eggJointData.h"
#include "eggNode.h"
#include "eggData.h"
#include "pointerTo.h"
#include "namable.h"
#include "nameUniquifier.h"

#include "pmap.h"

class EggCharacterCollection;
class EggSliderData;
class EggCharacterDb;

/**
 * Represents a single character, as read and collected from several models
 * and animation files.  This contains a hierarchy of EggJointData nodes
 * representing the skeleton, as well as a list of EggSliderData nodes
 * representing the morph channels for the character.
 *
 * This is very similar to the Character class from Panda, in that it's
 * capable of associating skeleton-morph animation channels with models and
 * calculating the vertex position for each frame.  To some degree, it
 * duplicates the functionality of Character.  However, it differs in one
 * fundamental principle: it is designed to be a non-real-time operation,
 * working directly on the Egg structures as they are, instead of first
 * boiling the Egg data into native Panda Geom tables for real-time animation.
 * Because of this, it is (a) double-precision instead of single precision,
 * (b) capable of generating modified Egg files, and (c) about a hundred times
 * slower than the Panda Character class.
 *
 * The data in this structure is normally filled in by the
 * EggCharacterCollection class.
 */
class EggCharacterData : public Namable {
public:
  EggCharacterData(EggCharacterCollection *collection);
  virtual ~EggCharacterData();

  void rename_char(const std::string &name);

  void add_model(int model_index, EggNode *model_root, EggData *egg_data);
  INLINE int get_num_models() const;
  INLINE int get_model_index(int n) const;
  INLINE EggNode *get_model_root(int n) const;
  INLINE EggData *get_egg_data(int n) const;
  int get_num_frames(int model_index) const;
  bool check_num_frames(int model_index);
  double get_frame_rate(int model_index) const;

  INLINE EggJointData *get_root_joint() const;
  INLINE EggJointData *find_joint(const std::string &name) const;
  INLINE EggJointData *make_new_joint(const std::string &name, EggJointData *parent);
  INLINE int get_num_joints() const;
  INLINE EggJointData *get_joint(int n) const;

  bool do_reparent();
  void choose_optimal_hierarchy();

  INLINE int get_num_sliders() const;
  INLINE EggSliderData *get_slider(int n) const;
  EggSliderData *find_slider(const std::string &name) const;
  EggSliderData *make_slider(const std::string &name);

  INLINE int get_num_components() const;
  INLINE EggComponentData *get_component(int n) const;

  size_t estimate_db_size() const;

  virtual void write(std::ostream &out, int indent_level = 0) const;

private:
  class Model {
  public:
    int _model_index;
    PT(EggNode) _model_root;
    PT(EggData) _egg_data;
  };
  typedef pvector<Model> Models;
  Models _models;

  EggCharacterCollection *_collection;
  EggJointData *_root_joint;

  typedef pmap<std::string, EggSliderData *> SlidersByName;
  SlidersByName _sliders_by_name;

  typedef pvector<EggSliderData *> Sliders;
  Sliders _sliders;

  typedef pvector<EggJointData *> Joints;
  Joints _joints;

  typedef pvector<EggComponentData *> Components;
  Components _components;

  NameUniquifier _component_names;

  friend class EggCharacterCollection;
};

#include "eggCharacterData.I"

#endif
