// Filename: qpgeomCacheEntry.cxx
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

#include "qpgeomCacheEntry.h"
#include "qpgeomCacheManager.h"
#include "mutexHolder.h"
#include "config_gobj.h"
#include "clockObject.h"

////////////////////////////////////////////////////////////////////
//     Function: qpGeomCacheEntry::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomCacheEntry::
~qpGeomCacheEntry() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomCacheEntry::record
//       Access: Public
//  Description: Records the entry in the global cache for the first
//               time.
////////////////////////////////////////////////////////////////////
PT(qpGeomCacheEntry) qpGeomCacheEntry::
record() {
  nassertr(_next == (qpGeomCacheEntry *)NULL && _prev == (qpGeomCacheEntry *)NULL, NULL);
  PT(qpGeomCacheEntry) keepme = this;

  qpGeomCacheManager *cache_mgr = qpGeomCacheManager::get_global_ptr();
  MutexHolder holder(cache_mgr->_lock);

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "recording cache entry: " << *this << ", total_size = "
      << cache_mgr->_total_size + 1 << "\n";
  }

  insert_before(cache_mgr->_list);
  ++cache_mgr->_total_size;
  cache_mgr->_geom_cache_size_pcollector.set_level(cache_mgr->_total_size);
  cache_mgr->_geom_cache_record_pcollector.add_level(1);
  _last_frame_used = ClockObject::get_global_clock()->get_frame_count();

  // Increment our own reference count while we're in the queue, just
  // so we don't have to play games with it later--this is inner-loop
  // stuff.
  ref();

  // Now remove any old entries if our cache is over the limit.  This may
  // also remove the entry we just added, especially if our cache size
  // is set to 0.  This may actually remove this very object.
  cache_mgr->evict_old_entries();

  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomCacheEntry::refresh
//       Access: Public
//  Description: Marks the cache entry recently used, so it will not
//               be evicted for a while.
////////////////////////////////////////////////////////////////////
void qpGeomCacheEntry::
refresh() {
  nassertv(_next != (qpGeomCacheEntry *)NULL && _prev != (qpGeomCacheEntry *)NULL);

  qpGeomCacheManager *cache_mgr = qpGeomCacheManager::get_global_ptr();
  MutexHolder holder(cache_mgr->_lock);

  remove_from_list();
  insert_before(cache_mgr->_list);

  _last_frame_used = ClockObject::get_global_clock()->get_frame_count();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomCacheEntry::erase
//       Access: Public
//  Description: Removes the entry from the queue, returning a pointer
//               to the entry.  Does not call evict_callback().
////////////////////////////////////////////////////////////////////
PT(qpGeomCacheEntry) qpGeomCacheEntry::
erase() {
  nassertr(_next != (qpGeomCacheEntry *)NULL && _prev != (qpGeomCacheEntry *)NULL, NULL);

  PT(qpGeomCacheEntry) keepme = this;
  unref();

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "remove_entry(" << *this << ")\n";
  }

  qpGeomCacheManager *cache_mgr = qpGeomCacheManager::get_global_ptr();
  MutexHolder holder(cache_mgr->_lock);

  remove_from_list();
  --cache_mgr->_total_size;
  cache_mgr->_geom_cache_size_pcollector.set_level(cache_mgr->_total_size);
  cache_mgr->_geom_cache_erase_pcollector.add_level(1);

  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomCacheEntry::evict_callback
//       Access: Public, Virtual
//  Description: Called when the entry is evicted from the cache, this
//               should clean up the owning object appropriately.
////////////////////////////////////////////////////////////////////
void qpGeomCacheEntry::
evict_callback() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomCacheEntry::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomCacheEntry::
output(ostream &out) const {
  out << "[ unknown ]";
}
