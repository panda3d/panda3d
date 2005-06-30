// Filename: geomCacheManager.I
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
