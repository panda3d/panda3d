// Filename: characterSlider.cxx
// Created by:  drose (03Mar99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "characterSlider.h"
#include "characterVertexSlider.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

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
//     Function: CharacterSlider::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CharacterSlider::
~CharacterSlider() {
  nassertv(_vertex_sliders.empty());
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
//     Function: CharacterSlider::update_internals
//       Access: Public, Virtual
//  Description: This is called by do_update() whenever the part or
//               some ancestor has changed values.  It is a hook for
//               derived classes to update whatever cache they may
//               have that depends on these.
//
//               The return value is true if the part has changed as a
//               result of the update, or false otherwise.
////////////////////////////////////////////////////////////////////
bool CharacterSlider::
update_internals(PartBundle *, PartGroup *, bool, bool, Thread *current_thread) {
  // Tell our related CharacterVertexSliders that they now need to
  // recompute themselves.
  VertexSliders::iterator vsi;
  for (vsi = _vertex_sliders.begin(); vsi != _vertex_sliders.end(); ++vsi) {
    (*vsi)->mark_modified(current_thread);
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterSlider::make_CharacterSlider
//       Access: Protected
//  Description: Factory method to generate a CharacterSlider object
////////////////////////////////////////////////////////////////////
TypedWritable* CharacterSlider::
make_CharacterSlider(const FactoryParams &params)
{
  CharacterSlider *me = new CharacterSlider;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterSlider::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a CharacterSlider object
////////////////////////////////////////////////////////////////////
void CharacterSlider::
register_with_read_factory()
{
  BamReader::get_factory()->register_factory(get_class_type(), make_CharacterSlider);
}




