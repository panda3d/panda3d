// Filename: userVertexTransform.cxx
// Created by:  drose (24Mar05)
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

#include "userVertexTransform.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle UserVertexTransform::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: UserVertexTransform::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
UserVertexTransform::
UserVertexTransform(const string &name) :
  _name(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: UserVertexTransform::get_matrix
//       Access: Published, Virtual
//  Description: Returns the transform's matrix.
////////////////////////////////////////////////////////////////////
void UserVertexTransform::
get_matrix(LMatrix4 &matrix) const {
  CDReader cdata(_cycler);
  matrix = cdata->_matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: UserVertexTransform::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void UserVertexTransform::
output(ostream &out) const {
  out << get_type() << " " << get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: UserVertexTransform::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *UserVertexTransform::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: UserVertexTransform::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               UserVertexTransform.
////////////////////////////////////////////////////////////////////
void UserVertexTransform::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: UserVertexTransform::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void UserVertexTransform::
write_datagram(BamWriter *manager, Datagram &dg) {
  VertexTransform::write_datagram(manager, dg);

  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: UserVertexTransform::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type UserVertexTransform is encountered
//               in the Bam file.  It should create the UserVertexTransform
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *UserVertexTransform::
make_from_bam(const FactoryParams &params) {
  UserVertexTransform *object = new UserVertexTransform("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: UserVertexTransform::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new UserVertexTransform.
////////////////////////////////////////////////////////////////////
void UserVertexTransform::
fillin(DatagramIterator &scan, BamReader *manager) {
  VertexTransform::fillin(scan, manager);

  manager->read_cdata(scan, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: UserVertexTransform::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void UserVertexTransform::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  _matrix.write_datagram(dg);
}

////////////////////////////////////////////////////////////////////
//     Function: UserVertexTransform::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new UserVertexTransform.
////////////////////////////////////////////////////////////////////
void UserVertexTransform::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _matrix.read_datagram(scan);
}
