// Filename: collisionHandlerEvent.I
// Created by:  drose (16Mar02)
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
//     Function: CollisionHandlerEvent::SortEntries::operator ()
//       Access: Public
//  Description: Orders the CollisionEntries in the set so that there
//               is one entry for each node/node intersection
//               detected.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionHandlerEvent::SortEntries::
operator () (const PT(CollisionEntry) &a,
             const PT(CollisionEntry) &b) const {
  if (a->get_from_node() != b->get_from_node()) {
    return a->get_from_node() < b->get_from_node();
  }
  if (a->get_into_node() != b->get_into_node()) {
    return a->get_into_node() < b->get_into_node();
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::SortEntries::operator =
//       Access: Public
//  Description: The assignment operator does absolutely nothing,
//               since this is just a function object class that
//               stores no data.  We define it just to quiet up g++ in
//               -Wall mode.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerEvent::SortEntries::
operator = (const CollisionHandlerEvent::SortEntries &) {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::set_in_pattern
//       Access: Public
//  Description: Sets the pattern string that indicates how the event
//               names are generated for each collision detected.
//               This is a string that may contain any of the
//               following:
//
//                  %fn  - the name of the "from" object's node
//                  %in  - the name of the "into" object's node
//                  %fs  - 't' if "from" is tangible, 'i' if intangible
//                  %is  - 't' if "into" is tangible, 'i' if intangible
//                  %ig  - 'c' if the collision is into a
//                          CollisionNode, 'g' if it is a geom.
//
//                  %(tag)fh - '1' if "from" has the indicated tag,
//                          '0' if it does not.
//                  %(tag)ih - '1' if "into" has the indicated tag,
//                          '0' if it does not.
//                  %(tag)iH - '1' if "into" has the indicated net tag,
//                          '0' if it does not.
//                  %(tag)ft - the indicated tag value of the "from" node.
//                  %(tag)it - the indicated tag value of the "to" node.
//                  %(tag)iT - the indicated net tag value of the "to" node.
//
//               Parentheses in the above are literal and should be
//               included in the actual pattern.
//
//               The event name will be based on the in_pattern
//               string specified here, with all occurrences of the
//               above strings replaced with the corresponding values.
//
//               In general, the in_pattern event is thrown on the
//               first detection of a collision between two particular
//               nodes.  In subsequent passes, as long as a collision
//               between those two nodes continues to be detected each
//               frame, the again_pattern is thrown.  The first frame
//               in which the collision is no longer detected, the
//               out_pattern event is thrown.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerEvent::
set_in_pattern(const string &in_pattern) {
  _in_pattern = in_pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::get_in_pattern
//       Access: Public
//  Description: Returns the pattern string that indicates how the
//               event names are generated for each collision
//               detected.  See set_in_pattern().
////////////////////////////////////////////////////////////////////
INLINE string CollisionHandlerEvent::
get_in_pattern() const {
  return _in_pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::set_again_pattern
//       Access: Public
//  Description: Sets the pattern string that indicates how the event
//               names are generated when a collision between two
//               particular nodes is *still* detected.  This event is
//               thrown each consecutive time a collision between two
//               particular nodes is detected, starting with the
//               second time.
//
//               In general, the in_pattern event is thrown on the
//               first detection of a collision between two particular
//               nodes.  In subsequent passes, as long as a collision
//               between those two nodes continues to be detected each
//               frame, the again_pattern is thrown.  The first frame
//               in which the collision is no longer detected, the
//               out_pattern event is thrown.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerEvent::
set_again_pattern(const string &again_pattern) {
  _again_pattern = again_pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::get_again_pattern
//       Access: Public
//  Description: Returns the pattern string that indicates how the
//               event names are generated when a collision between
//               two particular nodes is *still* detected.  See
//               set_again_pattern() and set_in_pattern().
////////////////////////////////////////////////////////////////////
INLINE string CollisionHandlerEvent::
get_again_pattern() const {
  return _again_pattern;
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::set_out_pattern
//       Access: Public
//  Description: Sets the pattern string that indicates how the event
//               names are generated when a collision between two
//               particular nodes is *no longer* detected.
//
//               In general, the in_pattern event is thrown on the
//               first detection of a collision between two particular
//               nodes.  In subsequent passes, as long as a collision
//               between those two nodes continues to be detected each
//               frame, the again_pattern is thrown.  The first frame
//               in which the collision is no longer detected, the
//               out_pattern event is thrown.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerEvent::
set_out_pattern(const string &out_pattern) {
  _out_pattern = out_pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::get_out_pattern
//       Access: Public
//  Description: Returns the pattern string that indicates how the
//               event names are generated when a collision between
//               two particular nodes is *no longer* detected.  See
//               set_out_pattern() and set_in_pattern().
////////////////////////////////////////////////////////////////////
INLINE string CollisionHandlerEvent::
get_out_pattern() const {
  return _out_pattern;
}

