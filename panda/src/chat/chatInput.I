// Filename: chatInput.I
// Created by:  drose (04Jul00)
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
  _has_max_chars = true;
}

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::clear_max_chars
//       Access: Public
//  Description: Removes the limit on the maximum number of
//               characters.
////////////////////////////////////////////////////////////////////
INLINE void ChatInput::
clear_max_chars() {
  _has_max_chars = false;
}

////////////////////////////////////////////////////////////////////
//     Function: ChatInput::has_max_chars
//       Access: Public
//  Description: Returns true if the maximum number of characters has
//               been set by a call to set_max_chars().
////////////////////////////////////////////////////////////////////
INLINE bool ChatInput::
has_max_chars() const {
  return _has_max_chars;
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
//     Function: ChatInput::get_string
//       Access: Public
//  Description: Returns the current string the user has entered so
//               far.
////////////////////////////////////////////////////////////////////
INLINE const string &ChatInput::
get_string() const { 
  return _str; 
}
