// Filename: pruneTransition.cxx
// Created by:  drose (26Apr00)
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
sub_render(NodeRelation *, const AllTransitionsWrapper &, 
           AllTransitionsWrapper &, RenderTraverser *) {
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
TypedWritable* PruneTransition::
make_PruneTransition(const FactoryParams &params)
{
  PruneTransition *me = new PruneTransition;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
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
