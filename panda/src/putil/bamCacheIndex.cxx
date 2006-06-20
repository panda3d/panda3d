// Filename: bamCacheIndex.cxx
// Created by:  drose (19Jun06)
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

#include "bamCacheIndex.h"
#include "indent.h"

TypeHandle BamCacheIndex::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BamCacheIndex::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BamCacheIndex::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "BamCacheIndex, " << _records.size() << " records:\n";

  Records::const_iterator ri;
  for (ri = _records.begin(); ri != _records.end(); ++ri) {
    PT(BamCacheRecord) record = (*ri).second;
    indent(out, indent_level + 2)
      << setw(10) << record->_record_size << " "
      << record->get_cache_filename() << " "
      << record->get_source_pathname() << "\n";
  }
  out << "\n";
  indent(out, indent_level)
    << setw(12) << _cache_size << " bytes total\n";
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheIndex::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               BamCacheRecord.
////////////////////////////////////////////////////////////////////
void BamCacheIndex::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheIndex::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void BamCacheIndex::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  dg.add_uint32(_records.size());
  Records::const_iterator ri;
  for (ri = _records.begin(); ri != _records.end(); ++ri) {
    manager->write_pointer(dg, (*ri).second);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheIndex::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type BamCacheIndex is encountered
//               in the Bam file.  It should create the BamCacheIndex
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *BamCacheIndex::
make_from_bam(const FactoryParams &params) {
  BamCacheIndex *object = new BamCacheIndex;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheIndex::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int BamCacheIndex::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritable::complete_pointers(p_list, manager);

  RecordVector::iterator vi;
  for (vi = _record_vector.begin(); vi != _record_vector.end(); ++vi) {
    PT(BamCacheRecord) record = DCAST(BamCacheRecord, p_list[pi++]);
    (*vi) = record;

    bool inserted = _records.insert(Records::value_type(record->get_source_pathname(), record)).second;
    if (!inserted) {
      util_cat.info()
        << "Multiple cache files defining " << record->get_source_pathname()
        << " in index.\n";
    } else {
      _cache_size += record->_record_size;
    }
  }

  _record_vector.clear();

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheIndex::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new BamCacheIndex.
////////////////////////////////////////////////////////////////////
void BamCacheIndex::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  int num_records = scan.get_uint32();
  _record_vector.reserve(num_records);
  for (int i = 0; i < num_records; ++i) {
    _record_vector.push_back(NULL);
    manager->read_pointer(scan);
  }
}
