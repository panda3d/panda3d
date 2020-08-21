/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file forceNode.cxx
 * @author charles
 * @date 2000-08-02
 */

#include "forceNode.h"
#include "config_physics.h"

TypeHandle ForceNode::_type_handle;

/**
 * default constructor
 */
ForceNode::
ForceNode(const std::string &name) :
  PandaNode(name) {
}

/**
 * copy constructor
 */
ForceNode::
ForceNode(const ForceNode &copy) :
  PandaNode(copy), _forces(copy._forces) {
}

/**
 * destructor
 */
ForceNode::
~ForceNode() {
}

/**
 * dynamic child copy
 */
PandaNode *ForceNode::
make_copy() const {
  return new ForceNode(*this);
}

/**
 * append operation
 */
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

/**
 * replace operation
 */
void ForceNode::
set_force(size_t index, BaseForce *force) {
  nassertv(index < _forces.size());

  _forces[index]->_force_node = nullptr;
  _forces[index]->_force_node_path.clear();
  _forces[index] = force;
  force->_force_node = this;
  force->_force_node_path = NodePath(this);
}

/**
 * insert operation
 */
void ForceNode::
insert_force(size_t index, BaseForce *force) {
  if (index > _forces.size()) {
    index = _forces.size();
  }

  _forces.insert(_forces.begin() + index, force);
  force->_force_node = this;
  force->_force_node_path = NodePath(this);
}

/**
 * remove operation
 */
void ForceNode::
remove_force(BaseForce *f) {
  pvector< PT(BaseForce) >::iterator found;
  PT(BaseForce) ptbf = f;
  found = find(_forces.begin(), _forces.end(), ptbf);
  if (found == _forces.end())
    return;
  _forces.erase(found);
}

/**
 * remove operation
 */
void ForceNode::
remove_force(size_t index) {
  nassertv(index <= _forces.size());

  pvector< PT(BaseForce) >::iterator remove;
  remove = _forces.begin() + index;
  (*remove)->_force_node = nullptr;
  (*remove)->_force_node_path = NodePath();

  _forces.erase(remove);
}

/**
 * Write a string representation of this instance to <out>.
 */
void ForceNode::
output(std::ostream &out) const {
  PandaNode::output(out);
  out<<" ("<<_forces.size()<<" forces)";
}

/**
 * Write a string representation of this instance to <out>.
 */
void ForceNode::
write_forces(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""<<"_forces ("<<_forces.size()<<" forces)"<<"\n";
  for (ForceVector::const_iterator i=_forces.begin();
       i != _forces.end();
       ++i) {
    out.width(indent+2); out<<""; out<<"(id "<<&(*i)<<" "<<(*i)->is_linear()<<")\n";
    // #*#(*i)->write(out, indent+2);
  }
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void ForceNode::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"ForceNode (id "<<this<<") ";
  // #*#PandaNode::output(out);
  out<<"\n";
  // #*#write_forces(out, indent+2);
  PandaNode::write(out, indent+4);
  #endif //] NDEBUG
}
