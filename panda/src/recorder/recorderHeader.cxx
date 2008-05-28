// Filename: recorderHeader.cxx
// Created by:  drose (29Jan04)
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

#include "recorderHeader.h"
#include "recorderTable.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "config_recorder.h"

TypeHandle RecorderHeader::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: RecorderHeader::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Lens.
////////////////////////////////////////////////////////////////////
void RecorderHeader::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderHeader::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void RecorderHeader::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);
  
  // One day this will need to be upgraded to a uint64, but probably
  // not before 2106.  (In 2038, Unix time will overflow a signed
  // 32-bit number, but this is an unsigned number and will still be
  // good until 2106.)
  dg.add_uint32(_start_time);

  dg.add_int32(_random_seed);
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderHeader::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Lens is encountered
//               in the Bam file.  It should create the Lens
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *RecorderHeader::
make_from_bam(const FactoryParams &params) {
  RecorderHeader *header = new RecorderHeader;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  header->fillin(scan, manager);

  return header;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderHeader::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new RecorderHeader.
////////////////////////////////////////////////////////////////////
void RecorderHeader::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  _start_time = scan.get_uint32();
  _random_seed = scan.get_int32();
}
