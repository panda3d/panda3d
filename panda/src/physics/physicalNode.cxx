// Filename: physicalNode.cxx
// Created by:  charles (01Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "physicalNode.h"

// static stuff.
TypeHandle PhysicalNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : PhysicalNode
//       Access : public
//  Description : default constructor
////////////////////////////////////////////////////////////////////
PhysicalNode::
PhysicalNode(const string &name) :
  NamedNode(name) {
}

////////////////////////////////////////////////////////////////////
//     Function : PhysicalNode
//       Access : public
//  Description : copy constructor
////////////////////////////////////////////////////////////////////
PhysicalNode::
PhysicalNode(const PhysicalNode &copy) :
  NamedNode(copy), _physicals(copy._physicals) {
}

////////////////////////////////////////////////////////////////////
//     Function : ~PhysicalNode
//       Access : public, virtual
//  Description : destructor
////////////////////////////////////////////////////////////////////
PhysicalNode::
~PhysicalNode(void) {
}

////////////////////////////////////////////////////////////////////
//     Function : operator =
//       Access : public
//  Description : assignment operator
////////////////////////////////////////////////////////////////////
PhysicalNode &PhysicalNode::
operator =(const PhysicalNode &copy) {
  NamedNode::operator =(copy);
  _physicals = copy._physicals;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function : make_copy
//       Access : public, virtual
//  Description : dynamic child copy
////////////////////////////////////////////////////////////////////
Node *PhysicalNode::
make_copy(void) const {
  return new PhysicalNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function : add_physicals_from
//       Access : public
//  Description : append operation
////////////////////////////////////////////////////////////////////
void PhysicalNode::
add_physicals_from(const PhysicalNode &other) {
  vector< PT(Physical) >::iterator last = _physicals.end() - 1;

  _physicals.insert(_physicals.end(), 
		    other._physicals.begin(), other._physicals.end());

  for (; last != _physicals.end(); last++)
    (*last)->_physical_node = this;
}

////////////////////////////////////////////////////////////////////
//     Function : remove_physical
//       Access : public
//  Description : remove operation
////////////////////////////////////////////////////////////////////
void PhysicalNode::
remove_physical(Physical *physical) {
  vector< PT(Physical) >::iterator found;
  PT(Physical) ptp = physical;
  found = find(_physicals.begin(), _physicals.end(), ptp);
  if (found == _physicals.end())
    return;
  _physicals.erase(found);
}

////////////////////////////////////////////////////////////////////
//     Function : remove_physical
//       Access : public
//  Description : remove operation
////////////////////////////////////////////////////////////////////
void PhysicalNode::
remove_physical(int index) {
  nassertv(index >= 0 && index <= (int)_physicals.size());

  vector< PT(Physical) >::iterator remove;
  remove = _physicals.begin() + index;
  (*remove)->_physical_node = (PhysicalNode *) NULL;

  _physicals.erase(remove);
}
