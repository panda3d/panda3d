// Filename: decalTransition.cxx
// Created by:  drose (17Apr00)
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

#include "decalTransition.h"
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

PT(NodeTransition) DecalTransition::_initial;
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
//     Function: DecalTransition::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated DecalTransition
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *DecalTransition::
make_initial() const {
  if (_initial.is_null()) {
    _initial = new DecalTransition;
  }
  return _initial;
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
TypedWritable* DecalTransition::
make_DecalTransition(const FactoryParams &params)
{
  DecalTransition *me = new DecalTransition;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
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
