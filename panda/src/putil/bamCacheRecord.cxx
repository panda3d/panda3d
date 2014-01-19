// Filename: bamCacheRecord.cxx
// Created by:  drose (09Jun06)
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

#include "bamCacheRecord.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "virtualFileSystem.h"
#include "virtualFile.h"
#include "indent.h"
#include "config_util.h" // util_cat

TypeHandle BamCacheRecord::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::Default Constructor
//       Access: Private
//  Description: Used when reading from a bam file.
////////////////////////////////////////////////////////////////////
BamCacheRecord::
BamCacheRecord() :
  _recorded_time(0),
  _record_size(0),
  _source_timestamp(0),
  _ptr(NULL),
  _ref_ptr(NULL),
  _record_access_time(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::Constructor
//       Access: Private
//  Description: Use BamCache::lookup() to create one of these.
////////////////////////////////////////////////////////////////////
BamCacheRecord::
BamCacheRecord(const Filename &source_pathname, 
               const Filename &cache_filename) :
  _source_pathname(source_pathname),
  _cache_filename(cache_filename),
  _recorded_time(0),
  _record_size(0),
  _source_timestamp(0),
  _ptr(NULL),
  _ref_ptr(NULL),
  _record_access_time(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::Copy Constructor
//       Access: Private
//  Description: Use make_copy() to make a copy.  The copy does not
//               share the data pointer.
////////////////////////////////////////////////////////////////////
BamCacheRecord::
BamCacheRecord(const BamCacheRecord &copy) :
  _source_pathname(copy._source_pathname),
  _cache_filename(copy._cache_filename),
  _recorded_time(copy._recorded_time),
  _record_size(copy._record_size),
  _source_timestamp(copy._source_timestamp),
  _ptr(NULL),
  _ref_ptr(NULL),
  _record_access_time(copy._record_access_time)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
BamCacheRecord::
~BamCacheRecord() {
  clear_data();
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::dependents_unchanged
//       Access: Published
//  Description: Returns true if all of the dependent files are still
//               the same as when the cache was recorded, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool BamCacheRecord::
dependents_unchanged() const {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  if (util_cat.is_debug()) {
    util_cat.debug()
      << "Validating dependents for " << get_source_pathname() << "\n";
  }

  DependentFiles::const_iterator fi;
  for (fi = _files.begin(); fi != _files.end(); ++fi) {
    const DependentFile &dfile = (*fi);
    PT(VirtualFile) file = vfs->get_file(dfile._pathname);
    if (file == (VirtualFile *)NULL) {
      // No such file.
      if (dfile._timestamp != 0) {
        if (util_cat.is_debug()) {
          util_cat.debug()
            << dfile._pathname << " does not exist.\n";
        }
        return false;
      }
    } else {
      if (file->get_timestamp() != dfile._timestamp ||
          file->get_file_size() != dfile._size) {
        // File has changed timestamp or size.
        if (util_cat.is_debug()) {
          util_cat.debug()
            << dfile._pathname << " has changed timestamp or size.\n";
        }
        return false;
      }
    }
    
    // Presumably, the file is unchanged.
    if (util_cat.is_debug()) {
      util_cat.debug()
        << dfile._pathname << " is unchanged.\n";
    }
  }

  if (util_cat.is_debug()) {
    util_cat.debug()
      << "Dependents valid.\n";
  }

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::clear_dependent_files
//       Access: Published
//  Description: Empties the list of files that contribute to the data
//               in this record.
////////////////////////////////////////////////////////////////////
void BamCacheRecord::
clear_dependent_files() {
  _files.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::add_dependent_file
//       Access: Published
//  Description: Adds the indicated file to the list of files that
//               will be loaded to generate the data in this record.
//               This should be called once for the primary source
//               file, and again for each secondary source file, if
//               any.
////////////////////////////////////////////////////////////////////
void BamCacheRecord::
add_dependent_file(const Filename &pathname) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  _files.push_back(DependentFile());
  DependentFile &dfile = _files.back();
  dfile._pathname = pathname;
  dfile._pathname.make_absolute();

  PT(VirtualFile) file = vfs->get_file(dfile._pathname);
  if (file == (VirtualFile *)NULL) {
    // No such file.
    dfile._timestamp = 0;
    dfile._size = 0;

  } else {
    dfile._timestamp = file->get_timestamp();
    dfile._size = file->get_file_size();

    if (dfile._pathname == _source_pathname) {
      _source_timestamp = dfile._timestamp;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void BamCacheRecord::
output(ostream &out) const {
  out << "BamCacheRecord " << get_source_pathname();
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void BamCacheRecord::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "BamCacheRecord " << get_source_pathname() << "\n";
  indent(out, indent_level)
    << "source " << format_timestamp(_source_timestamp) << "\n";
  indent(out, indent_level)
    << "recorded " << format_timestamp(_recorded_time) << "\n";

  indent(out, indent_level)
    << _files.size() << " dependent files.\n";
  DependentFiles::const_iterator fi;
  for (fi = _files.begin(); fi != _files.end(); ++fi) {
    const DependentFile &dfile = (*fi);
    indent(out, indent_level + 2)
      << setw(10) << dfile._size << " "
      << format_timestamp(dfile._timestamp) << " "
      << dfile._pathname << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::format_timestamp
//       Access: Private, Static
//  Description: Returns a timestamp value formatted nicely for
//               output.
////////////////////////////////////////////////////////////////////
string BamCacheRecord::
format_timestamp(time_t timestamp) {
  static const size_t buffer_size = 512;
  char buffer[buffer_size];

  if (timestamp == 0) {
    // A zero timestamp is a special case.
    return "  (no date) ";
  }

  time_t now = time(NULL);
  struct tm *tm_p = localtime(&timestamp);

  if (timestamp > now || (now - timestamp > 86400 * 365)) {
    // A timestamp in the future, or more than a year in the past,
    // gets a year appended.
    strftime(buffer, buffer_size, "%b %d  %Y", tm_p);
  } else {
    // Otherwise, within the past year, show the date and time.
    strftime(buffer, buffer_size, "%b %d %H:%M", tm_p);
  }

  return buffer;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::register_with_read_factory
//       Access: Published, Static
//  Description: Tells the BamReader how to create objects of type
//               BamCacheRecord.
////////////////////////////////////////////////////////////////////
void BamCacheRecord::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::write_datagram
//       Access: Published, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void BamCacheRecord::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);
  dg.add_string(_source_pathname);
  dg.add_string(_cache_filename);
  dg.add_uint32(_recorded_time);
  dg.add_uint64(_record_size);

  dg.add_uint16(_files.size());
  DependentFiles::const_iterator fi;
  for (fi = _files.begin(); fi != _files.end(); ++fi) {
    const DependentFile &file = (*fi);
    dg.add_string(file._pathname);
    dg.add_uint32(file._timestamp);
    dg.add_uint64(file._size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type BamCacheRecord is encountered
//               in the Bam file.  It should create the BamCacheRecord
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *BamCacheRecord::
make_from_bam(const FactoryParams &params) {
  BamCacheRecord *object = new BamCacheRecord;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new BamCacheRecord.
////////////////////////////////////////////////////////////////////
void BamCacheRecord::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);

  _source_pathname = scan.get_string();
  _cache_filename = scan.get_string();
  _recorded_time = scan.get_uint32();
  _record_size = scan.get_uint64();

  unsigned int num_files = scan.get_uint16();
  _files.reserve(num_files);
  for (unsigned int i = 0; i < num_files; ++i) {
    _files.push_back(DependentFile());
    DependentFile &file = _files.back();
    file._pathname = scan.get_string();
    file._timestamp = scan.get_uint32();
    file._size = scan.get_uint64();

    // If we come across the original source file (we normally expect
    // to), record that as its timestamp.
    if (file._pathname == _source_pathname) {
      _source_timestamp = file._timestamp;
    }
  }
}
