/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggCharacterCollection.h
 * @author drose
 * @date 2001-02-26
 */

#ifndef EGGCHARACTERCOLLECTION_H
#define EGGCHARACTERCOLLECTION_H

#include "pandatoolbase.h"

#include "eggCharacterData.h"

#include "eggData.h"
#include "eggNode.h"
#include "pointerTo.h"

class EggTable;
class EggAttributes;

/**
 * Represents a set of characters, as read and collected from possibly several
 * model and/or animation egg files.
 */
class EggCharacterCollection {
public:
  EggCharacterCollection();
  virtual ~EggCharacterCollection();

  int add_egg(EggData *egg);

  INLINE int get_num_eggs() const;
  INLINE EggData *get_egg(int i) const;
  INLINE int get_first_model_index(int egg_index) const;
  INLINE int get_num_models(int egg_index) const;

  INLINE int get_num_characters() const;
  INLINE EggCharacterData *get_character(int i) const;
  EggCharacterData *get_character_by_name(const std::string &character_name) const;

  INLINE EggCharacterData *get_character_by_model_index(int model_index) const;

  void rename_char(int i, const std::string &name);

  virtual void write(std::ostream &out, int indent_level = 0) const;
  void check_errors(std::ostream &out, bool force_initial_rest_frame);

  virtual EggCharacterData *make_character_data();
  virtual EggJointData *make_joint_data(EggCharacterData *char_data);
  virtual EggSliderData *make_slider_data(EggCharacterData *char_data);

public:
  EggCharacterData *make_character(const std::string &character_name);

  class EggInfo {
  public:
    PT(EggData) _egg;
    typedef pvector< PT(EggNode) > Models;
    Models _models;
    int _first_model_index;
  };

  typedef pvector<EggInfo> Eggs;
  Eggs _eggs;

  typedef pvector<EggCharacterData *> Characters;
  Characters _characters;
  Characters _characters_by_model_index;

private:
  bool scan_hierarchy(EggNode *egg_node);
  void scan_for_top_joints(EggNode *egg_node, EggNode *model_root,
                           const std::string &character_name);
  void scan_for_top_tables(EggTable *bundle, EggNode *model_root,
                           const std::string &character_name);
  void scan_for_morphs(EggNode *egg_node, int model_index,
                       EggCharacterData *char_data);
  void scan_for_sliders(EggNode *egg_node, int model_index,
                        EggCharacterData *char_data);

  void add_morph_back_pointers(EggAttributes *attrib, EggObject *egg_object,
                               int model_index, EggCharacterData *char_data);
  void add_morph_back_pointers_vertex(EggVertex *vertex, EggObject *egg_object,
                                      int model_index, EggCharacterData *char_data);

  // The _top_egg_nodes member is only used temporarily, when adding each pre-
  // existing egg file to the structure for the first time.
  typedef pvector<EggNode *> EggNodeList;
  class ModelDescription {
  public:
    INLINE ModelDescription();
    EggNodeList _top_nodes;
    EggObject *_root_node;
  };

  typedef pmap<EggNode *, ModelDescription> TopEggNodes;
  typedef pmap<std::string, TopEggNodes> TopEggNodesByName;
  TopEggNodesByName _top_egg_nodes;

  int _next_model_index;

  void match_egg_nodes(EggCharacterData *char_Data, EggJointData *joint_data,
                       EggNodeList &egg_nodes, int egg_index, int model_index);
  void found_egg_match(EggCharacterData *char_data, EggJointData *joint_data,
                       EggNode *egg_node, int egg_index, int model_index);
};

#include "eggCharacterCollection.I"

#endif
