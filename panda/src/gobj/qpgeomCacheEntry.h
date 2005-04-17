// Filename: qpgeomCacheEntry.h
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

#ifndef qpGEOMCACHEENTRY_H
#define qpGEOMCACHEENTRY_H

#include "pandabase.h"
#include "qpgeomCacheManager.h"
#include "referenceCount.h"
#include "config_gobj.h"
#include "pointerTo.h"
#include "mutexHolder.h"

class qpGeom;
class qpGeomPrimitive;

////////////////////////////////////////////////////////////////////
//       Class : qpGeomCacheEntry
// Description : This object contains a single cache entry in the
//               GeomCacheManager.  This is actually the base class of
//               any number of individual cache types.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomCacheEntry : public ReferenceCount {
public:
  INLINE qpGeomCacheEntry();
  virtual ~qpGeomCacheEntry();

  PT(qpGeomCacheEntry) record();
  void refresh();
  PT(qpGeomCacheEntry) erase();

  virtual void evict_callback();
  virtual void output(ostream &out) const;

private:
  int _last_frame_used;

  INLINE void remove_from_list();
  INLINE void insert_before(qpGeomCacheEntry *node);

private:  
  qpGeomCacheEntry *_prev, *_next;

  friend class qpGeomCacheManager;
};

INLINE ostream &operator << (ostream &out, const qpGeomCacheEntry &entry);

#include "qpgeomCacheEntry.I"

#endif
