// Filename: animChannelBase.I
// Created by:  drose (19Feb99)
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


