// Filename: mayaNodeTree.h
// Created by:  drose (06Jun03)
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

#ifndef MAYANODETREE_H
#define MAYANODETREE_H

#include "pandatoolbase.h"

#include "mayaNodeDesc.h"
#include "globPattern.h"

class EggData;
class EggGroupNode;

////////////////////////////////////////////////////////////////////
//       Class : MayaNodeTree
// Description : Describes a complete tree of maya nodes for
//               conversion.
////////////////////////////////////////////////////////////////////
class MayaNodeTree {
public:
  MayaNodeTree();
  MayaNodeDesc *build_node(const MDagPath &dag_path);
  bool build_hierarchy();

  void tag_all();
  bool tag_selected();
  bool tag_named(const GlobPattern &glob);

  int get_num_nodes() const;
  MayaNodeDesc *get_node(int n) const;

  void clear();
  void clear_egg(EggData *egg_data, EggGroupNode *egg_root, 
                 EggGroupNode *skeleton_node);
  EggGroup *get_egg_group(MayaNodeDesc *node_desc);
  EggTable *get_egg_table(MayaNodeDesc *node_desc);
  EggXfmSAnim *get_egg_anim(MayaNodeDesc *node_desc);

  PT(MayaNodeDesc) _root;
  float _fps;

private:
  EggData *_egg_data;
  EggGroupNode *_egg_root;
  EggGroupNode *_skeleton_node;

  MayaNodeDesc *r_build_node(const string &path);

  typedef pmap<string, MayaNodeDesc *> NodesByPath;
  NodesByPath _nodes_by_path;

  typedef pvector<MayaNodeDesc *> Nodes;
  Nodes _nodes;
};

#endif
