// Filename: paramNodePath.cxx
// Created by:  rdb (25Feb15)
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

#include "paramNodePath.h"
#include "dcast.h"
#include "pandaNode.h"

TypeHandle ParamNodePath::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ParamNodePath::output
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ParamNodePath::
output(ostream &out) const {
  out << "node path " << _node_path;
}

////////////////////////////////////////////////////////////////////
//     Function: ParamNodePath::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               ParamValue.
////////////////////////////////////////////////////////////////////
void ParamNodePath::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: ParamNodePath::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void ParamNodePath::
write_datagram(BamWriter *manager, Datagram &dg) {
  ParamValueBase::write_datagram(manager, dg);

  // We can't store a NodePath, so we store a pointer to the
  // underlying node, and pray that there is an unambiguous path
  // from the root to it.
  if (_node_path.is_empty()) {
    manager->write_pointer(dg, NULL);
  } else {
    manager->write_pointer(dg, _node_path.node());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ParamNodePath::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int ParamNodePath::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = ParamValueBase::complete_pointers(p_list, manager);
  _node_path = NodePath(DCAST(PandaNode, p_list[pi++]));

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: ParamNodePath::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type ParamValue is encountered
//               in the Bam file.  It should create the ParamValue
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *ParamNodePath::
make_from_bam(const FactoryParams &params) {
  ParamNodePath *param = new ParamNodePath;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

////////////////////////////////////////////////////////////////////
//     Function: ParamNodePath::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new ParamValue.
////////////////////////////////////////////////////////////////////
void ParamNodePath::
fillin(DatagramIterator &scan, BamReader *manager) {
  ParamValueBase::fillin(scan, manager);
  manager->read_pointer(scan);
}
