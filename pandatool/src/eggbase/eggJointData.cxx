// Filename: eggJointData.cxx
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "eggJointData.h"

#include <indent.h>


////////////////////////////////////////////////////////////////////
//     Function: EggJointData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggJointData::
EggJointData() {
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
EggJointData::
~EggJointData() {
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::matches_name
//       Access: Public
//  Description: Returns true if the indicated name matches any name
//               that was ever matched with this particular joint,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool EggJointData::
matches_name(const string &name) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::add_egg_node
//       Access: Public
//  Description: Adds the indicated model joint or anim table to the
//               data.
////////////////////////////////////////////////////////////////////
void EggJointData::
add_egg_node(int egg_index, int model_index, EggNode *egg_node) {
  if (!has_name() && egg_node->has_name()) {
    set_name(egg_node->get_name());
  }

  while ((int)_joints.size() <= egg_index) {
    _joints.push_back(JointNodes());
  }
  JointNodes &nodes = _joints[egg_index];
  while ((int)nodes.size() <= model_index) {
    nodes.push_back((EggNode *)NULL);
  }
  nodes[model_index] = egg_node;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointData::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void EggJointData::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "Joint " << get_name()
    << " (models:";
  for (size_t egg_index = 0; egg_index < _joints.size(); egg_index++) {
    const JointNodes &nodes = _joints[egg_index];
    if (nodes.size() == 1) {
      out << " " << egg_index;
    } else if (nodes.size() > 1) {
      for (size_t model_index = 0; model_index < nodes.size(); model_index++) {
	if (nodes[model_index] != (EggNode *)NULL) {
	  out << " " << egg_index << "/" << model_index;
	}
      }
    }
  }
  out << ") {\n";

  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->write(out, indent_level + 2);
  }

  indent(out, indent_level) << "}\n";
}
