// Filename: movingPartBase.I
// Created by:  drose (22Feb99)
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
  PartGroup(copy)
{
  // We don't copy the bound channels.
}
