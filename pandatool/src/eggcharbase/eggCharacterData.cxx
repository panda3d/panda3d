// Filename: eggCharacterData.cxx
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "eggCharacterData.h"
#include "eggCharacterCollection.h"
#include "eggJointData.h"
#include "eggSliderData.h"

#include <indent.h>

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggCharacterData::
EggCharacterData(EggCharacterCollection *collection) {
  _collection = collection;
  _root_joint = _collection->make_joint_data(this);
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
    EggSliderData *slider = (*si).second;
    delete slider;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::make_slider
//       Access: Public
//  Description: Returns the slider matching the indicated name.  If
//               no such slider exists already, creates a new one.
////////////////////////////////////////////////////////////////////
EggSliderData *EggCharacterData::
make_slider(const string &name) {
  Sliders::iterator si;
  si = _sliders.find(name);
  if (si != _sliders.end()) {
    return (*si).second;
  }

  EggSliderData *slider = _collection->make_slider_data(this);
  slider->set_name(name);
  _sliders.insert(Sliders::value_type(name, slider));
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
    EggSliderData *slider = (*si).second;
    slider->write(out, indent_level + 2);
  }
}
