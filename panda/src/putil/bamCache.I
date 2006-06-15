// Filename: bamCache.I
// Created by:  drose (09Jun06)
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
//     Function: BamCache::get_active
//       Access: Published
//  Description: Returns true if the BamCache is currently active,
//               false if it is not.  "active" means that the cache
//               should be consulted automatically on loads, "not
//               active" means that objects should be loaded directly
//               without consulting the cache.
////////////////////////////////////////////////////////////////////
INLINE bool BamCache::
get_active() const {
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::set_active
//       Access: Published
//  Description: Changes the state of the active flag.  "active" means
//               that the cache should be consulted automatically on
//               loads, "not active" means that objects should be
//               loaded directly without consulting the cache.
////////////////////////////////////////////////////////////////////
INLINE void BamCache::
set_active(bool active) {
  _active = active;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::get_root
//       Access: Published
//  Description: Returns the current root pathname of the cache.  See
//               set_root().
////////////////////////////////////////////////////////////////////
INLINE const Filename &BamCache::
get_root() const {
  return _root;
}

////////////////////////////////////////////////////////////////////
//     Function: BamCache::get_global_ptr
//       Access: Published, Static
//  Description: Returns a pointer to the global BamCache object,
//               which is used automatically by the ModelPool and
//               TexturePool.
////////////////////////////////////////////////////////////////////
INLINE BamCache *BamCache::
get_global_ptr() {
  if (_global_ptr == (BamCache *)NULL) {
    make_global();
  }
  return _global_ptr;
}
