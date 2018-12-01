/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomCacheManager.cxx
 * @author drose
 * @date 2005-03-11
 */

#include "geomCacheManager.h"
#include "geomCacheEntry.h"
#include "geomMunger.h"
#include "lightMutexHolder.h"
#include "lightReMutexHolder.h"
#include "clockObject.h"

GeomCacheManager *GeomCacheManager::_global_ptr = nullptr;

PStatCollector GeomCacheManager::_geom_cache_size_pcollector("Geom cache size");
PStatCollector GeomCacheManager::_geom_cache_active_pcollector("Geom cache size:Active");
PStatCollector GeomCacheManager::_geom_cache_record_pcollector("Geom cache operations:record");
PStatCollector GeomCacheManager::_geom_cache_erase_pcollector("Geom cache operations:erase");
PStatCollector GeomCacheManager::_geom_cache_evict_pcollector("Geom cache operations:evict");

/**
 *
 */
GeomCacheManager::
GeomCacheManager() :
  _lock("GeomCacheManager"),
  _total_size(0)
{
  // We deliberately hang on to this pointer forever.
  _list = new GeomCacheEntry;
  _list->ref();
  _list->_next = _list;
  _list->_prev = _list;
}

/**
 *
 */
GeomCacheManager::
~GeomCacheManager() {
  // Shouldn't be deleting this global object.
  nassert_raise("attempt to delete GeomCacheManager");
}

/**
 * Immediately empties all elements in the cache.
 */
void GeomCacheManager::
flush() {
  // Prevent deadlock
  LightReMutexHolder registry_holder(GeomMunger::get_registry()->_registry_lock);

  LightMutexHolder holder(_lock);
  evict_old_entries(0, false);
}

/**
 * Returns the global cache manager pointer.
 */
GeomCacheManager *GeomCacheManager::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new GeomCacheManager;
  }
  return _global_ptr;
}

/**
 * Trims the cache size down to the specified size by evicting old cache
 * entries as needed.  It is assumed that you already hold the lock before
 * calling this method.
 */
void GeomCacheManager::
evict_old_entries(int max_size, bool keep_current) {
  int current_frame = ClockObject::get_global_clock()->get_frame_count();
  int min_frames = geom_cache_min_frames;

  while (_total_size > max_size) {
    PT(GeomCacheEntry) entry = _list->_next;
    nassertv(entry != _list);

    if (keep_current && current_frame - entry->_last_frame_used < min_frames) {
      // Never mind, this one is too new.
      if (gobj_cat.is_debug()) {
        gobj_cat.debug()
          << "Oldest element in cache is "
          << current_frame - entry->_last_frame_used
          << " frames; keeping cache at " << _total_size << " entries.\n";
      }
      break;
    }

    entry->unref();

    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "cache total_size = " << _total_size << " entries, max_size = "
        << max_size << ", removing " << *entry << "\n";
    }

    entry->evict_callback();

    if (PStatClient::is_connected()) {
      if (entry->_last_frame_used == current_frame) {
        GeomCacheManager::_geom_cache_active_pcollector.sub_level(1);
      }
    }

    --_total_size;
    entry->remove_from_list();
    _geom_cache_evict_pcollector.add_level(1);
  }
  _geom_cache_size_pcollector.set_level(_total_size);
}
