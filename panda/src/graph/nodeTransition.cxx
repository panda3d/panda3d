// Filename: nodeTransition.cxx
// Created by:  drose (26Oct98)
// 
////////////////////////////////////////////////////////////////////

#include "nodeTransition.h"
#include "nodeTransitions.h"
#include "nodeRelation.h"

#include <indent.h>

TypeHandle NodeTransition::_type_handle;

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
//     Function: NodeTransition::sub_render
//       Access: Public, Virtual
//  Description: This virtual function is normally a no-op.  It is
//               called during the render traversal to allow a special
//               transition (e.g. a ShaderTransition) to intercept the
//               normal render traversal with some fancy rendering
//               process.
////////////////////////////////////////////////////////////////////
bool NodeTransition::
sub_render(NodeRelation *, const AllAttributesWrapper &, 
	   AllTransitionsWrapper &, GraphicsStateGuardianBase *) {
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


