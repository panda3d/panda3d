/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamCacheIndex.cxx
 * @author drose
 * @date 2006-06-19
 */

#include "bamCacheIndex.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "config_putil.h" // util_cat
#include "indent.h"
#include <algorithm>

TypeHandle BamCacheIndex::_type_handle;


/**
 *
 */
BamCacheIndex::
~BamCacheIndex() {
#ifndef NDEBUG
  // We need to "empty" the linked list to make the LinkedListNode destructors
  // happy.
  release_records();
#endif
}

/**
 *
 */
void BamCacheIndex::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "BamCacheIndex, " << _records.size() << " records:\n";

  Records::const_iterator ri;
  for (ri = _records.begin(); ri != _records.end(); ++ri) {
    BamCacheRecord *record = (*ri).second;
    indent(out, indent_level + 2)
      << std::setw(10) << record->_record_size << " "
      << record->get_cache_filename() << " "
      << record->get_source_pathname() << "\n";
  }
  out << "\n";
  indent(out, indent_level)
    << std::setw(12) << _cache_size << " bytes total\n";
}

/**
 * Should be called after the _records index has been filled externally, this
 * will sort the records by access time and calculate _cache_size.
 */
void BamCacheIndex::
process_new_records() {
  nassertv(_cache_size == 0);

  // Fill up a vector so we can sort the records into order by access time.
  RecordVector rv;
  rv.reserve(_records.size());

  Records::const_iterator ri;
  for (ri = _records.begin(); ri != _records.end(); ++ri) {
    BamCacheRecord *record = (*ri).second;
    _cache_size += record->_record_size;
    rv.push_back(record);
  }

  sort(rv.begin(), rv.end(), BamCacheRecord::SortByAccessTime());

  // Now put them into the linked list.
  RecordVector::const_iterator rvi;
  for (rvi = rv.begin(); rvi != rv.end(); ++rvi) {
    BamCacheRecord *record = *rvi;
    record->insert_before(this);
  }
}

/**
 * This is the inverse of process_new_records: it releases the records from
 * the linked list, so that they may be added to another index or whatever.
 * Calling this, of course, invalidates the index until process_new_records()
 * is called again.
 */
void BamCacheIndex::
release_records() {
  Records::const_iterator ri;
  for (ri = _records.begin(); ri != _records.end(); ++ri) {
    BamCacheRecord *record = (*ri).second;
    record->_next = nullptr;
    record->_prev = nullptr;
  }
  _next = this;
  _prev = this;
  _cache_size = 0;
}

/**
 * Evicts an old file from the cache.  Records the record.  Returns NULL if
 * the cache is empty.
 */
PT(BamCacheRecord) BamCacheIndex::
evict_old_file() {
  if (_next == this) {
    // Nothing in the cache.
    return nullptr;
  }

  // The first record in the linked list is the least-recently-used one.
  PT(BamCacheRecord) record = (BamCacheRecord *)_next;
  bool removed = remove_record(record->get_source_pathname());
  nassertr(removed, nullptr);

  return record;
}

/**
 * Adds a newly-created BamCacheRecord into the index.  If a matching record
 * is already in the index, it is replaced with the new record.  Returns true
 * if the record was added, or false if the equivalent record was already
 * there and the index is unchanged.
 */
bool BamCacheIndex::
add_record(BamCacheRecord *record) {
  std::pair<Records::iterator, bool> result =
    _records.insert(Records::value_type(record->get_source_pathname(), record));
  if (!result.second) {
    // We already had a record for this filename; it gets replaced.
    BamCacheRecord *orig_record = (*result.first).second;
    orig_record->remove_from_list();
    if (*orig_record == *record) {
      // Well, never mind.  The record hasn't changed.
      orig_record->insert_before(this);
      return false;
    }

    _cache_size -= orig_record->_record_size;
    (*result.first).second = record;
  }
  record->insert_before(this);

  _cache_size += record->_record_size;
  return true;
}

/**
 * Searches for the matching record in the index and removes it if it is
 * found.  Returns true if the record was found and removed, or false if there
 * was no such record and the index is unchanged.
 */
bool BamCacheIndex::
remove_record(const Filename &source_pathname) {
  Records::iterator ri = _records.find(source_pathname);
  if (ri == _records.end()) {
    // No entry for this record; no problem.
    return false;
  }

  BamCacheRecord *record = (*ri).second;
  record->remove_from_list();
  _cache_size -= record->_record_size;
  _records.erase(ri);
  return true;
}

/**
 * Tells the BamReader how to create objects of type BamCacheRecord.
 */
void BamCacheIndex::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BamCacheIndex::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  dg.add_uint32(_records.size());
  Records::const_iterator ri;
  for (ri = _records.begin(); ri != _records.end(); ++ri) {
    manager->write_pointer(dg, (*ri).second);
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type BamCacheIndex is encountered in the Bam file.  It should create the
 * BamCacheIndex and extract its information from the file.
 */
TypedWritable *BamCacheIndex::
make_from_bam(const FactoryParams &params) {
  BamCacheIndex *object = new BamCacheIndex;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
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
    }
  }

  _record_vector.clear();

  process_new_records();

  return pi;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new BamCacheIndex.
 */
void BamCacheIndex::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  int num_records = scan.get_uint32();
  _record_vector.reserve(num_records);
  for (int i = 0; i < num_records; ++i) {
    _record_vector.push_back(nullptr);
    manager->read_pointer(scan);
  }
}
