// Filename: characterJointBundle.cxx
// Created by:  drose (23Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "characterJointBundle.h"
#include "character.h"
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle CharacterJointBundle::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CharacterJointBundle::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CharacterJointBundle::
CharacterJointBundle(const string &name) : PartBundle(name) {
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJointBundle::make_copy
//       Access: Public, Virtual
//  Description: Allocates and returns a new copy of the node.
//               Children are not copied, but see copy_subgraph().
////////////////////////////////////////////////////////////////////
PartGroup *CharacterJointBundle::
make_copy() const {
  return new CharacterJointBundle(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJointBundle::make_CharacterJointBundle
//       Access: Protected
//  Description: Factory method to generate a CharacterJointBundle object
////////////////////////////////////////////////////////////////////
TypedWriteable* CharacterJointBundle::
make_CharacterJointBundle(const FactoryParams &params)
{
  CharacterJointBundle *me = new CharacterJointBundle;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  manager->register_finalize(me);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJointBundle::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a CharacterJointBundle object
////////////////////////////////////////////////////////////////////
void CharacterJointBundle::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_CharacterJointBundle);
}

