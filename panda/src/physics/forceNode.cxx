// Filename: forceNode.cxx
// Created by:  charles (02Aug00)
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

#include "forceNode.h"
#include "config_physics.h"

TypeHandle ForceNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : ForceNode
//       Access : public
//  Description : default constructor
////////////////////////////////////////////////////////////////////
ForceNode::
ForceNode(const string &name) :
  PandaNode(name) {
}

////////////////////////////////////////////////////////////////////
//     Function : ForceNode
//       Access : protected
//  Description : copy constructor
////////////////////////////////////////////////////////////////////
ForceNode::
ForceNode(const ForceNode &copy) :
  PandaNode(copy), _forces(copy._forces) {
}

////////////////////////////////////////////////////////////////////
//     Function : ~ForceNode
//       Access : public, virtual
//  Description : destructor
////////////////////////////////////////////////////////////////////
ForceNode::
~ForceNode() {
}

////////////////////////////////////////////////////////////////////
//     Function : make_copy
//       Access : public, virtual
//  Description : dynamic child copy
////////////////////////////////////////////////////////////////////
PandaNode *ForceNode::
make_copy() const {
  return new ForceNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function : add_forces_from
//       Access : public
//  Description : append operation
////////////////////////////////////////////////////////////////////
void ForceNode::
add_forces_from(const ForceNode &other) {
  pvector< PT(BaseForce) >::iterator last = _forces.end() - 1;

  _forces.insert(_forces.end(),
                 other._forces.begin(), other._forces.end());

  for (; last != _forces.end(); last++) {
    (*last)->_force_node = this;
  }
}

////////////////////////////////////////////////////////////////////
//     Function : remove_force
//       Access : public
//  Description : remove operation
////////////////////////////////////////////////////////////////////
void ForceNode::
remove_force(BaseForce *f) {
  pvector< PT(BaseForce) >::iterator found;
  PT(BaseForce) ptbf = f;
  found = find(_forces.begin(), _forces.end(), ptbf);
  if (found == _forces.end())
    return;
  _forces.erase(found);
}

////////////////////////////////////////////////////////////////////
//     Function : remove_force
//       Access : public
//  Description : remove operation
////////////////////////////////////////////////////////////////////
void ForceNode::
remove_force(int index) {
  nassertv(index >= 0 && index <= (int)_forces.size());

  pvector< PT(BaseForce) >::iterator remove;
  remove = _forces.begin() + index;
  (*remove)->_force_node = (ForceNode *) NULL;

  _forces.erase(remove);
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void ForceNode::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"ForceNode";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write_linear_forces
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void ForceNode::
write_forces(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""<<"_forces\n";
  for (ForceVector::const_iterator i=_forces.begin();
       i != _forces.end();
       ++i) {
    (*i)->write(out, indent+2);
  }
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void ForceNode::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"ForceNode\n";
  write_forces(out, indent+2);
  //PandaNode::write(out, indent+2);
  #endif //] NDEBUG
}
