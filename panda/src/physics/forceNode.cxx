// Filename: forceNode.cxx
// Created by:  charles (02Aug00)
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
  NamedNode(name) {
}

////////////////////////////////////////////////////////////////////
//     Function : ForceNode
//       Access : public
//  Description : copy constructor
////////////////////////////////////////////////////////////////////
ForceNode::
ForceNode(const ForceNode &copy) :
  NamedNode(copy), _forces(copy._forces) {
}

////////////////////////////////////////////////////////////////////
//     Function : ~ForceNode
//       Access : public, virtual
//  Description : destructor
////////////////////////////////////////////////////////////////////
ForceNode::
~ForceNode(void) {
}

////////////////////////////////////////////////////////////////////
//     Function : operator =
//       Access : public
//  Description : assignment operator
////////////////////////////////////////////////////////////////////
ForceNode &ForceNode::
operator =(const ForceNode &copy) {
  NamedNode::operator =(copy);
  _forces = copy._forces;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function : make_copy
//       Access : public, virtual
//  Description : dynamic child copy
////////////////////////////////////////////////////////////////////
Node *ForceNode::
make_copy(void) const {
  return new ForceNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function : add_forces_from
//       Access : public
//  Description : append operation
////////////////////////////////////////////////////////////////////
void ForceNode::
add_forces_from(const ForceNode &other) {
  vector< PT(BaseForce) >::iterator last = _forces.end() - 1;

  _forces.insert(_forces.end(), 
		 other._forces.begin(), other._forces.end());

  for (; last != _forces.end(); last++)
    (*last)->_force_node = this;
}

////////////////////////////////////////////////////////////////////
//     Function : remove_force
//       Access : public
//  Description : remove operation
////////////////////////////////////////////////////////////////////
void ForceNode::
remove_force(int index) {
  nassertv(index >= 0 && index <= (int)_forces.size());

  vector< PT(BaseForce) >::iterator remove;
  remove = _forces.begin() + index;
  (*remove)->_force_node = (ForceNode *) NULL;

  _forces.erase(remove);
}
