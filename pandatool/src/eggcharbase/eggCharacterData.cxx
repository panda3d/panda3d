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
