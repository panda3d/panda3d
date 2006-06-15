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

////////////////////////////////////////////////////////////////////
//       Class : BamCache
// Description : This class maintains a cache of Bam and/or Txo
//               objects generated from model files and texture images
//               (as well as possibly other kinds of loadable objects
//               that can be stored in bam file format).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BamCache {
PUBLISHED:
  BamCache();
  ~BamCache();

  INLINE bool get_active() const;
  INLINE void set_active(bool flag);

  INLINE const Filename &get_root() const;
  void set_root(const Filename &root);

  PT(BamCacheRecord) lookup(const Filename &source_filename, 
                            const string &cache_extension);
  bool store(BamCacheRecord *record);

  INLINE static BamCache *get_global_ptr();

private:
  PT(BamCacheRecord) find_and_read_record(const Filename &source_pathname,
                                          const Filename &cache_filename) const;
  PT(BamCacheRecord) read_record(const Filename &source_pathname,
                                 const Filename &cache_filename,
                                 int pass) const;

  static string hash_filename(const string &filename);
  static void make_global();

  bool _active;
  Filename _root;
  static BamCache *_global_ptr;
};

#include "bamCache.I"

#endif
