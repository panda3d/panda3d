// Filename: forceNode.cxx
// Created by:  charles (02Aug00)
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

  NodePath node_path(this);
  for (; last != _forces.end(); last++) {
    (*last)->_force_node = this;
    (*last)->_force_node_path = node_path;
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
  (*remove)->_force_node_path = NodePath();

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
  PandaNode::output(out);
  out<<" ("<<_forces.size()<<" forces)";
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
  out.width(indent); out<<""<<"_forces ("<<_forces.size()<<" forces)"<<"\n";
  for (ForceVector::const_iterator i=_forces.begin();
       i != _forces.end();
       ++i) {
    out.width(indent+2); out<<""; out<<"(id "<<&(*i)<<" "<<(*i)->is_linear()<<")\n";
    //#*#(*i)->write(out, indent+2);
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
  out.width(indent); out<<""; out<<"ForceNode (id "<<this<<") ";
  //#*#PandaNode::output(out);
  out<<"\n";
  //#*#write_forces(out, indent+2);
  PandaNode::write(out, indent+4);
  #endif //] NDEBUG
}
