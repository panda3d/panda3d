// Filename: geomCacheManager.I
// Created by:  drose (11Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: GeomCacheManager::set_max_size
//       Access: Published
//  Description: Specifies the maximum number of entries in the cache
//               for storing pre-processed data for rendering
//               vertices.  This limit is flexible, and may be
//               temporarily exceeded if many different Geoms are
//               pre-processed during the space of a single frame.
//
//               This is not a limit on the actual vertex data, which
//               is what it is; it is also not a limit on the amount
//               of memory used by the video driver or the system
//               graphics interface, which Panda has no control over.
////////////////////////////////////////////////////////////////////
INLINE void GeomCacheManager::
set_max_size(int max_size) const {
  // We directly change the config variable.
  geom_cache_size = max_size;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomCacheManager::get_max_size
//       Access: Published
//  Description: Returns the maximum number of entries in the cache
//               for storing pre-processed data for rendering
//               vertices.  See set_max_size().
////////////////////////////////////////////////////////////////////
INLINE int GeomCacheManager::
get_max_size() const {
  return geom_cache_size;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomCacheManager::get_total_size
//       Access: Published
//  Description: Returns the number of entries currently in the cache.
////////////////////////////////////////////////////////////////////
INLINE int GeomCacheManager::
get_total_size() const {
  return _total_size;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomCacheManager::evict_old_entries
//       Access: Public
//  Description: Trims the cache size down to get_max_size() by
//               evicting old cache entries as needed.  It is assumed
//               that you already hold the lock before calling this
//               method.
////////////////////////////////////////////////////////////////////
INLINE void GeomCacheManager::
evict_old_entries() {
  evict_old_entries(get_max_size(), true);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomCacheManager::flush_level
//       Access: Public, Static
//  Description: Flushes the PStatCollectors used during traversal.
////////////////////////////////////////////////////////////////////
INLINE void GeomCacheManager::
flush_level() {
  _geom_cache_size_pcollector.flush_level();
  _geom_cache_active_pcollector.flush_level();
  _geom_cache_record_pcollector.flush_level();
  _geom_cache_erase_pcollector.flush_level();
  _geom_cache_evict_pcollector.flush_level();
}
