// Filename: animChannelBase.I
// Created by:  drose (19Feb99)
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
//     Function: AnimChannelBase::Protected constructor
//       Access: Protected
//  Description: Don't use this constructor.  It exists only so that
//               AnimChannelFixed may define itself outside of the
//               hierarchy.  Normally, an AnimChannel must be created
//               as part of a hierarchy.
////////////////////////////////////////////////////////////////////
INLINE AnimChannelBase::
AnimChannelBase(const string &name)
  : AnimGroup(name)
{
  _last_frame = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelBase::Copy Constructor
//       Access: Protected
//  Description: Creates a new AnimChannelBase, just like this one,
//               without copying any children.  The new copy is added
//               to the indicated parent.  Intended to be called by
//               make_copy() only.
////////////////////////////////////////////////////////////////////
INLINE AnimChannelBase::
AnimChannelBase(AnimGroup *parent, const AnimChannelBase &copy) : 
  AnimGroup(parent, copy),
  _last_frame(copy._last_frame)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelBase::Constructor
//       Access: Public
//  Description: This is the normal constructor, which automatically
//               places the AnimChannel in the previously-created
//               hierarchy.
////////////////////////////////////////////////////////////////////
INLINE AnimChannelBase::
AnimChannelBase(AnimGroup *parent, const string &name)
  : AnimGroup(parent, name)
{
  _last_frame = -1;
}


