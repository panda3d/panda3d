// Filename: materialPool.I
// Created by:  drose (30Apr01)
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
//     Function: MaterialPool::get_material
//       Access: Public, Static
//  Description: Returns a const Material pointer that represents the
//               same material described by temp, except that it is a
//               shared pointer.
//
//               Each call to get_material() passing an equivalent
//               Material pointer will return the same shared pointer.
//
//               This shared pointer must not be further modified.
//               Doing so will (a) cause global changes to other
//               objects that may share this material, and (b) damage
//               the sorted container that stores these things, so
//               that future pointers may not be properly shared.
//
//               Also, the return value may be a different pointer
//               than that passed in, or it may be the same pointer.
//               In either case, the passed in pointer has now been
//               sacrificed to the greater good and should not be used
//               again (like any other PointerTo, it will be freed
//               when the last reference count is removed).
////////////////////////////////////////////////////////////////////
INLINE const Material *MaterialPool::
get_material(const CPT(Material) &temp) {
  return get_ptr()->ns_get_material(temp);
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialPool::garbage_collect
//       Access: Public, Static
//  Description: Releases only those materials in the pool that have a
//               reference count of exactly 1; i.e. only those
//               materials that are not being used outside of the pool.
//               Returns the number of materials released.
////////////////////////////////////////////////////////////////////
INLINE int MaterialPool::
garbage_collect() {
  return get_ptr()->ns_garbage_collect();
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialPool::list_contents
//       Access: Public, Static
//  Description: Lists the contents of the material pool to the
//               indicated output stream.
////////////////////////////////////////////////////////////////////
INLINE void MaterialPool::
list_contents(ostream &out) {
  get_ptr()->ns_list_contents(out);
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialPool::Constructor
//       Access: Private
//  Description: The constructor is not intended to be called
//               directly; there's only supposed to be one MaterialPool
//               in the universe and it constructs itself.
////////////////////////////////////////////////////////////////////
INLINE MaterialPool::
MaterialPool() {
}
