// Filename: geomCacheManager.h
// Created by:  drose (11Mar05)
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

#ifndef GEOMCACHEMANAGER_H
#define GEOMCACHEMANAGER_H

#include "pandabase.h"
#include "config_gobj.h"
#include "lightMutex.h"
#include "pStatCollector.h"

class GeomCacheEntry;

////////////////////////////////////////////////////////////////////
//       Class : GeomCacheManager
// Description : This is used to keep track of, and limit the size of,
//               the cache of munged vertices, which would otherwise
//               be distributed through all of the GeomVertexData
//               objects in the system.
//
//               The actual data in the cache is not stored here, but
//               rather it is distributed among the various
//               GeomVertexData source objects.  This allows the cache
//               data to propagate through the multiprocess pipeline.
//
//               This structure actually caches any of a number of
//               different types of pointers, and mixes them all up in
//               the same LRU cache list.  Some of them (such as
//               GeomMunger) are reference-counted here in the cache;
//               most are not.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GOBJ GeomCacheManager {
protected:
  GeomCacheManager();
  ~GeomCacheManager();

PUBLISHED:
  INLINE void set_max_size(int max_size) const;
  INLINE int get_max_size() const;

  INLINE int get_total_size() const;

  void flush();

  static GeomCacheManager *get_global_ptr();

public:
  INLINE void evict_old_entries();
  void evict_old_entries(int max_size, bool keep_current);
  INLINE static void flush_level();

private:
  // This mutex protects all operations on this object, especially the
  // linked-list operations.
  LightMutex _lock;

  int _total_size;

  // We maintain a doubly-linked list to keep the cache entries in
  // least-recently-used order: the items at the head of the list are
  // ready to be flushed.  We use our own doubly-linked list instead
  // of an STL list, just so we can avoid a tiny bit of overhead,
  // especially in keeping the pointer directly into the list from the
  // calling objects.

  // The tail and the head of the list are both kept by the _prev and
  // _next pointers, respectively, within the following object, which
  // always exists solely to keep a handle to the list.  Keeping a
  // token of the list this way avoids special cases for an empty
  // list.
  GeomCacheEntry *_list;

  static GeomCacheManager *_global_ptr;

public:
  static PStatCollector _geom_cache_size_pcollector;
  static PStatCollector _geom_cache_active_pcollector;
  static PStatCollector _geom_cache_record_pcollector;
  static PStatCollector _geom_cache_erase_pcollector;
  static PStatCollector _geom_cache_evict_pcollector;

  friend class GeomCacheEntry;
};

#include "geomCacheManager.I"

#endif
