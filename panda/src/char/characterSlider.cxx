// Filename: characterSlider.cxx
// Created by:  drose (03Mar99)
// 
////////////////////////////////////////////////////////////////////

#include "characterSlider.h"
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle CharacterSlider::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CharacterSlider::Default Constructor
//       Access: Protected
//  Description: For internal use only.
////////////////////////////////////////////////////////////////////
CharacterSlider::
CharacterSlider() {
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterSlider::Copy Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
CharacterSlider::
CharacterSlider(const CharacterSlider &copy) :
  MovingPartScalar(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterSlider::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CharacterSlider::
CharacterSlider(PartGroup *parent, const string &name)
  : MovingPartScalar(parent, name) {
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterSlider::make_copy
//       Access: Public, Virtual
//  Description: Allocates and returns a new copy of the node.
//               Children are not copied, but see copy_subgraph().
////////////////////////////////////////////////////////////////////
PartGroup *CharacterSlider::
make_copy() const {
  return new CharacterSlider(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterSlider::make_CharacterSlider
//       Access: Protected
//  Description: Factory method to generate a CharacterSlider object
////////////////////////////////////////////////////////////////////
TypedWriteable* CharacterSlider::
make_CharacterSlider(const FactoryParams &params)
{
  CharacterSlider *me = new CharacterSlider;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterSlider::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a CharacterSlider object
////////////////////////////////////////////////////////////////////
void CharacterSlider::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_CharacterSlider);
}




