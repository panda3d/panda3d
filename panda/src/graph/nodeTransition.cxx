// Filename: nodeTransition.cxx
// Created by:  drose (26Oct98)
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

#include "nodeTransition.h"
#include "nodeTransitions.h"
#include "nodeRelation.h"
#include "config_graph.h"

#include "indent.h"

TypeHandle NodeTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::compare_to
//       Access: Public
//  Description: This function works like strcmp(): it compares the
//               two transitions and returns a number less than zero
//               if this transition sorts before the other one, equal
//               to zero if they are equivalent, or greater than zero
//               if this transition sorts after the other one.
//
//               This imposes an arbitrary sorting order across all
//               transitions, whose sole purpose is to allow grouping
//               of equivalent transitions together in STL structures
//               like maps and sets.
////////////////////////////////////////////////////////////////////
int NodeTransition::
compare_to(const NodeTransition &other) const {
  if (this == &other) {
    // Same pointer, no comparison necessary.
    return 0;
  }

  TypeHandle my_handle = get_handle();
  TypeHandle other_handle = other.get_handle();

  if (my_handle != other_handle) {
    return
      (my_handle < other_handle) ? -1 : 1;

  } else if (_priority != other._priority) {
    return _priority - other._priority;

  } else {
    return internal_compare_to(&other);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::get_handle
//       Access: Public, Virtual
//  Description: Returns the TypeHandle that is used to identify this
//               particular transition (and its attribute) on the
//               graph arcs.  Normally this will be the same as the
//               transition's type, e.g. get_type(), but certain
//               transition types may want to redefine this.
////////////////////////////////////////////////////////////////////
TypeHandle NodeTransition::
get_handle() const {
  return get_type();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::make_initial
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeTransition *NodeTransition::
make_initial() const {
  graph_cat.warning()
    << "make_initial() not defined for " << get_type() << "\n";
  return (NodeTransition *)this;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::sub_render
//       Access: Public, Virtual
//  Description: This virtual function is normally a no-op.  It is
//               called during the render traversal to allow a special
//               transition (e.g. a ShaderTransition) to intercept the
//               normal render traversal with some fancy rendering
//               process.
////////////////////////////////////////////////////////////////////
bool NodeTransition::
sub_render(NodeRelation *, const AllTransitionsWrapper &, 
           AllTransitionsWrapper &, RenderTraverser *) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::has_sub_render
//       Access: Public, Virtual
//  Description: Should be redefined to return true if the function
//               sub_render(), above, expects to be called during
//               traversal.
////////////////////////////////////////////////////////////////////
bool NodeTransition::
has_sub_render() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void NodeTransition::
output(ostream &out) const {
  out << get_handle();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void NodeTransition::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void NodeTransition::
issue(GraphicsStateGuardianBase *) {
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::internal_generate_hash
//       Access: Protected, Virtual
//  Description: Should be overridden by particular NodeTransitions to
//               generate a hash value uniquifying this particular
//               transition.
////////////////////////////////////////////////////////////////////
void NodeTransition::
internal_generate_hash(GraphHashGenerator &) const {
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::state_changed
//       Access: Protected
//  Description: This should be called by any internal method (like
//               set_priority() or set_value()) that changes the state
//               of the transition and may thus affect the state cache
//               on the scene graph.  It notifies all arcs that this
//               transition has been applied to of the change.
////////////////////////////////////////////////////////////////////
void NodeTransition::
state_changed() {
  TypeHandle handle = get_handle();

  Arcs::const_iterator ai;
  for (ai = _arcs.begin(); ai != _arcs.end(); ++ai) {
    (*ai)->changed_transition(handle);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void NodeTransition::
write_datagram(BamWriter *, Datagram &me)
{
  me.add_uint16(_priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void NodeTransition::
fillin(DatagramIterator& scan, BamReader*)
{
  _priority = scan.get_uint16();
}

