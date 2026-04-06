/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileList.I
 * @author drose
 * @date 2002-08-03
 */

/**
 *
 */
INLINE VirtualFileList::
VirtualFileList() {
}

/**
 *
 */
INLINE VirtualFileList::
~VirtualFileList() {
}

/**
 * Adds a new file to the list.
 */
INLINE void VirtualFileList::
add_file(VirtualFile *file) {
  _files.push_back(file);
}

/**
 * Returns the number of files in the list.
 */
INLINE size_t VirtualFileList::
get_num_files() const {
  return _files.size();
}

/**
 * Returns the nth file in the list.
 */
INLINE VirtualFile *VirtualFileList::
get_file(size_t n) const {
  nassertr(n < _files.size(), nullptr);
  return _files[n];
}

/**
 * Returns the nth file in the list.
 */
INLINE VirtualFile *VirtualFileList::
operator [](size_t n) const {
  nassertr(n < _files.size(), nullptr);
  return _files[n];
}

/**
 * Returns the number of files in the list.
 */
INLINE size_t VirtualFileList::
size() const {
  return _files.size();
}

/**
 * Appends the other list onto the end of this one.
 */
INLINE void VirtualFileList::
operator += (const VirtualFileList &other) {
  _files.insert(_files.end(), other._files.begin(), other._files.end());
}

/**
 * Returns a VirtualFileList representing the concatenation of the two lists.
 */
INLINE VirtualFileList VirtualFileList::
operator + (const VirtualFileList &other) const {
  VirtualFileList a(*this);
  a += other;
  return a;
}
