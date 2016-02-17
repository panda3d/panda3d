/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcFile.I
 * @author drose
 * @date 2006-01-10
 */

/**
 * Returns true if all of the classes read from the DC file were defined and
 * valid, or false if any of them were undefined ("bogus classes").  If this
 * is true, we might have read a partial file.
 */
INLINE bool DCFile::
all_objects_valid() const {
  return _all_objects_valid;
}

/**
 * Rebuilds all of the inherited fields tables, if necessary.
 */
INLINE void DCFile::
check_inherited_fields() {
  if (_inherited_fields_stale) {
    rebuild_inherited_fields();
  }
}

/**
 * Indicates that something has changed in one or more of the inheritance
 * chains or the set of fields; the next time check_inherited_fields() is
 * called, the inherited fields tables of all classes will be rebuilt.
 */
INLINE void DCFile::
mark_inherited_fields_stale() {
  _inherited_fields_stale = true;
}
