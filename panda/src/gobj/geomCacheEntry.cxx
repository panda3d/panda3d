/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomCacheEntry.cxx
 * @author drose
 * @date 2005-03-21
 */

#include "geomCacheEntry.h"
#include "geomCacheManager.h"
#include "lightMutexHolder.h"
#include "config_gobj.h"
#include "clockObject.h"

TypeHandle GeomCacheEntry::_type_handle;

/**
 *
 */
GeomCacheEntry::
~GeomCacheEntry() {
}

/**
 * Records the entry in the global cache for the first time.
 */
PT(GeomCacheEntry) GeomCacheEntry::
record(Thread *current_thread) {
  nassertr(_next == nullptr && _prev == nullptr, nullptr);
  PT(GeomCacheEntry) keepme = this;

  GeomCacheManager *cache_mgr = GeomCacheManager::get_global_ptr();
  LightMutexHolder holder(cache_mgr->_lock);

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "recording cache entry: " << *this << ", total_size = "
      << cache_mgr->_total_size + 1 << "\n";
  }

  insert_before(cache_mgr->_list);
  ++cache_mgr->_total_size;
  cache_mgr->_geom_cache_size_pcollector.set_level(cache_mgr->_total_size);
  cache_mgr->_geom_cache_record_pcollector.add_level(1);
  _last_frame_used = ClockObject::get_global_clock()->get_frame_count(current_thread);

  if (PStatClient::is_connected()) {
    GeomCacheManager::_geom_cache_active_pcollector.add_level(1);
  }

  // Increment our own reference count while we're in the queue, just so we
  // don't have to play games with it later--this is inner-loop stuff.
  ref();

  // Now remove any old entries if our cache is over the limit.  This may also
  // remove the entry we just added, especially if our cache size is set to 0.
  // This may actually remove this very object.
  cache_mgr->evict_old_entries();

  return this;
}

/**
 * Marks the cache entry recently used, so it will not be evicted for a while.
 */
void GeomCacheEntry::
refresh(Thread *current_thread) {
  GeomCacheManager *cache_mgr = GeomCacheManager::get_global_ptr();
  LightMutexHolder holder(cache_mgr->_lock);
  nassertv(_next != nullptr && _prev != nullptr);

  remove_from_list();
  insert_before(cache_mgr->_list);

  int current_frame = ClockObject::get_global_clock()->get_frame_count(current_thread);
  if (PStatClient::is_connected()) {
    if (_last_frame_used != current_frame) {
      GeomCacheManager::_geom_cache_active_pcollector.add_level(1);
    }
  }

  _last_frame_used = current_frame;
}

/**
 * Removes the entry from the queue, returning a pointer to the entry.  Does
 * not call evict_callback().
 */
PT(GeomCacheEntry) GeomCacheEntry::
erase() {
  nassertr(_next != nullptr && _prev != nullptr, nullptr);

  PT(GeomCacheEntry) keepme;
  keepme.cheat() = this;

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "remove_entry(" << *this << ")\n";
  }

  GeomCacheManager *cache_mgr = GeomCacheManager::get_global_ptr();
  LightMutexHolder holder(cache_mgr->_lock);

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

/**
 * Called when the entry is evicted from the cache, this should clean up the
 * owning object appropriately.
 */
void GeomCacheEntry::
evict_callback() {
}

/**
 *
 */
void GeomCacheEntry::
output(std::ostream &out) const {
  out << "[ unknown ]";
}
