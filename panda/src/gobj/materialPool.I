/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialPool.I
 * @author drose
 * @date 2001-04-30
 */

/**
 * Returns a Material pointer that represents the same material described by
 * temp, except that it is a shared pointer.
 *
 * Each call to get_material() passing an equivalent Material pointer will
 * return the same shared pointer.
 *
 * If you modify the shared pointer, it will automatically disassociate it
 * from the pool.
 *
 * Also, the return value may be a different pointer than that passed in, or
 * it may be the same pointer.  In either case, the passed in pointer has now
 * been sacrificed to the greater good and should not be used again (like any
 * other PointerTo, it will be freed when the last reference count is
 * removed).
 */
INLINE Material *MaterialPool::
get_material(Material *temp) {
  return get_global_ptr()->ns_get_material(temp);
}

/**
 * Removes the indicated material from the pool.
 */
INLINE void MaterialPool::
release_material(Material *material) {
  get_global_ptr()->ns_release_material(material);
}

/**
 * Releases all materials in the pool and restores the pool to the empty
 * state.
 */
INLINE void MaterialPool::
release_all_materials() {
  get_global_ptr()->ns_release_all_materials();
}

/**
 * Releases only those materials in the pool that have a reference count of
 * exactly 1; i.e.  only those materials that are not being used outside of
 * the pool.  Returns the number of materials released.
 */
INLINE int MaterialPool::
garbage_collect() {
  return get_global_ptr()->ns_garbage_collect();
}

/**
 * Lists the contents of the material pool to the indicated output stream.
 */
INLINE void MaterialPool::
list_contents(std::ostream &out) {
  get_global_ptr()->ns_list_contents(out);
}

/**
 * The constructor is not intended to be called directly; there's only
 * supposed to be one MaterialPool in the universe and it constructs itself.
 */
INLINE MaterialPool::
MaterialPool() {
}
