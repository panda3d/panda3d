// Filename: cullFaceTransition.cxx
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "cullFaceTransition.h"
#include "cullFaceAttribute.h"

#include <indent.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle CullFaceTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CullFaceTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated CullFaceTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *CullFaceTransition::
make_copy() const {
  return new CullFaceTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated CullFaceAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *CullFaceTransition::
make_attrib() const {
  return new CullFaceAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another CullFaceTransition.
////////////////////////////////////////////////////////////////////
void CullFaceTransition::
set_value_from(const OnTransition *other) {
  const CullFaceTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceTransition::compare_values
//       Access: Protected, Virtual
//  Description: Returns true if the two transitions have the same
//               value.  It is guaranteed that the other transition is
//               another CullFaceTransition, and that both are "on".
////////////////////////////////////////////////////////////////////
int CullFaceTransition::
compare_values(const OnTransition *other) const {
  const CullFaceTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void CullFaceTransition::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void CullFaceTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceTransition::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void CullFaceTransition::
write_datagram(BamWriter *manager, Datagram &me)
{
  OnTransition::write_datagram(manager, me);
  _value.write_datagram(me);
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceTransition::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void CullFaceTransition::
fillin(DatagramIterator& scan, BamReader* manager)
{
  OnTransition::fillin(scan, manager);
  _value.read_datagram(scan);
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceTransition::make_CullFaceTransition
//       Access: Protected
//  Description: Factory method to generate a CullFaceTransition object
////////////////////////////////////////////////////////////////////
TypedWriteable* CullFaceTransition::
make_CullFaceTransition(const FactoryParams &params)
{
  CullFaceTransition *me = new CullFaceTransition;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceTransition::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a CullFaceTransition object
////////////////////////////////////////////////////////////////////
void CullFaceTransition::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_CullFaceTransition);
}



