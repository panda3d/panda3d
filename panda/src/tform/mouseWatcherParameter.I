// Filename: mouseWatcherParameter.I
// Created by:  drose (06Jul01)
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
//     Function: MouseWatcherParameter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MouseWatcherParameter::
MouseWatcherParameter() {
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MouseWatcherParameter::
MouseWatcherParameter(const MouseWatcherParameter &copy) :
  _button(copy._button),
  _mods(copy._mods),
  _mouse(copy._mouse),
  _flags(copy._flags)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherParameter::
operator = (const MouseWatcherParameter &copy) {
  _button = copy._button;
  _mods = copy._mods;
  _mouse = copy._mouse;
  _flags = copy._flags;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MouseWatcherParameter::
~MouseWatcherParameter() {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::set_button
//       Access: Public
//  Description: Sets the mouse or keyboard button that generated this
//               event, if any.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherParameter::
set_button(const ButtonHandle &button) {
  _button = button;
  _flags |= F_has_button;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::set_modifier_buttons
//       Access: Public
//  Description: Sets the modifier buttons that were being held while
//               this event was generated.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherParameter::
set_modifier_buttons(const ModifierButtons &mods) {
  _mods = mods;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::set_mouse
//       Access: Public
//  Description: Sets the mouse position that was current at the time
//               the event was generated.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherParameter::
set_mouse(const LPoint2f &mouse) {
  _mouse = mouse;
  _flags |= F_has_mouse;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::set_outside
//       Access: Public
//  Description: Sets the state of the "outside" flag.  This is true
//               if the mouse was outside the region at the time the
//               event was generated, false otherwise.  This only has
//               meaning for "release" events.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherParameter::
set_outside(bool flag) {
  if (flag) {
    _flags |= F_is_outside;
  } else {
    _flags &= ~F_is_outside;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::has_button
//       Access: Published
//  Description: Returns true if this parameter has an associated
//               mouse or keyboard button, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcherParameter::
has_button() const {
  return (_flags & F_has_button) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::get_button
//       Access: Published
//  Description: Returns the mouse or keyboard button associated with
//               this event.  It is valid to call this only if
//               has_button(), above, returned true.
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle MouseWatcherParameter::
get_button() const {
  nassertr(has_button(), ButtonHandle::none());
  return _button;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::get_modifier_buttons
//       Access: Published
//  Description: Returns the set of modifier buttons that were being
//               held down while the event was generated.
////////////////////////////////////////////////////////////////////
INLINE const ModifierButtons &MouseWatcherParameter::
get_modifier_buttons() const {
  return _mods;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::has_mouse
//       Access: Published
//  Description: Returns true if this parameter has an associated
//               mouse position, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcherParameter::
has_mouse() const {
  return (_flags & F_has_mouse) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::get_mouse
//       Access: Published
//  Description: Returns the mouse position at the time the event was
//               generated, in the normalized range (-1 .. 1).  It is
//               valid to call this only if has_mouse() returned true.
////////////////////////////////////////////////////////////////////
INLINE const LPoint2f &MouseWatcherParameter::
get_mouse() const {
  nassertr(has_mouse(), _mouse);
  return _mouse;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::is_outside
//       Access: Published
//  Description: Returns true if the mouse was outside the region at
//               the time the event was generated, false otherwise.
//               This is only valid for "release" type events.
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcherParameter::
is_outside() const {
  return (_flags & F_is_outside) != 0;
}

INLINE ostream &
operator << (ostream &out, const MouseWatcherParameter &parm) {
  parm.output(out);
  return out;
}
