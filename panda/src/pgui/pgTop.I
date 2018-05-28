/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgTop.I
 * @author drose
 * @date 2002-03-13
 */

/**
 *
 */
INLINE PGTop::
PGTop(const PGTop &copy) :
  PandaNode(copy),
  _watcher(copy._watcher),
  _start_sort(copy._start_sort)
{
}

/**
 * Returns the MouseWatcher pointer that the PGTop object registers its PG
 * items with, or NULL if the MouseWatcher has not yet been set.
 */
INLINE MouseWatcher *PGTop::
get_mouse_watcher() const {
  return _watcher;
}

/**
 * Returns the MouseWatcherGroup pointer that the PGTop object registers its
 * PG items with, or NULL if the MouseWatcher has not yet been set.
 */
INLINE MouseWatcherGroup *PGTop::
get_group() const {
  return _watcher_group;
}

/**
 * Specifies the sort index that is assigned during the traversal to the first
 * PGItem that is discovered during traversal.  Subsequent PGItems will be
 * assigned consecutively higher sort indexes.
 *
 * This number is used by the MouseWatcher system to rank the clickable mouse
 * regions in the same order in which the items are rendered, so that items on
 * top will receive mouse priority.
 *
 * Normally, it makes the most sense to leave this initial value at its
 * default value of 0, unless you need the PGItems to have a particular sort
 * value with respect to some other objects in the scene (particularly with a
 * second PGTop node).
 */
INLINE void PGTop::
set_start_sort(int start_sort) {
  _start_sort = start_sort;
}

/**
 * Returns the sort index that is assigned during the traversal to the first
 * PGItem that is discovered during traversal.  See set_start_sort().
 */
INLINE int PGTop::
get_start_sort() const {
  return _start_sort;
}


/**
 * Adds the indicated region to the set of regions in the group.
 */
INLINE void PGTop::
add_region(MouseWatcherRegion *region) {
  nassertv(_watcher_group != nullptr);
  _watcher_group->add_region(region);
}
