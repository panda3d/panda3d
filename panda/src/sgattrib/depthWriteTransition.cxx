// Filename: depthWriteTransition.cxx
// Created by:  drose (31Mar00)
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

#include "depthWriteTransition.h"
#include "depthWriteAttribute.h"

#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle DepthWriteTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated DepthWriteTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *DepthWriteTransition::
make_copy() const {
  return new DepthWriteTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated DepthWriteAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *DepthWriteTransition::
make_attrib() const {
  return new DepthWriteAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteTransition::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a DepthWriteTransition object
////////////////////////////////////////////////////////////////////
void DepthWriteTransition::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_DepthWriteTransition);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteTransition::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void DepthWriteTransition::
write_datagram(BamWriter *manager, Datagram &me) {
  OnOffTransition::write_datagram(manager, me);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteTransition::make_DepthWriteTransition
//       Access: Public
//  Description: Factory method to generate a DepthWriteTransition object
////////////////////////////////////////////////////////////////////
TypedWritable *DepthWriteTransition::
make_DepthWriteTransition(const FactoryParams &params) {
  DepthWriteTransition *me = new DepthWriteTransition;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteTransition::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void DepthWriteTransition::
fillin(DatagramIterator &scan, BamReader *manager) {
  OnOffTransition::fillin(scan, manager);
}
