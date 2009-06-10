// Filename: cfCommand.cxx
// Created by:  drose (19Feb09)
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

#include "cfCommand.h"

TypeHandle CFCommand::_type_handle;
TypeHandle CFDoCullCommand::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CFCommand::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CFCommand::
~CFCommand() {
}

////////////////////////////////////////////////////////////////////
//     Function: CFDoCullCommand::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               CFDoCullCommand.
////////////////////////////////////////////////////////////////////
void CFDoCullCommand::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: CFDoCullCommand::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void CFDoCullCommand::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);
  manager->write_pointer(dg, _scene);
}

////////////////////////////////////////////////////////////////////
//     Function: CFDoCullCommand::update_bam_nested
//       Access: Public, Virtual
//  Description: Called by the BamWriter when this object has not
//               itself been modified recently, but it should check
//               its nested objects for updates.
////////////////////////////////////////////////////////////////////
void CFDoCullCommand::
update_bam_nested(BamWriter *manager) {
  manager->consider_update(_scene);
}

////////////////////////////////////////////////////////////////////
//     Function: CFDoCullCommand::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int CFDoCullCommand::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritable::complete_pointers(p_list, manager);
  
  PandaNode *scene;
  DCAST_INTO_R(scene, p_list[pi++], pi);
  _scene = scene;

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: CFDoCullCommand::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type CFDoCullCommand is encountered
//               in the Bam file.  It should create the CFDoCullCommand
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *CFDoCullCommand::
make_from_bam(const FactoryParams &params) {
  CFDoCullCommand *node = new CFDoCullCommand;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: CFDoCullCommand::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new CFDoCullCommand.
////////////////////////////////////////////////////////////////////
void CFDoCullCommand::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
  manager->read_pointer(scan);
}
