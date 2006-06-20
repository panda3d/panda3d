// Filename: bamCache.h
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

#ifndef BAMCACHE_H
#define BAMCACHE_H

#include "pandabase.h"
#include "bamCacheRecord.h"
#include "pointerTo.h"
#include "filename.h"
#include "pmap.h"
#include "pvector.h"

class BamCacheIndex;

////////////////////////////////////////////////////////////////////
//       Class : BamCache
// Description : This class maintains a cache of Bam and/or Txo
//               objects generated from model files and texture images
//               (as well as possibly other kinds of loadable objects
//               that can be stored in bam file format).
//
//               This class also maintains a persistent index that
//               lists all of the cached objects (see BamCacheIndex).
//               We go through some considerable effort to make sure
//               this index gets saved correctly to disk, even in the
//               presence of multiple different processes writing to
//               the same index, and without relying too heavily on
//               low-level os-provided file locks (which work poorly
//               with C++ iostreams).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BamCache {
PUBLISHED:
  BamCache();
  ~BamCache();

  INLINE void set_active(bool flag);
  INLINE bool get_active() const;

  void set_root(const Filename &root);
  INLINE const Filename &get_root() const;

  INLINE void set_flush_time(int flush_time);
  INLINE int get_flush_time() const;

  INLINE void set_cache_max_kbytes(int max_kbytes);
  INLINE int get_cache_max_kbytes() const;

  PT(BamCacheRecord) lookup(const Filename &source_filename, 
                            const string &cache_extension);
  bool store(BamCacheRecord *record);

  void consider_flush_index();
  void flush_index();

  INLINE static BamCache *get_global_ptr();

private:
  void read_index();
  bool read_index_pathname(Filename &index_pathname,
                           string &index_ref_contents) const;
  void merge_index(BamCacheIndex *new_index);
  void rebuild_index();
  INLINE void mark_index_stale();

  void add_to_index(const BamCacheRecord *record);
  void remove_from_index(const Filename &source_filename);

  void check_cache_size();

  static BamCacheIndex *do_read_index(Filename &index_pathname);
  static bool do_write_index(Filename &index_pathname, const BamCacheIndex *index);

  PT(BamCacheRecord) find_and_read_record(const Filename &source_pathname,
                                          const Filename &cache_filename);
  PT(BamCacheRecord) read_record(const Filename &source_pathname,
                                 const Filename &cache_filename,
                                 int pass);
  static PT(BamCacheRecord) do_read_record(Filename &cache_pathname, bool read_data);

  static string hash_filename(const string &filename);
  static void make_global();

  bool _active;
  Filename _root;
  int _flush_time;
  int _max_kbytes;
  static BamCache *_global_ptr;

  BamCacheIndex *_index;
  time_t _index_stale_since;

  Filename _index_pathname;
  string _index_ref_contents;
};

#include "bamCache.I"

#endif
