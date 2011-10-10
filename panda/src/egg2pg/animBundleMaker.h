// Filename: animBundleMaker.h
// Created by:  drose (22Feb99)
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

#ifndef ANIMBUNDLEMAKER_H
#define ANIMBUNDLEMAKER_H

#include "pandabase.h"
#include "typedef.h"

class EggNode;
class EggGroupNode;
class EggTable;
class EggXfmSAnim;
class EggSAnimData;
class AnimGroup;
class AnimBundle;
class AnimBundleNode;
class AnimChannelScalarTable;
class AnimChannelMatrixXfmTable;

////////////////////////////////////////////////////////////////////
//       Class : AnimBundleMaker
// Description : Converts an EggTable hierarchy, beginning with a
//               <Bundle> entry, into an AnimBundle hierarchy.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG AnimBundleMaker {
public:
  AnimBundleMaker(EggTable *root);

  AnimBundleNode *make_node();

private:
  AnimBundle *make_bundle();

  void inspect_tree(EggNode *node);
  void build_hierarchy(EggTable *egg_table, AnimGroup *parent);

  AnimChannelScalarTable *
  create_s_channel(EggSAnimData *egg_anim, const string &name,
                   AnimGroup *parent);
  AnimChannelMatrixXfmTable *
  create_xfm_channel(EggNode *egg_node, const string &name,
                     AnimGroup *parent);
  AnimChannelMatrixXfmTable *
  create_xfm_channel(EggXfmSAnim *egg_anim, const string &name,
                     AnimGroup *parent);

  PN_stdfloat _fps;
  int _num_frames;
  bool _ok_fps;
  bool _ok_num_frames;

  EggTable *_root;

};

#endif
