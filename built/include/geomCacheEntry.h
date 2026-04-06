/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomCacheEntry.h
 * @author drose
 * @date 2005-03-21
 */

#ifndef GEOMCACHEENTRY_H
#define GEOMCACHEENTRY_H

#include "pandabase.h"
#include "geomCacheManager.h"
#include "referenceCount.h"
#include "config_gobj.h"
#include "pointerTo.h"
#include "mutexHolder.h"

class Geom;
class GeomPrimitive;

/**
 * This object contains a single cache entry in the GeomCacheManager.  This is
 * actually the base class of any number of individual cache types.
 */
class EXPCL_PANDA_GOBJ GeomCacheEntry : public ReferenceCount {
public:
  INLINE GeomCacheEntry();
  virtual ~GeomCacheEntry();

  PT(GeomCacheEntry) record(Thread *current_thread);
  void refresh(Thread *current_thread);
  PT(GeomCacheEntry) erase();

  virtual void evict_callback();
  virtual void output(std::ostream &out) const;

private:
  int _last_frame_used;

  INLINE void remove_from_list();
  INLINE void insert_before(GeomCacheEntry *node);

private:
  GeomCacheEntry *_prev, *_next;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    register_type(_type_handle, "GeomCacheEntry",
                  ReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class GeomCacheManager;
};

INLINE std::ostream &operator << (std::ostream &out, const GeomCacheEntry &entry);

#include "geomCacheEntry.I"

#endif
