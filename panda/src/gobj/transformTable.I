/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file transformTable.I
 * @author drose
 * @date 2005-03-23
 */

/**
 * Returns true if this table has been registered.  Once it has been
 * registered, the set of transforms in a TransformTable may not be further
 * modified; but it must be registered before it can be assigned to a Geom.
 */
INLINE bool TransformTable::
is_registered() const {
  return _is_registered;
}

/**
 * Registers a TransformTable for use.  This is similar to
 * GeomVertexFormat::register_format().  Once registered, a TransformTable may
 * no longer be modified (although the individual VertexTransform objects may
 * modify their reported transforms).
 *
 * This must be called before a table may be used in a Geom.  After this call,
 * you should discard the original pointer you passed in (which may or may not
 * now be invalid) and let its reference count decrement normally; you should
 * use only the returned value from this point on.
 */
INLINE CPT(TransformTable) TransformTable::
register_table(const TransformTable *table) {
  // We don't actually bother adding the table object to a registry.  This
  // means there may be multiple copies of identical registered
  // TransformTables.  Big deal.  We can always go back and make a registry
  // later if we really need it.
  if (table->is_registered()) {
    return table;
  }

  ((TransformTable *)table)->do_register();
  return table;
}

/**
 * Returns the number of transforms in the table.
 */
INLINE size_t TransformTable::
get_num_transforms() const {
  return _transforms.size();
}

/**
 * Returns the nth transform in the table.
 */
INLINE const VertexTransform *TransformTable::
get_transform(size_t n) const {
  nassertr(n < _transforms.size(), nullptr);
  return _transforms[n];
}

/**
 * Returns a sequence number that's guaranteed to change at least when any
 * VertexTransforms in the table change.  (However, this is only true for a
 * registered table.  An unregistered table may or may not reflect an update
 * here when a VertexTransform changes.)
 */
INLINE UpdateSeq TransformTable::
get_modified(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_modified;
}

/**
 * Called internally whenever a nested VertexTransform reports that it has
 * been modified.
 */
INLINE void TransformTable::
update_modified(UpdateSeq modified, Thread *current_thread) {
  CDWriter cdata(_cycler, true, current_thread);
  cdata->_modified = modified;
}

/**
 *
 */
INLINE TransformTable::CData::
CData() {
}

/**
 *
 */
INLINE TransformTable::CData::
CData(const TransformTable::CData &copy) :
  _modified(copy._modified)
{
}
