// Filename: nodeAttributes.cxx
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "nodeAttributes.h"
#include "nodeTransitionCache.h"
#include "config_graph.h"
#include "setTransitionHelpers.h"

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttributes::
NodeAttributes() {
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttributes::
NodeAttributes(const NodeAttributes &copy) :
  _attributes(copy._attributes)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeAttributes::
operator = (const NodeAttributes &copy) {
  _attributes = copy._attributes;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttributes::
~NodeAttributes() {
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::is_empty
//       Access: Public
//  Description: Returns true if there are no Attributes stored in
//               the set, or false if there are any (even initial)
//               Attributes.
////////////////////////////////////////////////////////////////////
bool NodeAttributes::
is_empty() const {
  return _attributes.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::set_attribute
//       Access: Public
//  Description: Stores the indicated attribute pointer in the
//               NodeAttributes set, according to the indicated
//               TypeHandle.  Node that attributes are stored
//               according to the TypeHandle of the associated
//               *transition*, not of the attributes' own TypeHandle.
//               Thus, there cannot be a flavor of set_attribute()
//               that automatically infers the correct TypeHandle
//               based on the attribute type.
//
//               The NodeAttribute may be NULL indicating that the
//               attribute should be cleared.  If the NodeAttribute is
//               not NULL, it must match the type indicated by the
//               TypeHandle.
//
//               The return value is a pointer to the *previous*
//               attribute in the set, if any, or NULL if there was
//               none.
////////////////////////////////////////////////////////////////////
PT(NodeAttribute) NodeAttributes::
set_attribute(TypeHandle handle, NodeAttribute *attrib) {
  if (attrib == (NodeAttribute *)NULL) {
    return clear_attribute(handle);

  } else {
    Attributes::iterator ti;
    ti = _attributes.find(handle);
    if (ti != _attributes.end()) {
      PT(NodeAttribute) result = (*ti).second;
      (*ti).second = attrib;
      return result;
    }

    _attributes.insert(Attributes::value_type(handle, attrib));
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::clear_attribute
//       Access: Public
//  Description: Removes any attribute associated with the indicated
//               handle from the set.
//
//               The return value is a pointer to the previous
//               attribute in the set, if any, or NULL if there was
//               none.
////////////////////////////////////////////////////////////////////
PT(NodeAttribute) NodeAttributes::
clear_attribute(TypeHandle handle) {
  nassertr(handle != TypeHandle::none(), NULL);

  Attributes::iterator ti;
  ti = _attributes.find(handle);
  if (ti != _attributes.end()) {
    PT(NodeAttribute) result = (*ti).second;
    _attributes.erase(ti);
    return result;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::has_attribute
//       Access: Public
//  Description: Returns true if ab attribute associated with the
//               indicated handle has been stored in the set (even if
//               it is the initial attribute), or false otherwise.
////////////////////////////////////////////////////////////////////
bool NodeAttributes::
has_attribute(TypeHandle handle) const {
  nassertr(handle != TypeHandle::none(), false);
  return _attributes.count(handle) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::get_attribute
//       Access: Public
//  Description: Returns the attribute associated with the indicated
//               handle, or NULL if no such attribute has been stored
//               in the set.
////////////////////////////////////////////////////////////////////
NodeAttribute *NodeAttributes::
get_attribute(TypeHandle handle) const {
  nassertr(handle != TypeHandle::none(), NULL);
  Attributes::const_iterator ai;
  ai = _attributes.find(handle);
  if (ai != _attributes.end()) {
    return (*ai).second;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::clear
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeAttributes::
clear() {
  _attributes.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::is_initial
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool NodeAttributes::
is_initial() const {
  Attributes::const_iterator ai;
  for (ai = _attributes.begin(); ai != _attributes.end(); ++ai) {
    if ((*ai).second != (NodeAttribute *)NULL) {
      //        && !(*ai).second->is_initial()
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
int NodeAttributes::
compare_to(const NodeAttributes &other) const {
  return tmap_compare_attr(_attributes.begin(), _attributes.end(),
			   other._attributes.begin(), other._attributes.end());
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::apply_from
//       Access: Public
//  Description: Modifies the current NodeAttributes object to reflect
//               the application of the indicated NodeAttributes and
//               the indicated transitions.  The "other"
//               NodeAttributes object may be the same as this.
////////////////////////////////////////////////////////////////////
void NodeAttributes::
apply_from(const NodeAttributes &other, const NodeTransitionCache &trans) {
  Attributes temp;

  tmap_apply(other._attributes.begin(), other._attributes.end(),
	     trans._cache.begin(), trans._cache.end(),
	     inserter(temp, temp.begin()));

  _attributes.swap(temp);
}


////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeAttributes::
output(ostream &out) const {
  bool written_any = false;

  Attributes::const_iterator ai;
  for (ai = _attributes.begin(); ai != _attributes.end(); ++ai) {
    if ((*ai).second != (NodeAttribute *)NULL) {
      if (written_any) {
	out << " ";
      }
      out << *(*ai).second;
      written_any = true;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeAttributes::
write(ostream &out, int indent_level) const {
  Attributes::const_iterator ai;
  for (ai = _attributes.begin(); ai != _attributes.end(); ++ai) {
    if ((*ai).second != (NodeAttribute *)NULL) {
      (*ai).second->write(out, indent_level);
    }
  }
}
