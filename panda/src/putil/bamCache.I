/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamCache.I
 * @author drose
 * @date 2006-06-09
 */

/**
 * Changes the state of the active flag.  "active" means that the cache should
 * be consulted automatically on loads, "not active" means that objects should
 * be loaded directly without consulting the cache.
 *
 * This represents the global flag.  Also see the individual cache_models,
 * cache_textures, cache_compressed_textures flags.
 */
INLINE void BamCache::
set_active(bool active) {
  ReMutexHolder holder(_lock);
  _active = active;
}

/**
 * Returns true if the BamCache is currently active, false if it is not.
 * "active" means that the cache should be consulted automatically on loads,
 * "not active" means that objects should be loaded directly without
 * consulting the cache.
 *
 * This represents the global flag.  Also see the individual cache_models,
 * cache_textures, cache_compressed_textures flags.
 */
INLINE bool BamCache::
get_active() const {
  ReMutexHolder holder(_lock);
  return _active;
}

/**
 * Indicates whether model files (e.g.  egg files and bam files) will be
 * stored in the cache, as bam files.
 */
INLINE void BamCache::
set_cache_models(bool flag) {
  ReMutexHolder holder(_lock);
  _cache_models = flag;
}

/**
 * Returns whether model files (e.g.  egg files and bam files) will be stored
 * in the cache, as bam files.
 *
 * This also returns false if get_active() is false.
 */
INLINE bool BamCache::
get_cache_models() const {
  ReMutexHolder holder(_lock);
  return _cache_models && _active;
}

/**
 * Indicates whether texture files will be stored in the cache, as
 * uncompressed txo files.
 */
INLINE void BamCache::
set_cache_textures(bool flag) {
  ReMutexHolder holder(_lock);
  _cache_textures = flag;
}

/**
 * Returns whether texture files (e.g.  egg files and bam files) will be
 * stored in the cache, as txo files.
 *
 * This also returns false if get_active() is false.
 */
INLINE bool BamCache::
get_cache_textures() const {
  ReMutexHolder holder(_lock);
  return _cache_textures && _active;
}

/**
 * Indicates whether compressed texture files will be stored in the cache, as
 * compressed txo files.  The compressed data may either be generated in-CPU,
 * via the squish library, or it may be extracted from the GSG after the
 * texture has been loaded.
 *
 * This may be set in conjunction with set_cache_textures(), or independently
 * of it.  If set_cache_textures() is true and this is false, all textures
 * will be cached in their uncompressed form.  If set_cache_textures() is
 * false and this is true, only compressed textures will be cached, and they
 * will be cached in their compressed form.  If both are true, all textures
 * will be cached, in their uncompressed or compressed form appropriately.
 */
INLINE void BamCache::
set_cache_compressed_textures(bool flag) {
  ReMutexHolder holder(_lock);
  _cache_compressed_textures = flag;
}

/**
 * Returns whether compressed texture files will be stored in the cache, as
 * compressed txo files.  See set_cache_compressed_textures().
 *
 * This also returns false if get_active() is false.
 */
INLINE bool BamCache::
get_cache_compressed_textures() const {
  ReMutexHolder holder(_lock);
  return _cache_compressed_textures && _active;
}

/**
 * Indicates whether compiled shader programs will be stored in the cache, as
 * binary .sho files.  This may not be supported by all shader languages or
 * graphics renderers.
 */
INLINE void BamCache::
set_cache_compiled_shaders(bool flag) {
  ReMutexHolder holder(_lock);
  _cache_compiled_shaders = flag;
}

/**
 * Returns whether compiled shader programs will be stored in the cache, as
 * binary .txo files.  See set_cache_compiled_shaders().
 *
 * This also returns false if get_active() is false.
 */
INLINE bool BamCache::
get_cache_compiled_shaders() const {
  ReMutexHolder holder(_lock);
  return _cache_compiled_shaders && _active;
}

/**
 * Returns the current root pathname of the cache.  See set_root().
 */
INLINE Filename BamCache::
get_root() const {
  ReMutexHolder holder(_lock);
  return _root;
}

/**
 * Specifies the time in seconds between automatic flushes of the cache index.
 */
INLINE void BamCache::
set_flush_time(int flush_time) {
  ReMutexHolder holder(_lock);
  _flush_time = flush_time;
}

/**
 * Returns the time in seconds between automatic flushes of the cache index.
 */
INLINE int BamCache::
get_flush_time() const {
  ReMutexHolder holder(_lock);
  return _flush_time;
}

/**
 * Specifies the maximum size, in kilobytes, which the cache is allowed to
 * grow to.  If a newly cached file would exceed this size, an older file is
 * removed from the cache.
 *
 * Note that in the case of multiple different processes simultaneously
 * operating on the same cache directory, the actual cache size may slightly
 * exceed this value from time to time due to latency in checking between the
 * processes.
 */
INLINE void BamCache::
set_cache_max_kbytes(int max_kbytes) {
  ReMutexHolder holder(_lock);
  _max_kbytes = max_kbytes;
  check_cache_size();
}

/**
 * Returns the maximum size, in kilobytes, which the cache is allowed to grow
 * to.  See set_cache_max_kbytes().
 */
INLINE int BamCache::
get_cache_max_kbytes() const {
  ReMutexHolder holder(_lock);
  return _max_kbytes;
}

/**
 * Can be used to put the cache in read-only mode, or take it out of read-only
 * mode.  Note that if you put it into read-write mode, and it discovers that
 * it does not have write access, it will put itself right back into read-only
 * mode.
 */
INLINE void BamCache::
set_read_only(bool ro) {
  ReMutexHolder holder(_lock);
  _read_only = ro;
}

/**
 * Returns true if the cache is in read-only mode.  Normally, the cache starts
 * in read-write mode.  It can put itself into read-only mode automatically if
 * it discovers that it does not have write access to the cache.
 */
INLINE bool BamCache::
get_read_only() const {
  ReMutexHolder holder(_lock);
  return _read_only;
}

/**
 * Returns a pointer to the global BamCache object, which is used
 * automatically by the ModelPool and TexturePool.
 */
INLINE BamCache *BamCache::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    make_global();
  }
  return _global_ptr;
}

/**
 * If there is a global BamCache object, calls consider_flush_index() on it.
 */
INLINE void BamCache::
consider_flush_global_index() {
  if (_global_ptr != nullptr) {
    _global_ptr->consider_flush_index();
  }
}

/**
 * If there is a global BamCache object, calls flush_index() on it.
 */
INLINE void BamCache::
flush_global_index() {
  if (_global_ptr != nullptr) {
    _global_ptr->flush_index();
  }
}

/**
 * Indicates that the index has been modified and will need to be written to
 * disk eventually.
 */
INLINE void BamCache::
mark_index_stale() {
  if (_index_stale_since == 0) {
    _index_stale_since = time(nullptr);
  }
}
