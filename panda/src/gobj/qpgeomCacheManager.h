// Filename: qpgeomCacheManager.h
// Created by:  drose (11Mar05)
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

#ifndef qpGEOMCACHEMANAGER_H
#define qpGEOMCACHEMANAGER_H

#include "pandabase.h"
#include "config_gobj.h"
#include "pmutex.h"

class qpGeomCacheEntry;

////////////////////////////////////////////////////////////////////
//       Class : qpGeomCacheManager
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
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomCacheManager {
protected:
  qpGeomCacheManager();
  ~qpGeomCacheManager();

PUBLISHED:
  INLINE void set_max_size(int max_size) const;
  INLINE int get_max_size() const;

  INLINE int get_total_size() const;

  static qpGeomCacheManager *get_global_ptr();

public:
  void evict_old_entries();

private:
  // This mutex protects all operations on this object, especially the
  // linked-list operations.
  Mutex _lock;

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
  qpGeomCacheEntry *_list;

  static qpGeomCacheManager *_global_ptr;
  friend class qpGeomCacheEntry;
};

#include "qpgeomCacheManager.I"

#endif
