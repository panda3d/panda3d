// Filename: movingPartBase.I
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
//     Function: MovingPartBase::Copy Constructor
//       Access: Protected
//  Description: Normally, you'd use make_copy() or copy_subgraph() to
//               make a copy of this.
////////////////////////////////////////////////////////////////////
INLINE MovingPartBase::
MovingPartBase(const MovingPartBase &copy) :
  PartGroup(copy),
  _num_effective_channels(0),
  _effective_control(NULL),
  _forced_channel(copy._forced_channel)
{
  // We don't copy the bound channels.  We do copy the forced_channel,
  // though this is just a pointerwise copy.
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartBase::get_max_bound
//       Access: Published
//  Description: Returns the number of channels that might be bound to
//               this PartGroup.  This might not be the actual number
//               of channels, since there might be holes in the list;
//               it is one more than the index number of the highest
//               bound channel.  Thus, it is called get_max_bound()
//               instead of get_num_bound().
////////////////////////////////////////////////////////////////////
INLINE int MovingPartBase::
get_max_bound() const {
  return _channels.size();
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartBase::get_bound
//       Access: Published
//  Description: Returns the nth bound channel on this PartGroup.  n
//               can be determined by iterating from 0 to one less
//               than get_max_bound(); or n might be
//               AnimControl::get_channel_index().
//
//               This will return NULL if there is no channel bound on
//               the indicated index.  It is an error to call this if
//               n is less than zero or greater than or equal to
//               get_max_bound().
////////////////////////////////////////////////////////////////////
INLINE AnimChannelBase *MovingPartBase::
get_bound(int n) const {
  nassertr(n >= 0 && n < (int)_channels.size(), NULL);
  return _channels[n];
}
