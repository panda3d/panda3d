/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamCacheRecord.I
 * @author drose
 * @date 2006-06-09
 */

/**
 * Returns a duplicate of the BamCacheRecord.  The duplicate will not have a
 * data pointer set, even though one may have been assigned to the original
 * via set_data().
 */
INLINE PT(BamCacheRecord) BamCacheRecord::
make_copy() const {
  return new BamCacheRecord(*this);
}

/**
 * Returns true if the record matches the other record in those attributes
 * which get written to disk.  Does not compare the data pointer.
 */
INLINE bool BamCacheRecord::
operator == (const BamCacheRecord &other) const {
  return (_source_pathname == other._source_pathname &&
          _cache_filename == other._cache_filename &&
          _recorded_time == other._recorded_time &&
          _record_size == other._record_size);
}

/**
 * Returns the full pathname to the source file that originally generated this
 * cache request.  In some cases, for instance in the case of a of a multipage
 * texture like "cube_#.png", this may not not a true filename on disk.
 */
INLINE const Filename &BamCacheRecord::
get_source_pathname() const {
  return _source_pathname;
}

/**
 * Returns the name of the cache file as hashed from the source_pathname.
 * This will be relative to the root of the cache directory, and it will not
 * include any suffixes that may be appended to resolve hash conflicts.
 */
INLINE const Filename &BamCacheRecord::
get_cache_filename() const {
  return _cache_filename;
}

/**
 * Returns the file timestamp of the original source file that generated this
 * cache record, if available.  In some cases the original file timestamp is
 * not available, and this will return 0.
 */
INLINE time_t BamCacheRecord::
get_source_timestamp() const {
  return _source_timestamp;
}

/**
 * Returns the time at which this particular record was recorded or updated.
 */
INLINE time_t BamCacheRecord::
get_recorded_time() const {
  return _recorded_time;
}

/**
 * Returns the number of source files that contribute to the cache.
 */
INLINE int BamCacheRecord::
get_num_dependent_files() const {
  return _files.size();
}

/**
 * Returns the full pathname of the nth source files that contributes to the
 * cache.
 */
INLINE const Filename &BamCacheRecord::
get_dependent_pathname(int n) const {
  nassertr(n >= 0 && n < (int)_files.size(), _files[0]._pathname);
  return _files[n]._pathname;
}

/**
 * Returns true if this cache record has an in-memory data object associated--
 * that is, the object stored in the cache.
 */
INLINE bool BamCacheRecord::
has_data() const {
  return (_ptr != nullptr);
}

/**
 * Removes the in-memory data object associated with this record, if any.
 * This does not affect the on-disk representation of the record.
 */
INLINE void BamCacheRecord::
clear_data() {
  if (_ref_ptr != nullptr) {
    unref_delete(_ref_ptr);
  }

  _ptr = nullptr;
  _ref_ptr = nullptr;
}

/**
 * Returns a pointer to the data stored in the record, or NULL if there is no
 * data.  The pointer is not removed from the record.
 */
INLINE TypedWritable *BamCacheRecord::
get_data() const {
  return _ptr;
}

/**
 * Fills ptr and ref_ptr with the two different-typed pointers to the same
 * object, the data stored within this record.  This transfers ownership of
 * the data pointer; the caller will be responsible for managing the reference
 * counts on this object subsequently.
 *
 * Returns true if the record contained any data (and the pointers have been
 * filled), false if it didn't (and the pointers are NULL).
 */
INLINE bool BamCacheRecord::
extract_data(TypedWritable *&ptr, ReferenceCount *&ref_ptr) {
  ptr = _ptr;
  ref_ptr = _ref_ptr;
  clear_data();
  return (ptr != nullptr);
}

/**
 * Stores a new data object on the record.  You should pass the same pointer
 * twice, to both parameters; this allows the C++ typecasting to automatically
 * convert the pointer into both a TypedWritable and a ReferenceCount pointer,
 * so that the BamCacheRecord object can reliably manage the reference counts.
 *
 * You may pass 0 or NULL as the second parameter.  If you do this, the
 * BamCacheRecord will not manage the object's reference count; it will be up
 * to you to ensure the object is not deleted during the lifetime of the
 * BamCacheRecord object.
 */
INLINE void BamCacheRecord::
set_data(TypedWritable *ptr, ReferenceCount *ref_ptr) {
  if (_ptr != ptr) {
    clear_data();
    _ptr = ptr;
    _ref_ptr = ref_ptr;
    if (_ref_ptr != nullptr) {
      _ref_ptr->ref();
    }
  }
}

/**
 * This variant on set_data() is provided to easily pass objects deriving from
 * TypedWritable.
 */
INLINE void BamCacheRecord::
set_data(TypedWritable *ptr) {
  set_data(ptr, ptr->as_reference_count());
}

/**
 * This variant on set_data() is provided to easily pass objects deriving from
 * TypedWritableReferenceCount.
 */
INLINE void BamCacheRecord::
set_data(TypedWritableReferenceCount *ptr) {
  set_data((TypedWritable *)ptr, (ReferenceCount *)ptr);
}

/**
 * This variant on set_data() is provided just to allow Python code to pass a
 * 0 as the second parameter.
 */
INLINE void BamCacheRecord::
set_data(TypedWritable *ptr, int dummy) {
  nassertv(dummy == 0);
  set_data(ptr, nullptr);
}

/**
 * Returns true if a sorts before b, false otherwise.
 */
INLINE bool BamCacheRecord::SortByAccessTime::
operator () (const BamCacheRecord *a, const BamCacheRecord *b) const {
  return (a->_record_access_time < b->_record_access_time);
}
