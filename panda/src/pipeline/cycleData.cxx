// Filename: cycleData.cxx
// Created by:  drose (21Feb02)
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

#include "cycleData.h"


////////////////////////////////////////////////////////////////////
//     Function: CycleData::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData::
~CycleData() {
}

////////////////////////////////////////////////////////////////////
//     Function: CycleData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void CycleData::
write_datagram(BamWriter *, Datagram &) const {
}

////////////////////////////////////////////////////////////////////
//     Function: CycleData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void CycleData::
write_datagram(BamWriter *, Datagram &, void *) const {
}

////////////////////////////////////////////////////////////////////
//     Function: CycleData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int CycleData::
complete_pointers(TypedWritable **, BamReader *) {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CycleData::fillin
//       Access: Public, Virtual
//  Description: This internal function is intended to be called by
//               each class's make_from_bam() method to read in all of
//               the relevant data from the BamFile for the new
//               object.
////////////////////////////////////////////////////////////////////
void CycleData::
fillin(DatagramIterator &, BamReader *) {
}


////////////////////////////////////////////////////////////////////
//     Function: CycleData::fillin
//       Access: Public, Virtual
//  Description: This internal function is intended to be called by
//               each class's make_from_bam() method to read in all of
//               the relevant data from the BamFile for the new
//               object.
////////////////////////////////////////////////////////////////////
void CycleData::
fillin(DatagramIterator &, BamReader *, void *) {
}

////////////////////////////////////////////////////////////////////
//     Function: CycleData::get_parent_type
//       Access: Public, Virtual
//  Description: Returns the type of the container that owns the
//               CycleData.  This is useful mainly for debugging.
////////////////////////////////////////////////////////////////////
TypeHandle CycleData::
get_parent_type() const {
  return TypeHandle::none();
}

////////////////////////////////////////////////////////////////////
//     Function: CycleData::output
//       Access: Public, Virtual
//  Description: Formats the contents of the CycleData in some
//               meaningful way for humans.  This is useful mainly for
//               debugging.
////////////////////////////////////////////////////////////////////
void CycleData::
output(ostream &out) const {
  out << get_parent_type() << "::CData";
}
