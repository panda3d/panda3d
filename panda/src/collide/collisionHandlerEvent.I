// Filename: collisionHandlerEvent.I
// Created by:  drose (16Mar02)
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
//     Function: CollisionHandlerEvent::SortEntries::operator ()
//       Access: Public
//  Description: Orders the CollisionEntries in the set so that there
//               is one entry for each node/node intersection
//               detected.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionHandlerEvent::SortEntries::
operator () (const PT(CollisionEntry) &a,
             const PT(CollisionEntry) &b) const {
  int compare_from = a->get_from_node_path().compare_to(b->get_from_node_path());
  if (compare_from != 0) {
    return compare_from < 0;
  }
  return a->get_into_node_path() < b->get_into_node_path();
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
//     Function: CollisionHandlerEvent::clear_in_patterns
//       Access: Public
//  Description: Removes all of the previously-added in patterns.  See
//               add_in_pattern.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerEvent::
clear_in_patterns() {
  _in_patterns.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::add_in_pattern
//       Access: Public
//  Description: Adds a pattern string to the list of events that will
//               be generated in response to a collision.  The pattern
//               string describes how the event name will be composed.
//               It is a string that may contain any of the following:
//
//                  %fn  - the name of the "from" object's node
//                  %in  - the name of the "into" object's node
//                  %fs  - 't' if "from" is tangible, 'i' if intangible
//                  %is  - 't' if "into" is tangible, 'i' if intangible
//                  %ig  - 'c' if the collision is into a
//                          CollisionNode, 'g' if it is a geom.
//
//                  %(tag)fh - generate event only if "from" node has
//                          the indicated net tag.
//                  %(tag)fx - generate event only if "from" node does 
//                          not have the indicated net tag.
//                  %(tag)ih - generate event only if "into" node has
//                          the indicated net tag.
//                  %(tag)ix - generate event only if "into" node does 
//                          not have the indicated net tag.
//                  %(tag)ft - the indicated net tag value of the "from" node.
//                  %(tag)it - the indicated net tag value of the "into" node.
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
add_in_pattern(const string &in_pattern) {
  _in_patterns.push_back(in_pattern);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::set_in_pattern
//       Access: Public
//  Description: This method is deprecated; it completely replaces all
//               the in patterns that have previously been set with
//               the indicated pattern.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerEvent::
set_in_pattern(const string &in_pattern) {
  clear_in_patterns();
  add_in_pattern(in_pattern);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::get_num_in_patterns
//       Access: Public
//  Description: Returns the number of in pattern strings that have
//               been added.
////////////////////////////////////////////////////////////////////
INLINE int CollisionHandlerEvent::
get_num_in_patterns() const {
  return _in_patterns.size();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::get_in_pattern
//       Access: Public
//  Description: Returns the nth pattern string that indicates how the
//               event names are generated for each collision
//               detected.  See add_in_pattern().
////////////////////////////////////////////////////////////////////
INLINE string CollisionHandlerEvent::
get_in_pattern(int n) const {
  nassertr(n >= 0 && n < (int)_in_patterns.size(), string());
  return _in_patterns[n];
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::clear_again_patterns
//       Access: Public
//  Description: Removes all of the previously-added in patterns.  See
//               add_again_pattern.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerEvent::
clear_again_patterns() {
  _again_patterns.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::add_again_pattern
//       Access: Public
//  Description: Adds the pattern string that indicates how the event
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
add_again_pattern(const string &again_pattern) {
  _again_patterns.push_back(again_pattern);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::set_again_pattern
//       Access: Public
//  Description: This method is deprecated; it completely replaces all
//               the in patterns that have previously been set with
//               the indicated pattern.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerEvent::
set_again_pattern(const string &again_pattern) {
  clear_again_patterns();
  add_again_pattern(again_pattern);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::get_num_again_patterns
//       Access: Public
//  Description: Returns the number of in pattern strings that have
//               been added.
////////////////////////////////////////////////////////////////////
INLINE int CollisionHandlerEvent::
get_num_again_patterns() const {
  return _again_patterns.size();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::get_again_pattern
//       Access: Public
//  Description: Returns the nth pattern string that indicates how the
//               event names are generated for each collision
//               detected.  See add_again_pattern().
////////////////////////////////////////////////////////////////////
INLINE string CollisionHandlerEvent::
get_again_pattern(int n) const {
  nassertr(n >= 0 && n < (int)_again_patterns.size(), string());
  return _again_patterns[n];
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::clear_out_patterns
//       Access: Public
//  Description: Removes all of the previously-added in patterns.  See
//               add_out_pattern.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerEvent::
clear_out_patterns() {
  _out_patterns.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::add_out_pattern
//       Access: Public
//  Description: Adds the pattern string that indicates how the event
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
add_out_pattern(const string &out_pattern) {
  _out_patterns.push_back(out_pattern);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::set_out_pattern
//       Access: Public
//  Description: This method is deprecated; it completely replaces all
//               the in patterns that have previously been set with
//               the indicated pattern.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerEvent::
set_out_pattern(const string &out_pattern) {
  clear_out_patterns();
  add_out_pattern(out_pattern);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::get_num_out_patterns
//       Access: Public
//  Description: Returns the number of in pattern strings that have
//               been added.
////////////////////////////////////////////////////////////////////
INLINE int CollisionHandlerEvent::
get_num_out_patterns() const {
  return _out_patterns.size();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerEvent::get_out_pattern
//       Access: Public
//  Description: Returns the nth pattern string that indicates how the
//               event names are generated for each collision
//               detected.  See add_out_pattern().
////////////////////////////////////////////////////////////////////
INLINE string CollisionHandlerEvent::
get_out_pattern(int n) const {
  nassertr(n >= 0 && n < (int)_out_patterns.size(), string());
  return _out_patterns[n];
}
