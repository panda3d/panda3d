// Filename: fadelodNode.I
// Created by:  sshodhan (14Jun04)
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
//     Function: FadeLODNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FadeLODNode::
FadeLODNode(const string &name) :
  LODNode(name) 
{
  _fade_time = 1.0;
  _fade_mode = false;
  _previous_child = 0;
}

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
  _fade_mode = copy._fade_mode;
  _previous_child = copy._previous_child;
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

