// Filename: chatInput.I
// Created by:  drose (04Jul00)
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
//     Function: ChatInput::set_max_chars
//       Access: Public
//  Description: Sets a limit on the number of characters the user is
//               allowed to type.  When this limit is exceeded, no
//               more characters will be accepted, and the event
//               "chat_overflow" is thrown.
////////////////////////////////////////////////////////////////////
INLINE void ChatInput::
set_max_chars(int max_chars) {
  _max_chars = max_chars;
  _flags |= F_max_chars;
}

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::clear_max_chars
//       Access: Public
//  Description: Removes the limit on the maximum number of
//               characters.
////////////////////////////////////////////////////////////////////
INLINE void ChatInput::
clear_max_chars() {
  _flags &= ~F_max_chars;
}

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::has_max_chars
//       Access: Public
//  Description: Returns true if the maximum number of characters has
//               been set by a call to set_max_chars().
////////////////////////////////////////////////////////////////////
INLINE bool ChatInput::
has_max_chars() const {
  return (_flags & F_max_chars) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::get_max_chars
//       Access: Public
//  Description: If has_max_chars() returns true, this will return the
//               maximum number of characters that was set.
////////////////////////////////////////////////////////////////////
INLINE int ChatInput::
get_max_chars() const {
  nassertr(has_max_chars(), 0);
  return _max_chars;
}

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::set_max_lines
//       Access: Public
//  Description: Sets a limit on the number of lines the user is
//               allowed to type.  This makes sense only when wordwrap
//               is enabled on the TextNode; otherwise, it will always
//               be only one line.  When this limit is exceeded, no
//               more characters will be accepted, and the event
//               "chat_overflow" is thrown.
////////////////////////////////////////////////////////////////////
INLINE void ChatInput::
set_max_lines(int max_lines) {
  _max_lines = max_lines;
  _flags |= F_max_lines;
}

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::clear_max_lines
//       Access: Public
//  Description: Removes the limit on the maximum number of
//               characters.
////////////////////////////////////////////////////////////////////
INLINE void ChatInput::
clear_max_lines() {
  _flags &= ~F_max_lines;
}

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::has_max_lines
//       Access: Public
//  Description: Returns true if the maximum number of characters has
//               been set by a call to set_max_lines().
////////////////////////////////////////////////////////////////////
INLINE bool ChatInput::
has_max_lines() const {
  return (_flags & F_max_lines) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::get_max_lines
//       Access: Public
//  Description: If has_max_lines() returns true, this will return the
//               maximum number of characters that was set.
////////////////////////////////////////////////////////////////////
INLINE int ChatInput::
get_max_lines() const {
  nassertr(has_max_lines(), 0);
  return _max_lines;
}

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::set_max_width
//       Access: Public
//  Description: Sets a limit on the total width of the line the user
//               is allowed to type.  When this limit is exceeded, no
//               more characters will be accepted, and the event
//               "chat_overflow" is thrown.
//
//               This is different than set_max_chars(), as some
//               letters use more width than others; capital W, for
//               instance, takes up more space than a lowercase i.  It
//               only makes sense to set this option when wordwrap is
//               *off* for the TextNode.  To limit the text length
//               with wordwrap on, use set_max_lines().
////////////////////////////////////////////////////////////////////
INLINE void ChatInput::
set_max_width(float max_width) {
  _max_width = max_width;
  _flags |= F_max_width;
}

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::clear_max_width
//       Access: Public
//  Description: Removes the limit on the maximum number of
//               characters.
////////////////////////////////////////////////////////////////////
INLINE void ChatInput::
clear_max_width() {
  _flags &= ~F_max_width;
}

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::has_max_width
//       Access: Public
//  Description: Returns true if the maximum number of characters has
//               been set by a call to set_max_width().
////////////////////////////////////////////////////////////////////
INLINE bool ChatInput::
has_max_width() const {
  return (_flags & F_max_width) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::get_max_width
//       Access: Public
//  Description: If has_max_width() returns true, this will return the
//               maximum number of characters that was set.
////////////////////////////////////////////////////////////////////
INLINE float ChatInput::
get_max_width() const {
  nassertr(has_max_width(), 0.0);
  return _max_width;
}

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::get_string
//       Access: Public
//  Description: Returns the current string the user has entered so
//               far.
////////////////////////////////////////////////////////////////////
INLINE const string &ChatInput::
get_string() const {
  return _str;
}
