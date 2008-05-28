// Filename: buttonEvent.I
// Created by:  drose (01Mar00)
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
//     Function: ButtonEvent::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonEvent::
ButtonEvent() :
  _button(ButtonHandle::none()),
  _keycode(0),
  _type(T_down),
  _time(0.0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonEvent::
ButtonEvent(ButtonHandle button, ButtonEvent::Type type, double time) :
  _button(button),
  _keycode(0),
  _highlight_start(0),
  _highlight_end(0),
  _type(type),
  _time(time)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonEvent::
ButtonEvent(short keycode, double time) :
  _button(ButtonHandle::none()),
  _keycode(keycode),
  _highlight_start(0),
  _highlight_end(0),
  _type(T_keystroke),
  _time(time)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonEvent::
ButtonEvent(const wstring &candidate_string, size_t highlight_start, 
            size_t highlight_end, size_t cursor_pos) :
  _button(ButtonHandle::none()),
  _keycode(0),
  _candidate_string(candidate_string),
  _highlight_start(highlight_start),
  _highlight_end(highlight_end),
  _cursor_pos(cursor_pos),
  _type(T_candidate),
  _time(ClockObject::get_global_clock()->get_frame_time())
{
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonEvent::
ButtonEvent(const ButtonEvent &copy) :
  _button(copy._button),
  _keycode(copy._keycode),
  _candidate_string(copy._candidate_string),
  _highlight_start(copy._highlight_start),
  _highlight_end(copy._highlight_end),
  _cursor_pos(copy._cursor_pos),
  _type(copy._type),
  _time(copy._time)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ButtonEvent::
operator = (const ButtonEvent &copy) {
  _button = copy._button;
  _keycode = copy._keycode;
  _candidate_string = copy._candidate_string;
  _highlight_start = copy._highlight_start;
  _highlight_end = copy._highlight_end;
  _cursor_pos = copy._cursor_pos;
  _type = copy._type;
  _time = copy._time;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Equality Operator
//       Access: Public
//  Description: The equality operator does not consider time
//               significant.
////////////////////////////////////////////////////////////////////
INLINE bool ButtonEvent::
operator == (const ButtonEvent &other) const {
  return (_button == other._button &&
          _keycode == other._keycode &&
          _type == other._type);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Inequality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ButtonEvent::
operator != (const ButtonEvent &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ButtonEvent::
operator < (const ButtonEvent &other) const {
  if (_button != other._button) {
    return _button < other._button;
  }
  if (_keycode != other._keycode) {
    return _keycode < other._keycode;
  }

  return _type < other._type;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::update_mods
//       Access: Published
//  Description: Calls button_down() or button_up(), as appropriate,
//               according to the ButtonEvent.
////////////////////////////////////////////////////////////////////
INLINE bool ButtonEvent::
update_mods(ModifierButtons &mods) const {
  switch (_type) {
  case T_down:
    return mods.button_down(_button);

  case T_up:
    return mods.button_up(_button);

  default:
    return false;
  }
}
