/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animBundleMaker.h
 * @author drose
 * @date 1999-02-22
 */

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

/**
 * Converts an EggTable hierarchy, beginning with a <Bundle> entry, into an
 * AnimBundle hierarchy.
 */
class EXPCL_PANDA_EGG2PG AnimBundleMaker {
public:
  explicit AnimBundleMaker(EggTable *root);

  AnimBundleNode *make_node();

private:
  AnimBundle *make_bundle();

  void inspect_tree(EggNode *node);
  void build_hierarchy(EggTable *egg_table, AnimGroup *parent);

  AnimChannelScalarTable *
  create_s_channel(EggSAnimData *egg_anim, const std::string &name,
                   AnimGroup *parent);
  AnimChannelMatrixXfmTable *
  create_xfm_channel(EggNode *egg_node, const std::string &name,
                     AnimGroup *parent);
  AnimChannelMatrixXfmTable *
  create_xfm_channel(EggXfmSAnim *egg_anim, const std::string &name,
                     AnimGroup *parent);

  PN_stdfloat _fps;
  int _num_frames;
  bool _ok_fps;
  bool _ok_num_frames;

  EggTable *_root;

};

#endif
