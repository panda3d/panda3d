// Filename: bamCache.I
// Created by:  drose (09Jun06)
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


////////////////////////////////////////////////////////////////////
//     Function: BamCache::set_active
//       Access: Published
//  Description: Changes the state of the active flag.  "active" means
//               that the cache should be consulted automatically on
//               loads, "not active" means that objects should be
//               loaded directly without consulting the cache.
////////////////////////////////////////////////////////////////////
INLINE void BamCache::
set_active(bool active) {
  _active = active;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::get_active
//       Access: Published
//  Description: Returns true if the BamCache is currently active,
//               false if it is not.  "active" means that the cache
//               should be consulted automatically on loads, "not
//               active" means that objects should be loaded directly
//               without consulting the cache.
////////////////////////////////////////////////////////////////////
INLINE bool BamCache::
get_active() const {
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::get_root
//       Access: Published
//  Description: Returns the current root pathname of the cache.  See
//               set_root().
////////////////////////////////////////////////////////////////////
INLINE const Filename &BamCache::
get_root() const {
  return _root;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::set_flush_time
//       Access: Published
//  Description: Specifies the time in seconds between automatic
//               flushes of the cache index.
////////////////////////////////////////////////////////////////////
INLINE void BamCache::
set_flush_time(int flush_time) {
  _flush_time = flush_time;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::get_flush_time
//       Access: Published
//  Description: Returns the time in seconds between automatic
//               flushes of the cache index.
////////////////////////////////////////////////////////////////////
INLINE int BamCache::
get_flush_time() const {
  return _flush_time;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::set_cache_max_kbytes
//       Access: Published
//  Description: Specifies the maximum size, in kilobytes, which the
//               cache is allowed to grow to.  If a newly cached file
//               would exceed this size, an older file is removed from
//               the cache.
//
//               Note that in the case of multiple different processes
//               simultaneously operating on the same cache directory,
//               the actual cache size may slightly exceed this value
//               from time to time due to latency in checking between
//               the processes.
////////////////////////////////////////////////////////////////////
INLINE void BamCache::
set_cache_max_kbytes(int max_kbytes) {
  _max_kbytes = max_kbytes;
  check_cache_size();
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::get_cache_max_kbytes
//       Access: Published
//  Description: Returns the maximum size, in kilobytes, which the
//               cache is allowed to grow to.  See
//               set_cache_max_kbytes().
////////////////////////////////////////////////////////////////////
INLINE int BamCache::
get_cache_max_kbytes() const {
  return _max_kbytes;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::set_read_only
//       Access: Published
//  Description: Can be used to put the cache in read-only mode,
//               or take it out of read-only mode.  Note that if you
//               put it into read-write mode, and it discovers that
//               it does not have write access, it will put itself
//               right back into read-only mode.
////////////////////////////////////////////////////////////////////
INLINE void BamCache::
set_read_only(bool ro) {
  _read_only = ro;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::get_read_only
//       Access: Published
//  Description: Returns true if the cache is in read-only mode.
//               Normally, the cache starts in read-write mode.  It
//               can put itself into read-only mode automatically if
//               it discovers that it does not have write access to
//               the cache.
////////////////////////////////////////////////////////////////////
INLINE bool BamCache::
get_read_only() const {
  return _read_only;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::get_global_ptr
//       Access: Published, Static
//  Description: Returns a pointer to the global BamCache object,
//               which is used automatically by the ModelPool and
//               TexturePool.
////////////////////////////////////////////////////////////////////
INLINE BamCache *BamCache::
get_global_ptr() {
  if (_global_ptr == (BamCache *)NULL) {
    make_global();
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::mark_index_stale
//       Access: Private
//  Description: Indicates that the index has been modified and will
//               need to be written to disk eventually.
////////////////////////////////////////////////////////////////////
INLINE void BamCache::
mark_index_stale() {
  if (_index_stale_since == 0) {
    _index_stale_since = time(NULL);
  }
}
