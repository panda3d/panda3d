// Filename: cycleData.cxx
// Created by:  drose (21Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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

