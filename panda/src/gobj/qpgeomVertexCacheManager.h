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

class qpGeom;
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
//               This structure actually caches any of a number of
//               different types of pointers, and mixes them all up in
//               the same LRU cache list.  Some of them (such as
//               GeomMunger) are reference-counted here in the cache;
//               most are not.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomVertexCacheManager {
protected:
  qpGeomVertexCacheManager();
  ~qpGeomVertexCacheManager();

public:
  class Entry;

PUBLISHED:
  INLINE void set_max_size(int max_size) const;
  INLINE int get_max_size() const;

  INLINE int get_total_size() const;

  static qpGeomVertexCacheManager *get_global_ptr();

private:
  INLINE void record_munger(const qpGeomMunger *munger);
  INLINE void record_primitive(const qpGeomPrimitive *primitive,
                               int result_size);
  INLINE void remove_primitive(const qpGeomPrimitive *primitive);
  INLINE void record_data(const qpGeomVertexData *source,
                          const qpGeomVertexFormat *modifier,
                          int result_size);
  INLINE void remove_data(const qpGeomVertexData *source,
                          const qpGeomVertexFormat *modifier);
  INLINE void record_geom(const qpGeom *source,
                          const qpGeomMunger *modifier,
                          int result_size);
  INLINE void remove_geom(const qpGeom *geom, const qpGeomMunger *modifier);

  void record_entry(const Entry &entry);
  void remove_entry(const Entry &entry);

private:
  // This mutex protects all operations on this object.
  Mutex _lock;

  int _total_size;

  enum CacheType {
    CT_munger,
    CT_primitive,
    CT_data,
    CT_geom,
  };

public:
  // This class is public only so we can declare the global ostream
  // output operator.  It doesn't need to be visible outside this
  // class.  It contains a single cache entry, which might actually be
  // any of a handful of different pointer types.  The enumerated type
  // declared above, and the union declared below, serve to implement
  // this C-style polymorphism.
  class Entry {
  public:
    INLINE Entry(const qpGeomMunger *munger, int result_size);
    INLINE Entry(const qpGeomPrimitive *primitive, int result_size);
    INLINE Entry(const qpGeomVertexData *source,
                 const qpGeomVertexFormat *modifier, int result_size);
    INLINE Entry(const qpGeom *source, const qpGeomMunger *modifier, 
                 int result_size);
    INLINE Entry(const Entry &copy);
    INLINE void operator = (const Entry &copy);
    INLINE ~Entry();
    INLINE bool operator < (const Entry &other) const;

    void output(ostream &out) const;

    CacheType _cache_type;
    int _result_size;
    union {
      const qpGeomMunger *_munger;
      const qpGeomPrimitive *_primitive;
      struct {
        const qpGeomVertexData *_source;
        const qpGeomVertexFormat *_modifier;
      } _data;
      struct {
        const qpGeom *_source;
        const qpGeomMunger *_modifier;
      } _geom;
    } _u;
  };

private:
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
  friend class qpGeom;
};

INLINE ostream &operator << (ostream &out, const qpGeomVertexCacheManager::Entry &entry);

#include "qpgeomVertexCacheManager.I"

#endif
