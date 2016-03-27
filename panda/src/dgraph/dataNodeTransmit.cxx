/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dataNodeTransmit.cxx
 * @author drose
 * @date 2002-03-11
 */

#include "dataNodeTransmit.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle DataNodeTransmit::_type_handle;

/**
 *
 */
DataNodeTransmit::
~DataNodeTransmit() {
}

/**
 * Ensures that the given index number exists in the data array.
 */
void DataNodeTransmit::
slot_data(int index) {
  nassertv(index < 1000);
  while (index >= (int)_data.size()) {
    _data.push_back(EventParameter());
  }
}

/**
 * Tells the BamReader how to create objects of type Lens.
 */
void DataNodeTransmit::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void DataNodeTransmit::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  dg.add_uint16(_data.size());
  Data::const_iterator di;
  for (di = _data.begin(); di != _data.end(); ++di) {
    const EventParameter &param = (*di);
    TypedWritableReferenceCount *ptr = param.get_ptr();
    manager->write_pointer(dg, ptr);
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int DataNodeTransmit::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritable::complete_pointers(p_list, manager);

  Data::iterator di;
  for (di = _data.begin(); di != _data.end(); ++di) {
    (*di) = EventParameter(DCAST(TypedWritableReferenceCount, p_list[pi++]));
  }

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Lens is encountered in the Bam file.  It should create the Lens and
 * extract its information from the file.
 */
TypedWritable *DataNodeTransmit::
make_from_bam(const FactoryParams &params) {
  DataNodeTransmit *xmit = new DataNodeTransmit;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  xmit->fillin(scan, manager);

  return xmit;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new Lens.
 */
void DataNodeTransmit::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  int num_params = scan.get_uint16();
  _data.reserve(num_params);
  for (int i = 0; i < num_params; i++) {
    manager->read_pointer(scan);
    _data.push_back(EventParameter());
  }
}
