// Filename: eggCharacterCollection.h
// Created by:  drose (26Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGCHARACTERCOLLECTION_H
#define EGGCHARACTERCOLLECTION_H

#include <pandatoolbase.h>

#include "eggCharacterData.h"

#include <eggData.h>
#include <eggNode.h>
#include <pointerTo.h>

class EggTable;
class EggAttributes;

////////////////////////////////////////////////////////////////////
// 	 Class : EggCharacterCollection
// Description : Represents a set of characters, as read and collected
//               from possibly several model and/or animation egg
//               files.
////////////////////////////////////////////////////////////////////
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
  EggCharacterData *get_character_by_name(const string &character_name) const;

  INLINE EggCharacterData *get_character_by_model_index(int model_index) const;

  virtual void write(ostream &out, int indent_level = 0) const;

  virtual EggCharacterData *make_character_data();
  virtual EggJointData *make_joint_data(EggCharacterData *char_data);
  virtual EggSliderData *make_slider_data(EggCharacterData *char_data);

public:
  EggCharacterData *make_character(const string &character_name);

  class EggInfo {
  public:
    PT(EggData) _egg;
    typedef vector<PT(EggNode)> Models;
    Models _models;
    int _first_model_index;
  };
  
  typedef vector<EggInfo> Eggs;
  Eggs _eggs;

  typedef vector<EggCharacterData *> Characters;
  Characters _characters;
  Characters _characters_by_model_index;

private:
  bool scan_hierarchy(EggNode *egg_node);
  void scan_for_top_joints(EggNode *egg_node, EggNode *model_root,
			   const string &character_name);
  void scan_for_top_tables(EggTable *bundle, EggNode *model_root,
			   const string &character_name);
  void scan_for_morphs(EggNode *egg_node, int model_index,
		       EggCharacterData *char_data);
  void scan_for_sliders(EggNode *egg_node, int model_index,
			EggCharacterData *char_data);

  void add_morph_back_pointers(EggAttributes *attrib, EggObject *egg_object,
			       int model_index, EggCharacterData *char_data);

  typedef vector<EggNode *> EggNodeList;
  typedef map<EggNode *, EggNodeList> TopEggNodes;
  typedef map<string, TopEggNodes> TopEggNodesByName;
  TopEggNodesByName _top_egg_nodes;

  int _next_model_index;

  void match_egg_nodes(EggCharacterData *char_Data, EggJointData *joint_data,
		       EggNodeList &egg_nodes, int egg_index, int model_index);
  void found_egg_match(EggCharacterData *char_data, EggJointData *joint_data,
		       EggNode *egg_node, int egg_index, int model_index);
};

#include "eggCharacterCollection.I"

#endif


