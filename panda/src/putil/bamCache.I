// Filename: bamCache.I
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
//     Function: BamCache::set_active
//       Access: Published
//  Description: Changes the state of the active flag.  "active" means
//               that the cache should be consulted automatically on
//               loads, "not active" means that objects should be
//               loaded directly without consulting the cache.
//
//               This represents the global flag.  Also see the
//               individual cache_models, cache_textures,
//               cache_compressed_textures flags.
////////////////////////////////////////////////////////////////////
INLINE void BamCache::
set_active(bool active) {
  ReMutexHolder holder(_lock);
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
//
//               This represents the global flag.  Also see the
//               individual cache_models, cache_textures,
//               cache_compressed_textures flags.
////////////////////////////////////////////////////////////////////
INLINE bool BamCache::
get_active() const {
  ReMutexHolder holder(_lock);
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::set_cache_models
//       Access: Published
//  Description: Indicates whether model files (e.g. egg files and bam
//               files) will be stored in the cache, as bam files.
////////////////////////////////////////////////////////////////////
INLINE void BamCache::
set_cache_models(bool flag) {
  ReMutexHolder holder(_lock);
  _cache_models = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::get_cache_models
//       Access: Published
//  Description: Returns whether model files (e.g. egg files and bam
//               files) will be stored in the cache, as bam files.
//
//               This also returns false if get_active() is false.
////////////////////////////////////////////////////////////////////
INLINE bool BamCache::
get_cache_models() const {
  ReMutexHolder holder(_lock);
  return _cache_models && _active;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::set_cache_textures
//       Access: Published
//  Description: Indicates whether texture files will be stored in the
//               cache, as uncompressed txo files.
////////////////////////////////////////////////////////////////////
INLINE void BamCache::
set_cache_textures(bool flag) {
  ReMutexHolder holder(_lock);
  _cache_textures = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::get_cache_textures
//       Access: Published
//  Description: Returns whether texture files (e.g. egg files and bam
//               files) will be stored in the cache, as txo files.
//
//               This also returns false if get_active() is false.
////////////////////////////////////////////////////////////////////
INLINE bool BamCache::
get_cache_textures() const {
  ReMutexHolder holder(_lock);
  return _cache_textures && _active;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::set_cache_compressed_textures
//       Access: Published
//  Description: Indicates whether compressed texture files will be
//               stored in the cache, as compressed txo files.  The
//               compressed data may either be generated in-CPU, via
//               the squish library, or it may be extracted from the
//               GSG after the texture has been loaded.
//
//               This may be set in conjunction with
//               set_cache_textures(), or independently of it.  If
//               set_cache_textures() is true and this is false, all
//               textures will be cached in their uncompressed form.
//               If set_cache_textures() is false and this is true,
//               only compressed textures will be cached, and they
//               will be cached in their compressed form.  If both are
//               true, all textures will be cached, in their
//               uncompressed or compressed form appropriately.
////////////////////////////////////////////////////////////////////
INLINE void BamCache::
set_cache_compressed_textures(bool flag) {
  ReMutexHolder holder(_lock);
  _cache_compressed_textures = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::get_cache_compressed_textures
//       Access: Published
//  Description: Returns whether compressed texture files will be
//               stored in the cache, as compressed txo files.  See
//               set_cache_compressed_textures().
//
//               This also returns false if get_active() is false.
////////////////////////////////////////////////////////////////////
INLINE bool BamCache::
get_cache_compressed_textures() const {
  ReMutexHolder holder(_lock);
  return _cache_compressed_textures && _active;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::get_root
//       Access: Published
//  Description: Returns the current root pathname of the cache.  See
//               set_root().
////////////////////////////////////////////////////////////////////
INLINE Filename BamCache::
get_root() const {
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
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
