/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fontPool.I
 * @author drose
 * @date 2003-01-31
 */

/**
 * Returns true if the font has ever been loaded, false otherwise.
 */
INLINE bool FontPool::
has_font(const std::string &filename) {
  return get_ptr()->ns_has_font(filename);
}

/**
 * Loads the given filename up into a font, if it has not already been loaded,
 * and returns true to indicate success, or false to indicate failure.  If
 * this returns true, it is guaranteed that a subsequent call to load_font()
 * with the same font name will return a valid Font pointer.
 */
INLINE bool FontPool::
verify_font(const std::string &filename) {
  return load_font(filename) != nullptr;
}

/**
 * Loads the given filename up into a font, if it has not already been loaded,
 * and returns the new font.  If a font with the same filename was previously
 * loaded, returns that one instead.  If the font file cannot be found,
 * returns NULL.
 */
INLINE TextFont *FontPool::
load_font(const std::string &filename) {
  return get_ptr()->ns_load_font(filename);
}

/**
 * Adds the indicated already-loaded font to the pool.  The font will always
 * replace any previously-loaded font in the pool that had the same filename.
 */
INLINE void FontPool::
add_font(const std::string &filename, TextFont *font) {
  get_ptr()->ns_add_font(filename, font);
}

/**
 * Removes the indicated font from the pool, indicating it will never be
 * loaded again; the font may then be freed.  If this function is never
 * called, a reference count will be maintained on every font every loaded,
 * and fonts will never be freed.
 */
INLINE void FontPool::
release_font(const std::string &filename) {
  get_ptr()->ns_release_font(filename);
}

/**
 * Releases all fonts in the pool and restores the pool to the empty state.
 */
INLINE void FontPool::
release_all_fonts() {
  get_ptr()->ns_release_all_fonts();
}

/**
 * Releases only those fonts in the pool that have a reference count of
 * exactly 1; i.e.  only those fonts that are not being used outside of the
 * pool.  Returns the number of fonts released.
 */
INLINE int FontPool::
garbage_collect() {
  return get_ptr()->ns_garbage_collect();
}

/**
 * Lists the contents of the font pool to the indicated output stream.
 */
INLINE void FontPool::
list_contents(std::ostream &out) {
  get_ptr()->ns_list_contents(out);
}

/**
 * The constructor is not intended to be called directly; there's only
 * supposed to be one FontPool in the universe and it constructs itself.
 */
INLINE FontPool::
FontPool() {
}
