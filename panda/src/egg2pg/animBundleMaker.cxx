/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animBundleMaker.cxx
 * @author drose
 * @date 1999-02-22
 */

#include "animBundleMaker.h"
#include "config_egg2pg.h"

#include "eggTable.h"
#include "eggAnimData.h"
#include "eggSAnimData.h"
#include "eggXfmAnimData.h"
#include "eggXfmSAnim.h"
#include "eggGroupNode.h"
#include "animBundle.h"
#include "animBundleNode.h"
#include "animChannelMatrixXfmTable.h"
#include "animChannelScalarTable.h"

using std::min;

/**
 *
 */
AnimBundleMaker::
AnimBundleMaker(EggTable *root) : _root(root) {
  _fps = 0.0f;
  _num_frames = 1;

  _ok_fps = true;
  _ok_num_frames = true;

  inspect_tree(root);

  if (!_ok_fps) {
    egg2pg_cat.warning()
      << "AnimBundle " << _root->get_name()
      << " specifies contradictory frame rates.\n";
  } else if (_fps == 0.0f) {
    egg2pg_cat.warning()
      << "AnimBundle " << _root->get_name()
      << " does not specify a frame rate.\n";
    _fps = 24.0f;
  }

  if (!_ok_num_frames) {
    egg2pg_cat.warning()
      << "AnimBundle " << _root->get_name()
      << " specifies contradictory number of frames.\n";
  }
}


/**
 *
 */
AnimBundleNode *AnimBundleMaker::
make_node() {
  return new AnimBundleNode(_root->get_name(), make_bundle());
}

/**
 *
 */
AnimBundle *AnimBundleMaker::
make_bundle() {
  AnimBundle *bundle = new AnimBundle(_root->get_name(), _fps, _num_frames);

  EggTable::const_iterator ci;
  for (ci = _root->begin(); ci != _root->end(); ++ci) {
    if ((*ci)->is_of_type(EggTable::get_class_type())) {
      EggTable *child = DCAST(EggTable, *ci);
      build_hierarchy(child, bundle);
    }
  }

  bundle->sort_descendants();

  return bundle;
}


/**
 * Walks the egg tree, getting out the fps and the number of frames.
 */
void AnimBundleMaker::
inspect_tree(EggNode *egg_node) {
  if (egg_node->is_of_type(EggAnimData::get_class_type())) {
    // Check frame rate.
    EggAnimData *egg_anim = DCAST(EggAnimData, egg_node);
    if (egg_anim->has_fps()) {
      if (_fps == 0.0f) {
        _fps = egg_anim->get_fps();
      } else if (_fps != egg_anim->get_fps()) {
        // Whoops!  This table differs in opinion from the other tables.
        _fps = min(_fps, (PN_stdfloat)egg_anim->get_fps());
        _ok_fps = false;
      }
    }
  }

  if (egg_node->is_of_type(EggXfmSAnim::get_class_type())) {
    // Check frame rate.
    EggXfmSAnim *egg_anim = DCAST(EggXfmSAnim, egg_node);
    if (egg_anim->has_fps()) {
      if (_fps == 0.0f) {
        _fps = egg_anim->get_fps();
      } else if (_fps != egg_anim->get_fps()) {
        // Whoops!  This table differs in opinion from the other tables.
        _fps = min(_fps, (PN_stdfloat)egg_anim->get_fps());
        _ok_fps = false;
      }
    }
  }

  if (egg_node->is_of_type(EggSAnimData::get_class_type())) {
    // Check number of frames.
    EggSAnimData *egg_anim = DCAST(EggSAnimData, egg_node);
    int num_frames = egg_anim->get_num_rows();

    if (num_frames > 1) {
      if (_num_frames == 1) {
        _num_frames = num_frames;
      } else if (_num_frames != num_frames) {
        // Whoops!  Another disagreement.
        _num_frames = min(_num_frames, num_frames);
        _ok_num_frames = false;
      }
    }
  }

  if (egg_node->is_of_type(EggXfmAnimData::get_class_type())) {
    // Check number of frames.
    EggXfmAnimData *egg_anim = DCAST(EggXfmAnimData, egg_node);
    int num_frames = egg_anim->get_num_rows();

    if (num_frames > 1) {
      if (_num_frames == 1) {
        _num_frames = num_frames;
      } else if (_num_frames != num_frames) {
        // Whoops!  Another disagreement.
        _num_frames = min(_num_frames, num_frames);
        _ok_num_frames = false;
      }
    }
  }

  if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    // Now recurse.
    EggGroupNode *group = DCAST(EggGroupNode, egg_node);
    EggGroupNode::const_iterator ci;
    for (ci = group->begin(); ci != group->end(); ++ci) {
      inspect_tree(*ci);
    }
  }
}


/**
 * Walks the egg tree again, creating the AnimChannels as appropriate.
 */
