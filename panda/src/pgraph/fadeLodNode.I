// Filename: fadelodNode.I
// Created by:  sshodhan (14Jun04)
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
//     Function: FadeLODNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FadeLODNode::
FadeLODNode(const FadeLODNode &copy) :
  LODNode(copy)
{
  _fade_time = copy._fade_time;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::set_fade_time
//       Access: Published
//  Description: set the time taken to complete an LOD switch
////////////////////////////////////////////////////////////////////
INLINE void FadeLODNode::
set_fade_time(float t) {
  _fade_time = t;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::get_fade_time
//       Access: Published
//  Description: get the time taken to complete an LOD switch
////////////////////////////////////////////////////////////////////
INLINE float FadeLODNode::
get_fade_time() const {
  return _fade_time;
}

