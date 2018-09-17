/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file recorderTable.I
 * @author drose
 * @date 2004-01-27
 */

/**
 *
 */
INLINE RecorderTable::
RecorderTable() {
  _error = false;
}

/**
 *
 */
INLINE RecorderTable::
RecorderTable(const RecorderTable &copy) {
  *this = copy;
}

/**
 *
 */
INLINE void RecorderTable::
operator = (const RecorderTable &copy) {
  _recorders = copy._recorders;
  _error = copy._error;

  Recorders::iterator ri;
  for (ri = _recorders.begin(); ri != _recorders.end(); ++ri) {
    ri->second->ref();
  }
}

/**
 * Adds the named recorder to the set of recorders.
 */
INLINE void RecorderTable::
add_recorder(const std::string &name, RecorderBase *recorder) {
  nassertv(recorder != nullptr);
  recorder->ref();

  std::pair<Recorders::iterator, bool> result =
    _recorders.insert(Recorders::value_type(name, recorder));

  if (!result.second) {
    // Take out the previous one first.
    unref_delete(result.first->second);
    result.first->second = recorder;
  }
}

/**
 * Returns the recorder with the indicated name, or NULL if there is no such
 * recorder.
 */
INLINE RecorderBase *RecorderTable::
get_recorder(const std::string &name) const {
  Recorders::const_iterator ri = _recorders.find(name);
  if (ri != _recorders.end()) {
    return (*ri).second;
  }
  return nullptr;
}

/**
 * Removes the named recorder from the table.  Returns true if successful,
 * false if there was no such recorder.
 */
INLINE bool RecorderTable::
remove_recorder(const std::string &name) {
  Recorders::iterator ri = _recorders.find(name);
  if (ri != _recorders.end()) {
    unref_delete(ri->second);
    _recorders.erase(ri);
    return true;
  }
  return false;
}
