// Filename: eggCharacterData.h
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGCHARACTERDATA_H
#define EGGCHARACTERDATA_H

#include <pandatoolbase.h>

#include <eggData.h>
#include <eggNode.h>
#include <pointerTo.h>

class EggJointData;
class EggTable;

////////////////////////////////////////////////////////////////////
// 	 Class : EggCharacterData
// Description : Represents a set of characters, as read and collected
//               from several models and animation files.  Each
//               character is the root of a hierarchy of EggJointData
//               nodes.
////////////////////////////////////////////////////////////////////
class EggCharacterData {
public:
  EggCharacterData();
  virtual ~EggCharacterData();

  bool add_egg(EggData *egg);

  INLINE int get_num_eggs() const;
  INLINE EggData *get_egg(int i) const;

  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual EggJointData *make_joint_data();
  EggJointData *get_root_joint(const string &character_name);

  class EggInfo {
  public:
    PT(EggData) _egg;
    typedef vector<PT(EggNode)> Models;
    Models _models;
  };
  
  typedef vector<EggInfo> Eggs;
  Eggs _eggs;

  class CharacterInfo {
  public:
    string _name;
    EggJointData *_root_joint;
  };

  typedef vector<CharacterInfo> Characters;
  Characters _characters;

private:
  bool scan_hierarchy(EggNode *egg_node);
  void scan_for_top_joints(EggNode *egg_node, EggNode *model_root,
			   const string &character_name);
  void scan_for_top_tables(EggTable *bundle, EggNode *model_root,
			   const string &character_name);

  typedef vector<EggNode *> EggNodeList;
  typedef map<EggNode *, EggNodeList> TopEggNodes;
  typedef map<string, TopEggNodes> TopEggNodesByName;
  TopEggNodesByName _top_egg_nodes;

  void match_egg_nodes(EggJointData *joint_data, EggNodeList &egg_nodes,
		       int egg_index, int model_index);
  void found_egg_node_match(EggJointData *data, EggNode *egg_node,
			    int egg_index, int model_index);
};

#include "eggCharacterData.I"

#endif


