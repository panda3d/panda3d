// Filename: dataRelation.I
// Created by:  drose (08Jun00)
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
//     Function: DataRelation::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DataRelation::
DataRelation(Node *from, Node *to, int sort) :
  NodeRelation(from, to, sort, get_class_type())
{
}

////////////////////////////////////////////////////////////////////
//     Function: DataRelation::Constructor
//       Access: Protected
//  Description: Creates a new, unattached arc.  This constructor is
//               only intended for passing through the factory to
//               create an arc based on a particular type using
//               create_typed_arc(), below.  You shouldn't, in
//               general, attempt to create an unattached arc.
////////////////////////////////////////////////////////////////////
INLINE DataRelation::
DataRelation() :
  NodeRelation(get_class_type())
{
}

////////////////////////////////////////////////////////////////////
//     Function: DataRelation::register_with_factory
//       Access: Public, Static
//  Description: Registers the type DataRelation with the factory.
//               This is only intended to be called at initialization
//               time; don't call it directly.
////////////////////////////////////////////////////////////////////
INLINE void DataRelation::
register_with_factory() {
  get_factory().register_factory(get_class_type(), make_arc);
}
