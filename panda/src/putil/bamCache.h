/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamCache.h
 * @author drose
 * @date 2006-06-09
 */

#ifndef BAMCACHE_H
#define BAMCACHE_H

#include "pandabase.h"
#include "bamCacheRecord.h"
#include "pointerTo.h"
#include "filename.h"
#include "pmap.h"
#include "pvector.h"
#include "reMutex.h"
#include "reMutexHolder.h"

#include <time.h>

class BamCacheIndex;

/**
 * This class maintains a cache of Bam and/or Txo objects generated from model
 * files and texture images (as well as possibly other kinds of loadable
 * objects that can be stored in bam file format).
 *
 * This class also maintains a persistent index that lists all of the cached
 * objects (see BamCacheIndex). We go through some considerable effort to make
 * sure this index gets saved correctly to disk, even in the presence of
 * multiple different processes writing to the same index, and without relying
 * too heavily on low-level os-provided file locks (which work poorly with C++
 * iostreams).
 */
class EXPCL_PANDA_PUTIL BamCache {
PUBLISHED:
  BamCache();
  ~BamCache();

  INLINE void set_active(bool flag);
  INLINE bool get_active() const;

  INLINE void set_cache_models(bool flag);
  INLINE bool get_cache_models() const;

  INLINE void set_cache_textures(bool flag);
  INLINE bool get_cache_textures() const;

  INLINE void set_cache_compressed_textures(bool flag);
  INLINE bool get_cache_compressed_textures() const;

  INLINE void set_cache_compiled_shaders(bool flag);
  INLINE bool get_cache_compiled_shaders() const;

  void set_root(const Filename &root);
  INLINE Filename get_root() const;

  INLINE void set_flush_time(int flush_time);
  INLINE int get_flush_time() const;

  INLINE void set_cache_max_kbytes(int max_kbytes);
  INLINE int get_cache_max_kbytes() const;

  INLINE void set_read_only(bool ro);
  INLINE bool get_read_only() const;

  PT(BamCacheRecord) lookup(const Filename &source_filename,
                            const std::string &cache_extension);
  bool store(BamCacheRecord *record);

  void consider_flush_index();
  void flush_index();

  void list_index(std::ostream &out, int indent_level = 0) const;

  INLINE static BamCache *get_global_ptr();
  INLINE static void consider_flush_global_index();
  INLINE static void flush_global_index();

PUBLISHED:
  MAKE_PROPERTY(active, get_active, set_active);
  MAKE_PROPERTY(cache_models, get_cache_models, set_cache_models);
  MAKE_PROPERTY(cache_textures, get_cache_textures, set_cache_textures);
  MAKE_PROPERTY(cache_compressed_textures, get_cache_compressed_textures,
                                           set_cache_compressed_textures);
  MAKE_PROPERTY(cache_compiled_shaders, get_cache_compiled_shaders,
                                        set_cache_compiled_shaders);
  MAKE_PROPERTY(root, get_root, set_root);
  MAKE_PROPERTY(flush_time, get_flush_time, set_flush_time);
  MAKE_PROPERTY(cache_max_kbytes, get_cache_max_kbytes, set_cache_max_kbytes);
  MAKE_PROPERTY(read_only, get_read_only, set_read_only);

private:
  void read_index();
  bool read_index_pathname(Filename &index_pathname,
                           std::string &index_ref_contents) const;
  void merge_index(BamCacheIndex *new_index);
  void rebuild_index();
  INLINE void mark_index_stale();

  void add_to_index(const BamCacheRecord *record);
  void remove_from_index(const Filename &source_filename);

  void check_cache_size();

  void emergency_read_only();

  static BamCacheIndex *do_read_index(const Filename &index_pathname);
  static bool do_write_index(const Filename &index_pathname, const BamCacheIndex *index);

  PT(BamCacheRecord) find_and_read_record(const Filename &source_pathname,
                                          const Filename &cache_filename);
  PT(BamCacheRecord) read_record(const Filename &source_pathname,
                                 const Filename &cache_filename,
                                 int pass);
  static PT(BamCacheRecord) do_read_record(const Filename &cache_pathname,
                                           bool read_data);

  static std::string hash_filename(const std::string &filename);
  static void make_global();

  bool _active;
  bool _cache_models;
  bool _cache_textures;
  bool _cache_compressed_textures;
  bool _cache_compiled_shaders;
  bool _read_only;
  Filename _root;
  int _flush_time;
  int _max_kbytes;
  static BamCache *_global_ptr;

  BamCacheIndex *_index;
  time_t _index_stale_since;

  Filename _index_pathname;
  std::string _index_ref_contents;

  ReMutex _lock;
};

#include "bamCache.I"

#endif
