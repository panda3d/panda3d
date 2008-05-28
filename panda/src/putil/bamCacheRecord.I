// Filename: bamCacheRecord.I
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


////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::make_copy
//       Access: Published
//  Description: Returns a duplicate of the BamCacheRecord.  The
//               duplicate will not have a data pointer set, even
//               though one may have been assigned to the original via
//               set_data().
////////////////////////////////////////////////////////////////////
INLINE PT(BamCacheRecord) BamCacheRecord::
make_copy() const {
  return new BamCacheRecord(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::operator == 
//       Access: Published
//  Description: Returns true if the record matches the other record
//               in those attributes which get written to disk.  Does
//               not compare the data pointer.
////////////////////////////////////////////////////////////////////
INLINE bool BamCacheRecord::
operator == (const BamCacheRecord &other) const {
  return (_source_pathname == other._source_pathname &&
          _cache_filename == other._cache_filename &&
          _recorded_time == other._recorded_time &&
          _record_size == other._record_size);
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::get_source_pathname
//       Access: Published
//  Description: Returns the full pathname to the source file that
//               originally generated this cache request.  In some
//               cases, for instance in the case of a of a multipage
//               texture like "cube_#.png", this may not not a true
//               filename on disk.
////////////////////////////////////////////////////////////////////
INLINE const Filename &BamCacheRecord::
get_source_pathname() const {
  return _source_pathname;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::get_cache_filename
//       Access: Published
//  Description: Returns the name of the cache file as hashed from the
//               source_pathname.  This will be relative to the root
//               of the cache directory, and it will not include any
//               suffixes that may be appended to resolve hash
//               conflicts.
////////////////////////////////////////////////////////////////////
INLINE const Filename &BamCacheRecord::
get_cache_filename() const {
  return _cache_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::get_recorded_time
//       Access: Published
//  Description: Returns the time at which this particular record was
//               recorded or updated.
////////////////////////////////////////////////////////////////////
INLINE time_t BamCacheRecord::
get_recorded_time() const {
  return _recorded_time;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::get_num_dependent_files
//       Access: Published
//  Description: Returns the number of source files that contribute to
//               the cache.
////////////////////////////////////////////////////////////////////
INLINE int BamCacheRecord::
get_num_dependent_files() const {
  return _files.size();
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::get_dependent_pathname
//       Access: Published
//  Description: Returns the full pathname of the nth source files
//               that contributes to the cache.
////////////////////////////////////////////////////////////////////
INLINE const Filename &BamCacheRecord::
get_dependent_pathname(int n) const {
  nassertr(n >= 0 && n < (int)_files.size(), _files[0]._pathname);
  return _files[n]._pathname;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::has_data
//       Access: Published
//  Description: Returns true if this cache record has an in-memory
//               data object associated--that is, the object stored in
//               the cache.
////////////////////////////////////////////////////////////////////
INLINE bool BamCacheRecord::
has_data() const {
  return (_data != (TypedWritable *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::clear_data
//       Access: Published
//  Description: Removes the in-memory data object associated with
//               this record, if any.  This does not affect the
//               on-disk representation of the record.
////////////////////////////////////////////////////////////////////
INLINE void BamCacheRecord::
clear_data() {
  if (_owns_pointer) {
    delete _data;
  }
  _data = NULL;
  _owns_pointer = false;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::get_data
//       Access: Published
//  Description: Returns a read-only pointer to the data stored in the
//               record, or NULL if there is no data.  This does not
//               change ownership of the data pointer.
////////////////////////////////////////////////////////////////////
INLINE const TypedWritable *BamCacheRecord::
get_data() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::extract_data
//       Access: Published
//  Description: Removes the in-memory data object associated with
//               this record, and returns it.  This transfers
//               ownership of the data pointer; the caller will be
//               responsible for subsequently deleting this object.
//
//               It is an error to call this if the record does not
//               own the pointer.
////////////////////////////////////////////////////////////////////
INLINE TypedWritable *BamCacheRecord::
extract_data() {
  nassertr(_owns_pointer, NULL);
  TypedWritable *result = _data;
  _data = NULL;
  _owns_pointer = false;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::set_data
//       Access: Published
//  Description: Stores a new data object on the record.  If
//               owns_pointer is true, the record owns the pointer,
//               and will eventually be responsible for deleting it.
////////////////////////////////////////////////////////////////////
INLINE void BamCacheRecord::
set_data(TypedWritable *data, bool owns_pointer) {
  clear_data();
  _data = data;
  _owns_pointer = owns_pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCacheRecord::SortByAccessTime::operator ()
//       Access: Public
//  Description: Returns true if a sorts before b, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool BamCacheRecord::SortByAccessTime::
operator () (const BamCacheRecord *a, const BamCacheRecord *b) const {
  return (a->_record_access_time < b->_record_access_time);
}