void AnimBundleMaker::
build_hierarchy(EggTable *egg_table, AnimGroup *parent) {
  AnimGroup *this_node = nullptr;

  // First, scan the children of egg_table for anim data tables.  If any of
  // them is named "xform", it's a special case--this one stands for the
  // egg_table node itself.  Don't ask me why.

  EggTable::const_iterator ci;
  for (ci = egg_table->begin(); ci != egg_table->end(); ++ci) {
    if ((*ci)->get_name() == "xform") {
      if (this_node == nullptr) {
        this_node = create_xfm_channel((*ci), egg_table->get_name(), parent);
      } else {
        egg2pg_cat.warning()
          << "Duplicate xform table under node "
          << egg_table->get_name() << "\n";
      }
    }
  }

  // If none of them were named "xform", just create a plain old AnimGroup.
  if (this_node == nullptr) {
    this_node = new AnimGroup(parent, egg_table->get_name());
  }

  // Now walk the children again, creating any leftover tables, and recursing.
  for (ci = egg_table->begin(); ci != egg_table->end(); ++ci) {
    if ((*ci)->get_name() == "xform") {
      // Skip this one.  We already got it.
    } else if ((*ci)->is_of_type(EggSAnimData::get_class_type())) {
      EggSAnimData *egg_anim = DCAST(EggSAnimData, *ci);
      create_s_channel(egg_anim, egg_anim->get_name(), this_node);

    } else if ((*ci)->is_of_type(EggTable::get_class_type())) {
      EggTable *child = DCAST(EggTable, *ci);
      build_hierarchy(child, this_node);
    }
  }
}


/**
 * Creates an AnimChannelScalarTable corresponding to the given EggSAnimData
 * structure.
 */
AnimChannelScalarTable *AnimBundleMaker::
create_s_channel(EggSAnimData *egg_anim, const std::string &name,
                 AnimGroup *parent) {
  AnimChannelScalarTable *table
    = new AnimChannelScalarTable(parent, name);

  // First we have to copy the table data from PTA_double to PTA_stdfloat.
  PTA_stdfloat new_data = PTA_stdfloat::empty_array(egg_anim->get_num_rows(),
                                                    table->get_class_type());
  for (int i = 0; i < egg_anim->get_num_rows(); i++) {
    new_data[i] = (PN_stdfloat)egg_anim->get_value(i);
  }

  // Now we can assign the table.
  table->set_table(new_data);

  return table;
}


/**
 * Creates an AnimChannelMatrixXfmTable corresponding to the given EggNode
 * structure, if possible.
 */
AnimChannelMatrixXfmTable *AnimBundleMaker::
create_xfm_channel(EggNode *egg_node, const std::string &name,
                   AnimGroup *parent) {
  if (egg_node->is_of_type(EggXfmAnimData::get_class_type())) {
    EggXfmAnimData *egg_anim = DCAST(EggXfmAnimData, egg_node);
    EggXfmSAnim new_anim(*egg_anim);
    return create_xfm_channel(&new_anim, name, parent);

  } else if (egg_node->is_of_type(EggXfmSAnim::get_class_type())) {
    EggXfmSAnim *egg_anim = DCAST(EggXfmSAnim, egg_node);
    return create_xfm_channel(egg_anim, name, parent);
  }

  egg2pg_cat.warning()
    << "Inappropriate node named xform under node "
    << name << "\n";
  return nullptr;
}


/**
 * Creates an AnimChannelMatrixXfmTable corresponding to the given EggXfmSAnim
 * structure.
 */
AnimChannelMatrixXfmTable *AnimBundleMaker::
create_xfm_channel(EggXfmSAnim *egg_anim, const std::string &name,
                   AnimGroup *parent) {
  // Ensure that the anim table is optimal and that it is standard order.
  egg_anim->optimize_to_standard_order();

  AnimChannelMatrixXfmTable *table
    = new AnimChannelMatrixXfmTable(parent, name);

  // The EggXfmSAnim structure has a number of children which are EggSAnimData
  // tables.  Each of these represents a separate component of the transform
  // data, and will be added to the table.

  EggXfmSAnim::const_iterator ci;
  for (ci = egg_anim->begin(); ci != egg_anim->end(); ++ci) {
    if ((*ci)->is_of_type(EggSAnimData::get_class_type())) {
      EggSAnimData *child = DCAST(EggSAnimData, *ci);

      if (child->get_name().empty()) {
        egg2pg_cat.warning()
          << "Unnamed subtable of <Xfm$Anim_S$> " << name
          << "\n";
      } else {
        char table_id = child->get_name()[0];

        if (child->get_name().length() > 1 ||
            !table->is_valid_id(table_id)) {
          egg2pg_cat.warning()
            << "Unexpected table name " << child->get_name()
            << ", child of " << name << "\n";

        } else if (table->has_table(table_id)) {
          egg2pg_cat.warning()
            << "Duplicate table definition for " << table_id
            << " under " << name << "\n";

        } else {

          // Now we have to copy the table data from PTA_double to
          // PTA_stdfloat.
          PTA_stdfloat new_data=PTA_stdfloat::empty_array(child->get_num_rows(),
                                                    table->get_class_type());
          for (int i = 0; i < child->get_num_rows(); i++) {
            new_data[i] = (PN_stdfloat)child->get_value(i);
          }

          // Now we can assign the table.
          table->set_table(table_id, new_data);
        }
      }
    }
  }

  return table;
}
