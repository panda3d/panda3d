// Filename: partGroup.I
// Created by:  drose (22Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
  Namable(name)
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
  Namable(copy)
{
  // We don't copy children in the copy constructor.  However,
  // copy_subgraph() will do this.
}
