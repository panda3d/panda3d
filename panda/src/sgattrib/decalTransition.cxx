// Filename: decalTransition.cxx
// Created by:  drose (17Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "decalTransition.h"
#include "decalAttribute.h"
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle DecalTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DecalTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated DecalTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *DecalTransition::
make_copy() const {
  return new DecalTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DecalTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated DecalAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *DecalTransition::
make_attrib() const {
  return new DecalAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: DecalTransition::has_sub_render
//       Access: Public, Virtual
//  Description: DecalTransition doesn't actually have a sub_render()
//               function, but it might as well, because it's treated
//               as a special case.  We set this function to return
//               true so GraphReducer will behave correctly.
////////////////////////////////////////////////////////////////////
bool DecalTransition::
has_sub_render() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DecalTransition::make_DecalTransition
//       Access: Protected
//  Description: Factory method to generate a DecalTransition object
////////////////////////////////////////////////////////////////////
TypedWriteable* DecalTransition::
make_DecalTransition(const FactoryParams &params)
{
  DecalTransition *me = new DecalTransition;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: DecalTransition::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a DecalTransition object
////////////////////////////////////////////////////////////////////
void DecalTransition::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_DecalTransition);
}
