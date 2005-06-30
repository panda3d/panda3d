// Filename: geomCacheEntry.cxx
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

#include "geomCacheEntry.h"
#include "geomCacheManager.h"
#include "mutexHolder.h"
#include "config_gobj.h"
#include "clockObject.h"

////////////////////////////////////////////////////////////////////
//     Function: GeomCacheEntry::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GeomCacheEntry::
~GeomCacheEntry() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomCacheEntry::record
//       Access: Public
//  Description: Records the entry in the global cache for the first
//               time.
////////////////////////////////////////////////////////////////////
PT(GeomCacheEntry) GeomCacheEntry::
record() {
  nassertr(_next == (GeomCacheEntry *)NULL && _prev == (GeomCacheEntry *)NULL, NULL);
  PT(GeomCacheEntry) keepme = this;

  GeomCacheManager *cache_mgr = GeomCacheManager::get_global_ptr();
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

  if (PStatClient::is_connected()) {
    GeomCacheManager::_geom_cache_active_pcollector.add_level(1);
  }

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
//     Function: GeomCacheEntry::refresh
//       Access: Public
//  Description: Marks the cache entry recently used, so it will not
//               be evicted for a while.
////////////////////////////////////////////////////////////////////
void GeomCacheEntry::
refresh() {
  nassertv(_next != (GeomCacheEntry *)NULL && _prev != (GeomCacheEntry *)NULL);

  GeomCacheManager *cache_mgr = GeomCacheManager::get_global_ptr();
  MutexHolder holder(cache_mgr->_lock);

  remove_from_list();
  insert_before(cache_mgr->_list);

  int current_frame = ClockObject::get_global_clock()->get_frame_count();
  if (PStatClient::is_connected()) {
    if (_last_frame_used != current_frame) {
      GeomCacheManager::_geom_cache_active_pcollector.add_level(1);
    }
  }

  _last_frame_used = current_frame;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomCacheEntry::erase
//       Access: Public
//  Description: Removes the entry from the queue, returning a pointer
//               to the entry.  Does not call evict_callback().
////////////////////////////////////////////////////////////////////
PT(GeomCacheEntry) GeomCacheEntry::
erase() {
  nassertr(_next != (GeomCacheEntry *)NULL && _prev != (GeomCacheEntry *)NULL, NULL);

  PT(GeomCacheEntry) keepme = this;
  unref();

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "remove_entry(" << *this << ")\n";
  }

  GeomCacheManager *cache_mgr = GeomCacheManager::get_global_ptr();
  MutexHolder holder(cache_mgr->_lock);

  remove_from_list();
  --cache_mgr->_total_size;
  cache_mgr->_geom_cache_size_pcollector.set_level(cache_mgr->_total_size);
  cache_mgr->_geom_cache_erase_pcollector.add_level(1);

  if (PStatClient::is_connected()) {
    int current_frame = ClockObject::get_global_clock()->get_frame_count();
    if (_last_frame_used == current_frame) {
      GeomCacheManager::_geom_cache_active_pcollector.sub_level(1);
    }
  }

  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomCacheEntry::evict_callback
//       Access: Public, Virtual
//  Description: Called when the entry is evicted from the cache, this
//               should clean up the owning object appropriately.
////////////////////////////////////////////////////////////////////
void GeomCacheEntry::
evict_callback() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomCacheEntry::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomCacheEntry::
output(ostream &out) const {
  out << "[ unknown ]";
}
