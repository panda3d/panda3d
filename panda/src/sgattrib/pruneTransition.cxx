// Filename: pruneTransition.cxx
// Created by:  drose (26Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "pruneTransition.h"
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle PruneTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PruneTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated PruneTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *PruneTransition::
make_copy() const {
  return new PruneTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PruneTransition::sub_render 
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool PruneTransition::
sub_render(NodeRelation *, const AllAttributesWrapper &,
	   AllTransitionsWrapper &, GraphicsStateGuardianBase *) {
  return false; 
}

////////////////////////////////////////////////////////////////////
//     Function: PruneTransition::has_sub_render
//       Access: Public, Virtual
//  Description: Should be redefined to return true if the function
//               sub_render(), above, expects to be called during
//               traversal.
////////////////////////////////////////////////////////////////////
bool PruneTransition::
has_sub_render() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PruneTransition::make_PruneTransition
//       Access: Protected
//  Description: Factory method to generate a PruneTransition object
////////////////////////////////////////////////////////////////////
TypedWriteable* PruneTransition::
make_PruneTransition(const FactoryParams &params)
{
  PruneTransition *me = new PruneTransition;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: PruneTransition::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a PruneTransition object
////////////////////////////////////////////////////////////////////
void PruneTransition::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_PruneTransition);
}
