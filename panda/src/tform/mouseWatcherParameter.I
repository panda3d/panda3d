// Filename: mouseWatcherParameter.I
// Created by:  drose (06Jul01)
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
//     Function: MouseWatcherParameter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MouseWatcherParameter::
MouseWatcherParameter() {
  _keycode = 0;
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
  _keycode(copy._keycode),
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
  _keycode = copy._keycode;
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
//     Function: MouseWatcherParameter::set_keyrepeat
//       Access: Public
//  Description: Sets the state of the "keyrepeat" flag.  This is true
//               if a button-press event was generated due to
//               keyrepeat, or false if it is an original button
//               press.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherParameter::
set_keyrepeat(bool flag) {
  if (flag) {
    _flags |= F_is_keyrepeat;
  } else {
    _flags &= ~F_is_keyrepeat;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::set_keycode
//       Access: Public
//  Description: Sets the keycode associated with this event, if any.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherParameter::
set_keycode(int keycode) {
  _keycode = keycode;
  _flags |= F_has_keycode;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::set_candidate
//       Access: Public
//  Description: Sets the candidate string associated with this event,
//               if any.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherParameter::
set_candidate(const wstring &candidate_string,
              size_t highlight_start, size_t highlight_end,
              size_t cursor_pos) {
  _candidate_string = candidate_string;
  _highlight_start = highlight_start;
  _highlight_end = highlight_end;
  _cursor_pos = cursor_pos;
  _flags |= F_has_candidate;
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
set_mouse(const LPoint2 &mouse) {
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
//               this event.  If has_button(), above, returns false,
//               this returns ButtonHandle::none().
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle MouseWatcherParameter::
get_button() const {
  return _button;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::is_keyrepeat
//       Access: Published
//  Description: Returns true if the button-down even was generated
//               due to keyrepeat, or false if it was an original
//               button down.
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcherParameter::
is_keyrepeat() const {
  return (_flags & F_is_keyrepeat) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::has_keycode
//       Access: Published
//  Description: Returns true if this parameter has an associated
//               keycode, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcherParameter::
has_keycode() const {
  return (_flags & F_has_keycode) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::get_keycode
//       Access: Published
//  Description: Returns the keycode associated with this event.  If
//               has_keycode(), above, returns false, this returns 0.
////////////////////////////////////////////////////////////////////
INLINE int MouseWatcherParameter::
get_keycode() const {
  return _keycode;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::has_candidate
//       Access: Published
//  Description: Returns true if this parameter has an associated
//               candidate string, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcherParameter::
has_candidate() const {
  return (_flags & F_has_candidate) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::get_candidate_string
//       Access: Published
//  Description: Returns the candidate string associated with this
//               event.  If has_candidate(), above, returns false,
//               this returns the empty string.
////////////////////////////////////////////////////////////////////
INLINE const wstring &MouseWatcherParameter::
get_candidate_string() const {
  return _candidate_string;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::get_candidate_string_encoded
//       Access: Published
//  Description: Returns the candidate string associated with this
//               event.  If has_candidate(), above, returns false,
//               this returns the empty string.
////////////////////////////////////////////////////////////////////
INLINE string MouseWatcherParameter::
get_candidate_string_encoded() const {
  return get_candidate_string_encoded(TextEncoder::get_default_encoding());
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::get_candidate_string_encoded
//       Access: Published
//  Description: Returns the candidate string associated with this
//               event.  If has_candidate(), above, returns false,
//               this returns the empty string.
////////////////////////////////////////////////////////////////////
INLINE string MouseWatcherParameter::
get_candidate_string_encoded(TextEncoder::Encoding encoding) const {
  return TextEncoder::encode_wtext(_candidate_string, encoding);
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::get_highlight_start
//       Access: Published
//  Description: Returns the first highlighted character in the
//               candidate string.
////////////////////////////////////////////////////////////////////
INLINE size_t MouseWatcherParameter::
get_highlight_start() const {
  return _highlight_start;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::get_highlight_end
//       Access: Published
//  Description: Returns one more than the last highlighted character
//               in the candidate string.
////////////////////////////////////////////////////////////////////
INLINE size_t MouseWatcherParameter::
get_highlight_end() const {
  return _highlight_end;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::get_cursor_pos
//       Access: Published
//  Description: Returns the position of the user's edit cursor within
//               the candidate string.
////////////////////////////////////////////////////////////////////
INLINE size_t MouseWatcherParameter::
get_cursor_pos() const {
  return _cursor_pos;
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
INLINE const LPoint2 &MouseWatcherParameter::
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
