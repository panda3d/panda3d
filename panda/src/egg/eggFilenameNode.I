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
}

////////////////////////////////////////////////////////////////////
//     Function: EggFilenameNode::update_filename
//       Access: Public
//  Description: Returns a modifiable reference to the filename.
////////////////////////////////////////////////////////////////////
INLINE Filename &EggFilenameNode::
update_filename() {
  return _filename;
}
