// Filename: eggCharacterData.cxx
// Created by:  drose (23Feb01)
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

#include "eggCharacterData.h"
#include "eggCharacterCollection.h"
#include "eggJointData.h"
#include "eggSliderData.h"

#include "indent.h"

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggCharacterData::
EggCharacterData(EggCharacterCollection *collection) {
  _collection = collection;
  _root_joint = _collection->make_joint_data(this);
  // The fictitious root joint is not added to the _components list.
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
EggCharacterData::
~EggCharacterData() {
  delete _root_joint;

  Sliders::iterator si;
  for (si = _sliders.begin(); si != _sliders.end(); ++si) {
    EggSliderData *slider = (*si);
    delete slider;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::add_model
//       Access: Public
//  Description: Indicates that the given model_index (with the
//               indicated model_root) is associated with this
//               character.  This is normally called by the
//               EggCharacterCollection class as new models are
//               discovered.
//
//               A "model" here is either a character model (or one
//               LOD of a character model), or a character animation
//               file: in either case, a hierarchy of joints.
////////////////////////////////////////////////////////////////////
void EggCharacterData::
add_model(int model_index, EggNode *model_root) {
  Model m;
  m._model_index = model_index;
  m._model_root = model_root;
  _models.push_back(m);
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::get_num_frames
//       Access: Public
//  Description: Returns the number of frames of animation of the
//               indicated model.  This is more reliable than asking a
//               particular joint or slider of the animation for its
//               number of frames, since a particular joint may have
//               only 1 frame (if it is unanimated), even though the
//               overall animation has many frames.
////////////////////////////////////////////////////////////////////
int EggCharacterData::
get_num_frames(int model_index) const {
  int max_num_frames = 0;
  Components::const_iterator ci;
  for (ci = _components.begin(); ci != _components.end(); ++ci) {
    EggComponentData *component = (*ci);
    int num_frames = component->get_num_frames(model_index);
    if (num_frames > 1) {
      // We have a winner.  Assume all other components will be
      // similar.
      return num_frames;
    }
    max_num_frames = max(max_num_frames, num_frames);
  }

  // Every component had either 1 frame or 0 frames.  Return the
  // maximum of these.
  return max_num_frames;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::do_reparent
//       Access: Public
//  Description: Begins the process of restructuring the joint
//               hierarchy according to the previous calls to
//               reparent_to() on various joints.  This will reparent
//               the joint hierachy in all models as requested, while
//               adjusting the transforms as appropriate so that each
//               joint retains the same net transform across all
//               frames that it had before the operation.  Returns
//               true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool EggCharacterData::
do_reparent() {
  typedef pset<EggJointData *> InvalidSet;
  InvalidSet invalid_set;

  // First, make sure the list of new_children is accurate.
  Joints::const_iterator ji;
  for (ji = _joints.begin(); ji != _joints.end(); ++ji) {
    EggJointData *joint_data = (*ji);
    joint_data->do_begin_reparent();
  }
  // We also need to clear the children on the root joint, but the
  // root joint doesn't get any of the other operations (including
  // finish_reparent) applied to it.
  _root_joint->do_begin_reparent();

  // Now compute the new transforms for the joints' new positions.
  // This is done recursively through the new parent hierarchy, so we
  // can take advantage of caching the net value for a particular
  // frame.
  Models::const_iterator mi;
  for (mi = _models.begin(); mi != _models.end(); ++mi) {
    int model_index = (*mi)._model_index;
    int num_frames = get_num_frames(model_index);
    for (int f = 0; f < num_frames; f++) {
      // First, walk through all the joints and flush the computed net
      // transforms from before.
      for (ji = _joints.begin(); ji != _joints.end(); ++ji) {
        EggJointData *joint_data = (*ji);
        joint_data->do_begin_compute_reparent();
      }
      _root_joint->do_begin_compute_reparent();

      // Now go back through and compute the reparented transforms,
      // caching net transforms as necessary.
      for (ji = _joints.begin(); ji != _joints.end(); ++ji) {
        EggJointData *joint_data = (*ji);
        if (!joint_data->do_compute_reparent(model_index, f)) {
          // Oops, we got an invalid transform.
          invalid_set.insert(joint_data);
        }
      }
    }
  }

  // Now remove all of the old children and add in the new children.
  for (ji = _joints.begin(); ji != _joints.end(); ++ji) {
    EggJointData *joint_data = (*ji);
    if (!joint_data->do_finish_reparent()) {
      invalid_set.insert(joint_data);
    }
  }

  InvalidSet::const_iterator si;
  for (si = invalid_set.begin(); si != invalid_set.end(); ++si) {
    EggJointData *joint_data = (*si);
    // Don't bother reporting joints that no longer have a parent,
    // since we don't care about joints that are now outside the
    // hierarchy.
    if (joint_data->get_parent() != (EggJointData *)NULL) {
      nout << "Warning: reparenting " << joint_data->get_name()
           << " to ";
      if (joint_data->get_parent() == _root_joint) {
        nout << "the root";
      } else {
        nout << joint_data->get_parent()->get_name();
      }
      nout << " results in a skew transform.\n";
    }
  }

  return invalid_set.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::find_slider
//       Access: Public
//  Description: Returns the slider with the indicated name, or NULL
//               if no slider has that name.
////////////////////////////////////////////////////////////////////
EggSliderData *EggCharacterData::
find_slider(const string &name) const {
  SlidersByName::const_iterator si;
  si = _sliders_by_name.find(name);
  if (si != _sliders_by_name.end()) {
    return (*si).second;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::make_slider
//       Access: Public
//  Description: Returns the slider matching the indicated name.  If
//               no such slider exists already, creates a new one.
////////////////////////////////////////////////////////////////////
EggSliderData *EggCharacterData::
make_slider(const string &name) {
  SlidersByName::const_iterator si;
  si = _sliders_by_name.find(name);
  if (si != _sliders_by_name.end()) {
    return (*si).second;
  }

  EggSliderData *slider = _collection->make_slider_data(this);
  slider->set_name(name);
  _sliders_by_name.insert(SlidersByName::value_type(name, slider));
  _sliders.push_back(slider);
  _components.push_back(slider);
  return slider;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void EggCharacterData::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "Character " << get_name() << ":\n";
  get_root_joint()->write(out, indent_level + 2);

  Sliders::const_iterator si;
  for (si = _sliders.begin(); si != _sliders.end(); ++si) {
    EggSliderData *slider = (*si);
    slider->write(out, indent_level + 2);
  }
}
