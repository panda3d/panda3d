// Filename: configFlags.I
// Created by:  drose (21Oct04)
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
//     Function: ConfigFlags::is_cache_valid
//       Access: Protected, Static
//  Description: Returns true if the local object's cache is still
//               valid (based on a comparison of the supplied
//               local_modified value with the global_modified value).
////////////////////////////////////////////////////////////////////
INLINE bool ConfigFlags::
is_cache_valid(AtomicAdjust::Integer local_modified) {
  return local_modified == _global_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigFlags::mark_cache_valid
//       Access: Protected, Static
//  Description: Updates the indicated local_modified value so that
//               the cache will appear to be valid, until someone next
//               calls invalidate_cache().
////////////////////////////////////////////////////////////////////
INLINE void ConfigFlags::
mark_cache_valid(AtomicAdjust::Integer &local_modified) {
  local_modified = _global_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigFlags::initial_invalid_cache
//       Access: Protected, Static
//  Description: Returns a value that will be appropriate for
//               initializing a local_modified value.  This value will
//               indicate an invalid cache in the next call to
//               is_cache_valid().
////////////////////////////////////////////////////////////////////
INLINE AtomicAdjust::Integer ConfigFlags::
initial_invalid_cache() {
  return _global_modified - 1;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigFlags::invalidate_cache
//       Access: Protected, Static
//  Description: Invalidates all of the global ConfigVariable caches
//               in the world at once, by incrementing the
//               global_modified counter.
////////////////////////////////////////////////////////////////////
INLINE void ConfigFlags::
invalidate_cache() {
  AtomicAdjust::inc(_global_modified);
}
