// Filename: xFileAnimationSet.cxx
// Created by:  drose (02Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "xFileAnimationSet.h"
#include "xFileToEggConverter.h"

#include "eggGroup.h"
#include "eggTable.h"
#include "eggData.h"
#include "eggXfmSAnim.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//     Function: XFileAnimationSet::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileAnimationSet::
XFileAnimationSet() {
}

////////////////////////////////////////////////////////////////////
//     Function: XFileAnimationSet::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileAnimationSet::
~XFileAnimationSet() {
}

////////////////////////////////////////////////////////////////////
//     Function: XFileAnimationSet::create_hierarchy
//       Access: Public
//  Description: Sets up the hierarchy of EggTables corresponding to
//               this AnimationSet.
////////////////////////////////////////////////////////////////////
bool XFileAnimationSet::
create_hierarchy(XFileToEggConverter *converter) {
  // Egg animation tables start off with one Table entry, enclosing a
  // Bundle entry.
  EggTable *table = new EggTable(get_name());
  converter->get_egg_data().add_child(table);
  EggTable *bundle = new EggTable(converter->_char_name);
  table->add_child(bundle);
  bundle->set_table_type(EggTable::TT_bundle);

  // Then the Bundle contains a "<skeleton>" entry, which begins the
  // animation table hierarchy.
  EggTable *skeleton = new EggTable("<skeleton>");
  bundle->add_child(skeleton);

  // Fill in the rest of the hierarchy with empty tables.
  mirror_table(converter->get_dart_node(), skeleton);

  // Now populate those empty tables with the frame data.
  JointData::const_iterator ji;
  for (ji = _joint_data.begin(); ji != _joint_data.end(); ++ji) {
    const string &joint_name = (*ji).first;
    const FrameData &table = (*ji).second;

    EggXfmSAnim *anim_table = get_table(joint_name);
    if (anim_table == (EggXfmSAnim *)NULL) {
      xfile_cat.warning()
        << "Frame " << joint_name << ", named by animation data, not defined.\n";
    } else {
      // If we have animation data, apply it.
      FrameData::const_iterator fi;
      for (fi = table.begin(); fi != table.end(); ++fi) {
        anim_table->add_data(*fi);
      }
      anim_table->optimize();
    }
  }

  // Put some data in the empty tables also.
  Tables::iterator ti;
  for (ti = _tables.begin(); ti != _tables.end(); ++ti) {
    const string &joint_name = (*ti).first;
    EggXfmSAnim *anim_table = (*ti).second._table;
    EggGroup *joint = (*ti).second._joint;
    if (anim_table->empty() && joint != (EggGroup *)NULL) {
      // If there's no animation data, assign the rest transform.
      anim_table->add_data(joint->get_transform());
    }
    anim_table->optimize();
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileAnimationSet::get_table
//       Access: Public
//  Description: Returns the table associated with the indicated joint
//               name.
////////////////////////////////////////////////////////////////////
EggXfmSAnim *XFileAnimationSet::
get_table(const string &joint_name) const {
  Tables::const_iterator ti;
  ti = _tables.find(joint_name);
  if (ti != _tables.end()) {
    return (*ti).second._table;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileAnimationSet::create_frame_data
//       Access: Public
//  Description: Returns a reference to a new FrameData table
//               corresponding to the indicated joint.
////////////////////////////////////////////////////////////////////
XFileAnimationSet::FrameData &XFileAnimationSet::
create_frame_data(const string &joint_name) {
  return _joint_data[joint_name];
}

////////////////////////////////////////////////////////////////////
//     Function: XFileAnimationSet::mirror_table
//       Access: Private
//  Description: Builds up a new set of EggTable nodes, as a
//               mirror of the existing set of EggGroup (joint)
//               nodes, and saves each new table in the _tables
//               record.
////////////////////////////////////////////////////////////////////
void XFileAnimationSet::
mirror_table(EggGroup *model_node, EggTable *anim_node) {
  EggGroupNode::iterator gi;
  for (gi = model_node->begin(); gi != model_node->end(); ++gi) {
    EggNode *child = (*gi);
    if (child->is_of_type(EggGroup::get_class_type())) {
      EggGroup *group = DCAST(EggGroup, child);
      if (group->get_group_type() == EggGroup::GT_joint) {
        // When we come to a <Joint>, create a new Table for it.
        EggTable *new_table = new EggTable(group->get_name());
        anim_node->add_child(new_table);
        EggXfmSAnim *xform = new EggXfmSAnim("xform");
        new_table->add_child(xform);
        TablePair &table_pair = _tables[group->get_name()];
        table_pair._table = xform;
        table_pair._joint = group;

        // Now recurse.
        mirror_table(group, new_table);

      } else {
        // If we come to an ordinary <Group>, skip past it.
        mirror_table(group, anim_node);
      }
    }
  }
}
