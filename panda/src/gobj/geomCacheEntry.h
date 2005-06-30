// Filename: geomCacheEntry.h
// Created by:  drose (21Mar05)
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

////////////////////////////////////////////////////////////////////
//       Class : GeomCacheEntry
// Description : This object contains a single cache entry in the
//               GeomCacheManager.  This is actually the base class of
//               any number of individual cache types.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomCacheEntry : public ReferenceCount {
public:
  INLINE GeomCacheEntry();
  virtual ~GeomCacheEntry();

  PT(GeomCacheEntry) record();
  void refresh();
  PT(GeomCacheEntry) erase();

  virtual void evict_callback();
  virtual void output(ostream &out) const;

private:
  int _last_frame_used;

  INLINE void remove_from_list();
  INLINE void insert_before(GeomCacheEntry *node);

private:  
  GeomCacheEntry *_prev, *_next;

  friend class GeomCacheManager;
};

INLINE ostream &operator << (ostream &out, const GeomCacheEntry &entry);

#include "geomCacheEntry.I"

#endif
