// Filename: clearableRegion.I
// Created by:  drose (11Jul02)
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
//     Function: ClearableRegion::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ClearableRegion::
ClearableRegion() : 
  _flags(0),
  _clear_color(0.0f, 0.0f, 0.0f, 0.0f),
  _clear_depth(1.0f)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ClearableRegion::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ClearableRegion::
ClearableRegion(const ClearableRegion &copy) :
  _flags(copy._flags),
  _clear_color(copy._clear_color),
  _clear_depth(copy._clear_depth)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ClearableRegion::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ClearableRegion::
operator = (const ClearableRegion &copy) {
  _flags = copy._flags;
  _clear_color = copy._clear_color;
  _clear_depth = copy._clear_depth;
}

////////////////////////////////////////////////////////////////////
//     Function: ClearableRegion::copy_clear_settings
//       Access: Public
//  Description: A convenience function that does the same thing as
//               the assignment operator; this is just syntactically a
//               little nicer (and a little clearer) to call from a
//               derived class.
////////////////////////////////////////////////////////////////////
INLINE void ClearableRegion::
copy_clear_settings(const ClearableRegion &copy) {
  operator = (copy);
}

////////////////////////////////////////////////////////////////////
//     Function: ClearableRegion::set_clear_color_active
//       Access: Published
//  Description: Toggles the flag that indicates whether the color
//               buffer should be cleared every frame.  If this is
//               true, the color buffer will be cleared to the color
//               indicated by set_clear_color(); otherwise, it will be
//               left alone.
////////////////////////////////////////////////////////////////////
INLINE void ClearableRegion::
set_clear_color_active(bool clear_color_active) {
  if (clear_color_active) {
    _flags |= F_clear_color_active;
  } else {
    _flags &= ~F_clear_color_active;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClearableRegion::get_clear_color_active
//       Access: Published
//  Description: Returns the current setting of the flag that
//               indicates whether the color buffer should be cleared
//               every frame.  See set_clear_color_active().
////////////////////////////////////////////////////////////////////
INLINE bool ClearableRegion::
get_clear_color_active() const {
  return ((_flags & F_clear_color_active) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ClearableRegion::set_clear_depth_active
//       Access: Published
//  Description: Toggles the flag that indicates whether the depth
//               buffer should be cleared every frame.  If this is
//               true, the depth buffer will be cleared to the depth
//               value indicated by set_clear_depth(); otherwise, it
//               will be left alone.
////////////////////////////////////////////////////////////////////
INLINE void ClearableRegion::
set_clear_depth_active(bool clear_depth_active) {
  if (clear_depth_active) {
    _flags |= F_clear_depth_active;
  } else {
    _flags &= ~F_clear_depth_active;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClearableRegion::get_clear_depth_active
//       Access: Published
//  Description: Returns the current setting of the flag that
//               indicates whether the depth buffer should be cleared
//               every frame.  See set_clear_depth_active().
////////////////////////////////////////////////////////////////////
INLINE bool ClearableRegion::
get_clear_depth_active() const {
  return ((_flags & F_clear_depth_active) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ClearableRegion::set_clear_color
//       Access: Published
//  Description: Sets the clear color to the indicated value.  This is
//               the value that will be used to clear the color buffer
//               every frame, but only if get_clear_color_active()
//               returns true.  If get_clear_color_active() returns
//               false, this is meaningless.
////////////////////////////////////////////////////////////////////
INLINE void ClearableRegion::
set_clear_color(const Colorf &color) {
  _clear_color = color;
}

////////////////////////////////////////////////////////////////////
//     Function: ClearableRegion::get_clear_color
//       Access: Published
//  Description: Returns the current clear color value.  This is
//               the value that will be used to clear the color buffer
//               every frame, but only if get_clear_color_active()
//               returns true.  If get_clear_color_active() returns
//               false, this is meaningless.
////////////////////////////////////////////////////////////////////
INLINE const Colorf &ClearableRegion::
get_clear_color() const {
  return _clear_color;
}

////////////////////////////////////////////////////////////////////
//     Function: ClearableRegion::set_clear_depth
//       Access: Published
//  Description: Sets the clear depth to the indicated value.  This is
//               the value that will be used to clear the depth buffer
//               every frame, but only if get_clear_depth_active()
//               returns true.  If get_clear_depth_active() returns
//               false, this is meaningless.
////////////////////////////////////////////////////////////////////
INLINE void ClearableRegion::
set_clear_depth(float depth) {
  _clear_depth = depth;
}

////////////////////////////////////////////////////////////////////
//     Function: ClearableRegion::get_clear_depth
//       Access: Published
//  Description: Returns the current clear depth value.  This is
//               the value that will be used to clear the depth buffer
//               every frame, but only if get_clear_depth_active()
//               returns true.  If get_clear_depth_active() returns
//               false, this is meaningless.
////////////////////////////////////////////////////////////////////
INLINE float ClearableRegion::
get_clear_depth() const {
  return _clear_depth;
}

////////////////////////////////////////////////////////////////////
//     Function: ClearableRegion::is_any_clear_active
//       Access: Published
//  Description: Returns true if any of the clear types (so far there
//               are just color or depth) have been set active, or
//               false if none of them are active and there is no need
//               to clear.
////////////////////////////////////////////////////////////////////
INLINE bool ClearableRegion::
is_any_clear_active() const {
  return (_flags != 0);
}
