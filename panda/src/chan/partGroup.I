// Filename: partGroup.I
// Created by:  drose (22Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: PartGroup::Default Constructor
//       Access: Protected
//  Description: This constructor is only intended for interal use and
//               for derived classes.  You should normally use the
//               non-default constructor, below.
////////////////////////////////////////////////////////////////////
INLINE PartGroup::
PartGroup(const string &name) :
  Namable(name),
  _children(get_class_type())
{
}

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::Copy Constructor
//       Access: Protected
//  Description: This constructor is only intended for interal use and
//               for derived classes.  You should normally use the
//               make_copy() interface to make copies..
////////////////////////////////////////////////////////////////////
INLINE PartGroup::
PartGroup(const PartGroup &copy) :
  Namable(copy),
  _children(get_class_type())
{
  // We don't copy children in the copy constructor.  However,
  // copy_subgraph() will do this.
}
