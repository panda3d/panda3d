// Filename: softNodeTree.h
// Created by:  masad (03Oct03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2003, Disney Enterprises, Inc.  All rights reserved
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
  SoftNodeDesc *build_node(SAA_Elem *model, const char *name);
  bool build_complete_hierarchy(SAA_Scene &scene, SAA_Database &database, char **root_name);
  //  bool build_selected_hierarchy(SAA_Scene *s, SAA_Database *d, char *scene_name);

  int get_num_nodes() const;
  SoftNodeDesc *get_node(int n) const;

  char *GetName(SAA_Scene *scene, SAA_Elem *element);
  char *GetFullName(SAA_Scene *scene, SAA_Elem *element);

  void clear_egg(EggData *egg_data, EggGroupNode *egg_root, 
                 EggGroupNode *skeleton_node);
  EggGroup *get_egg_group(SoftNodeDesc *node_desc);
  EggTable *get_egg_table(SoftNodeDesc *node_desc);
  EggXfmSAnim *get_egg_anim(SoftNodeDesc *node_desc);

  PT(SoftNodeDesc) _root;
  float _fps;

private:

  EggData *_egg_data;
  EggGroupNode *_egg_root;
  EggGroupNode *_skeleton_node;

  SoftNodeDesc *r_build_node(const string &path);
#if 0
  typedef pmap<string, SoftNodeDesc *> NodesByPath;
  NodesByPath _nodes_by_path;
#endif
  typedef pvector<SoftNodeDesc *> Nodes;
  Nodes _nodes;
};

#endif
