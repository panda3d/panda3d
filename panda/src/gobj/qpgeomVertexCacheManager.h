// Filename: qpgeomVertexCacheManager.h
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

#ifndef qpGEOMVERTEXCACHEMANAGER_H
#define qpGEOMVERTEXCACHEMANAGER_H

#include "pandabase.h"
#include "qpgeomMunger.h"
#include "config_gobj.h"
#include "pointerTo.h"
#include "pmutex.h"
#include "indirectLess.h"
#include "plist.h"
#include "pmap.h"

class qpGeomPrimitive;
class qpGeomVertexData;
class qpGeomVertexFormat;

////////////////////////////////////////////////////////////////////
//       Class : qpGeomVertexCacheManager
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
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomVertexCacheManager {
protected:
  qpGeomVertexCacheManager();
  ~qpGeomVertexCacheManager();

PUBLISHED:
  INLINE void set_max_size(int max_size) const;
  INLINE int get_max_size() const;

  INLINE int get_total_size() const;

  static qpGeomVertexCacheManager *get_global_ptr();

private:
  INLINE void record_munger(const qpGeomMunger *munger);
  INLINE void record_decompose(const qpGeomPrimitive *primitive,
                               int result_size);
  INLINE void remove_decompose(const qpGeomPrimitive *primitive);
  INLINE void record_data(const qpGeomVertexData *source,
                          const qpGeomVertexFormat *format,
                          int result_size);
  INLINE void remove_data(const qpGeomVertexData *source,
                          const qpGeomVertexFormat *format);

  class Entry;

  void record_entry(const Entry &entry);
  void remove_entry(const Entry &entry);

private:
  // This mutex protects all operations on this object.
  Mutex _lock;

  int _total_size;

  class Entry {
  public:
    INLINE bool operator < (const Entry &other) const;

    CPT(qpGeomMunger) _munger;
    const qpGeomPrimitive *_primitive;
    const qpGeomVertexData *_source;
    const qpGeomVertexFormat *_format;
    int _result_size;
  };

  // This list keeps the cache entries in least-recently-used order:
  // the items at the head of the list are ready to be flushed.
  typedef plist<Entry> Entries;
  Entries _entries;

  // And this indexes into the above list, for fast lookup.
  typedef pmap<const Entry *, Entries::iterator, IndirectLess<Entry> > EntriesIndex;
  EntriesIndex _entries_index;

  static qpGeomVertexCacheManager *_global_ptr;

  friend class qpGeomMunger;
  friend class qpGeomPrimitive;
  friend class qpGeomVertexData;
};

#include "qpgeomVertexCacheManager.I"

#endif
