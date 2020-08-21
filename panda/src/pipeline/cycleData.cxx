/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cycleData.cxx
 * @author drose
 * @date 2002-02-21
 */

#include "cycleData.h"

#ifdef DO_PIPELINING
TypeHandle CycleData::_type_handle;
#endif

/**
 *
 */
CycleData::
~CycleData() {
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void CycleData::
write_datagram(BamWriter *, Datagram &) const {
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void CycleData::
write_datagram(BamWriter *, Datagram &, void *) const {
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int CycleData::
complete_pointers(TypedWritable **, BamReader *) {
  return 0;
}

/**
 * This internal function is intended to be called by each class's
 * make_from_bam() method to read in all of the relevant data from the BamFile
 * for the new object.
 */
void CycleData::
fillin(DatagramIterator &, BamReader *) {
}


/**
 * This internal function is intended to be called by each class's
 * make_from_bam() method to read in all of the relevant data from the BamFile
 * for the new object.
 */
void CycleData::
fillin(DatagramIterator &, BamReader *, void *) {
}

/**
 * Returns the type of the container that owns the CycleData.  This is useful
 * mainly for debugging.
 */
TypeHandle CycleData::
get_parent_type() const {
  return TypeHandle::none();
}

/**
 * Formats the contents of the CycleData in some meaningful way for humans.
 * This is useful mainly for debugging.
 */
void CycleData::
output(std::ostream &out) const {
  out << get_parent_type() << "::CData";
}
