// Filename: eggFilenameNode.I
// Created by:  drose (11Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: EggFilenameNode::Default constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggFilenameNode::
EggFilenameNode() {
}

////////////////////////////////////////////////////////////////////
//     Function: EggFilenameNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggFilenameNode::
EggFilenameNode(const string &node_name, const Filename &filename) :
  EggNode(node_name),
  _filename(filename)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggFilenameNode::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggFilenameNode::
EggFilenameNode(const EggFilenameNode &copy) :
  EggNode(copy),
  _filename(copy._filename)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggFilenameNode::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggFilenameNode &EggFilenameNode::
operator = (const EggFilenameNode &copy) {
  EggNode::operator = (copy);
  _filename = copy._filename;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggFilenameNode::get_filename
//       Access: Public
//  Description: Returns a nonmodifiable reference to the filename.
////////////////////////////////////////////////////////////////////
INLINE const Filename &EggFilenameNode::
get_filename() const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: EggFilenameNode::set_filename
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggFilenameNode::
set_filename(const Filename &filename) {
  _filename = filename;
  _fullpath = filename;
}

////////////////////////////////////////////////////////////////////
//     Function: EggFilenameNode::get_fullpath
//       Access: Public
//  Description: Returns the full pathname to the file, if it is
//               known; otherwise, returns the same thing as
//               get_filename().
//
//               This function simply returns whatever was set by the
//               last call to set_fullpath().  This string is not
//               written to the egg file; its main purpose is to
//               record the full path to a filename (for instance, a
//               texture filename) if it is known, for egg structures
//               that are generated in-memory and then immediately
//               converted to a scene graph.
////////////////////////////////////////////////////////////////////
INLINE const Filename &EggFilenameNode::
get_fullpath() const {
  return _fullpath;
}

////////////////////////////////////////////////////////////////////
//     Function: EggFilenameNode::set_fullpath
//       Access: Public
//  Description: Records the full pathname to the file, for the
//               benefit of get_fullpath().
////////////////////////////////////////////////////////////////////
INLINE void EggFilenameNode::
set_fullpath(const Filename &fullpath) {
  _fullpath = fullpath;
}
