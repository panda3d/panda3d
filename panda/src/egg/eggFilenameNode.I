/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggFilenameNode.I
 * @author drose
 * @date 1999-02-11
 */

/**
 *
 */
INLINE EggFilenameNode::
EggFilenameNode() {
}

/**
 *
 */
INLINE EggFilenameNode::
EggFilenameNode(const std::string &node_name, const Filename &filename) :
  EggNode(node_name),
  _filename(filename),
  _fullpath(filename)
{
}

/**
 *
 */
INLINE EggFilenameNode::
EggFilenameNode(const EggFilenameNode &copy) :
  EggNode(copy),
  _filename(copy._filename),
  _fullpath(copy._fullpath)
{
}

/**
 *
 */
INLINE EggFilenameNode &EggFilenameNode::
operator = (const EggFilenameNode &copy) {
  EggNode::operator = (copy);
  _filename = copy._filename;
  _fullpath = copy._fullpath;
  return *this;
}

/**
 * Returns a nonmodifiable reference to the filename.
 */
INLINE const Filename &EggFilenameNode::
get_filename() const {
  return _filename;
}

/**
 *
 */
INLINE void EggFilenameNode::
set_filename(const Filename &filename) {
  _filename = filename;
  _fullpath = filename;
}

/**
 * Returns the full pathname to the file, if it is known; otherwise, returns
 * the same thing as get_filename().
 *
 * This function simply returns whatever was set by the last call to
 * set_fullpath().  This string is not written to the egg file; its main
 * purpose is to record the full path to a filename (for instance, a texture
 * filename) if it is known, for egg structures that are generated in-memory
 * and then immediately converted to a scene graph.
 */
INLINE const Filename &EggFilenameNode::
get_fullpath() const {
  return _fullpath;
}

/**
 * Records the full pathname to the file, for the benefit of get_fullpath().
 */
INLINE void EggFilenameNode::
set_fullpath(const Filename &fullpath) {
  _fullpath = fullpath;
}
