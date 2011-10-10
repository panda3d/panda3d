// Filename: mouseWatcherRegion.I
// Created by:  drose (13Jul00)
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
//     Function: MouseWatcherRegion::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MouseWatcherRegion::
MouseWatcherRegion(const string &name, PN_stdfloat left, PN_stdfloat right,
                   PN_stdfloat bottom, PN_stdfloat top) :
  Namable(name),
  _frame(left, right, bottom, top)
{
  _sort = 0;
  _flags = F_active;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MouseWatcherRegion::
MouseWatcherRegion(const string &name, const LVecBase4 &frame) :
  Namable(name),
  _frame(frame)
{
  _sort = 0;
  _flags = F_active;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::set_frame
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherRegion::
set_frame(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  set_frame(LVecBase4(left, right, bottom, top));
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::set_frame
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherRegion::
set_frame(const LVecBase4 &frame) {
  _frame = frame;
  _area = (_frame[1] - _frame[0]) * (_frame[3] - _frame[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::get_frame
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4 &MouseWatcherRegion::
get_frame() const {
  return _frame;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::get_area
//       Access: Published
//  Description: Returns the area of the rectangular region.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat MouseWatcherRegion::
get_area() const {
  return _area;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::set_sort
//       Access: Published
//  Description: Changes the sorting order of this particular region.
//               The sorting order is used to resolve conflicts in the
//               case of overlapping region; the region with the
//               highest sort value will be preferred, and between
//               regions of the same sort value, the smallest region
//               will be preferred.  The default sorting order, if
//               none is explicitly specified, is 0.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherRegion::
set_sort(int sort) {
  _sort = sort;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::get_sort
//       Access: Published
//  Description: Returns the current sorting order of this region.
//               See set_sort().
////////////////////////////////////////////////////////////////////
INLINE int MouseWatcherRegion::
get_sort() const {
  return _sort;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::set_active
//       Access: Published
//  Description: Sets whether the region is active or not.  If it is
//               not active, the MouseWatcher will never consider the
//               mouse to be over the region.  The region might still
//               receive keypress events if its set_keyboard() flag is
//               true.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherRegion::
set_active(bool active) {
  if (active) {
    _flags |= F_active;
  } else {
    _flags &= ~F_active;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::get_active
//       Access: Published
//  Description: Returns whether the region is active or not.  See
//               set_active().
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcherRegion::
get_active() const {
  return ((_flags & F_active) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::set_keyboard
//       Access: Published
//  Description: Sets whether the region is interested in global
//               keyboard events.  If this is true, then any keyboard
//               button events will be passed to press() and release()
//               regardless of the position of the mouse onscreen;
//               otherwise, these events will only be passed if the
//               mouse is over the region.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherRegion::
set_keyboard(bool keyboard) {
  if (keyboard) {
    _flags |= F_keyboard;
  } else {
    _flags &= ~F_keyboard;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::get_keyboard
//       Access: Published
//  Description: Returns whether the region is interested in global
//               keyboard events; see set_keyboard().
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcherRegion::
get_keyboard() const {
  return ((_flags & F_keyboard) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::set_suppress_flags
//       Access: Published
//  Description: Sets which events are suppressed when the mouse is
//               over the region.  This is the union of zero or more
//               various SF_* values.  Normally, this is 0, indicating
//               that no events are suppressed.
//
//               If you set this to a non-zero value, for instance
//               SF_mouse_position, then the mouse position will not
//               be sent along the data graph when the mouse is over
//               this particular region.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherRegion::
set_suppress_flags(int suppress_flags) {
  _flags = ((_flags & ~F_suppress_flags) | (suppress_flags & F_suppress_flags));
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::get_suppress_flags
//       Access: Published
//  Description: Returns the current suppress_flags.  See
//               set_suppress_flags().
////////////////////////////////////////////////////////////////////
INLINE int MouseWatcherRegion::
get_suppress_flags() const {
  return (_flags & F_suppress_flags);
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::Ordering Operator
//       Access: Public
//  Description: Returns true if this region should be preferred over
//               the other region when they overlap, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcherRegion::
operator < (const MouseWatcherRegion &other) const {
  if (_sort != other._sort) {
    return _sort > other._sort;
  }
  return _area < other._area;
}
