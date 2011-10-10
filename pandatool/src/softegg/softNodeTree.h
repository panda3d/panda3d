// Filename: softNodeTree.h
// Created by:  masad (03Oct03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef SOFTNODETREE_H
#define SOFTNODETREE_H

#include "pandatoolbase.h"
#include "softNodeDesc.h"

#include <SAA.h>

class EggGroup;
class EggTable;
class EggXfmSAnim;
class EggData;
class EggGroupNode;


////////////////////////////////////////////////////////////////////
//       Class : SoftNodeTree
// Description : Describes a complete tree of soft nodes for
//               conversion.
////////////////////////////////////////////////////////////////////
class SoftNodeTree {
public:
  SoftNodeTree();
  SoftNodeDesc *build_node(SAA_Scene *scene, SAA_Elem *model);
  bool build_complete_hierarchy(SAA_Scene &scene, SAA_Database &database);
  void handle_null(SAA_Scene *scene, SoftNodeDesc *node_desc, const char *node_name);
  //  bool build_selected_hierarchy(SAA_Scene *s, SAA_Database *d, char *scene_name);

  int get_num_nodes() const;
  SoftNodeDesc *get_node(int n) const;
  SoftNodeDesc *get_node(string name) const;

  char *GetRootName(const char *);
  char *GetModelNoteInfo(SAA_Scene *, SAA_Elem *);
  char *GetName(SAA_Scene *scene, SAA_Elem *element);
  char *GetFullName(SAA_Scene *scene, SAA_Elem *element);

  EggGroupNode *get_egg_root() {return _egg_root;}
  EggGroup *get_egg_group(SoftNodeDesc *node_desc);
  EggTable *get_egg_table(SoftNodeDesc *node_desc);
  EggXfmSAnim *get_egg_anim(SoftNodeDesc *node_desc);

  void clear_egg(EggData *egg_data, EggGroupNode *egg_root, EggGroupNode *skeleton_node);

  PT(SoftNodeDesc) _root;
  PN_stdfloat _fps;
  int _use_prefix;
  char *_search_prefix;
  

private:

  EggData *_egg_data;
  EggGroupNode *_egg_root;
  EggGroupNode *_skeleton_node;

  SoftNodeDesc *r_build_node(SoftNodeDesc *parent_node, const string &path);

  typedef pmap<string, SoftNodeDesc *> NodesByName;
  NodesByName _nodes_by_name;

  typedef pvector<SoftNodeDesc *> Nodes;
  Nodes _nodes;
};

#endif
