// Filename: buttonHandle.I
// Created by:  drose (01Mar00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::Constructor
//       Access: Public
//  Description: The default constructor must do nothing, because we
//               can't guarantee ordering of static initializers.  If
//               the constructor tried to initialize its value, it
//               might happen after the value had already been set
//               previously by another static initializer!
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle::
ButtonHandle() { 
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle::
ButtonHandle(const ButtonHandle &copy) : _index(copy._index) { 
}
 
////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::Equality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ButtonHandle::
operator == (const ButtonHandle &other) const {
  return (_index == other._index);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ButtonHandle::
operator != (const ButtonHandle &other) const {
  return (_index != other._index);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::Ordering Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ButtonHandle::
operator < (const ButtonHandle &other) const {
  return (_index < other._index);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::has_ascii_equivalent
//       Access: Public
//  Description: Returns true if the button was created with an ASCII
//               equivalent code (e.g. for a standard keyboard
//               button).
////////////////////////////////////////////////////////////////////
INLINE bool ButtonHandle::
has_ascii_equivalent() const {
  return (_index > 0 && _index < 128);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::get_ascii_equivalent
//       Access: Public
//  Description: Returns the character code associated with the
//               button, or '\0' if no ASCII code was associated.
////////////////////////////////////////////////////////////////////
INLINE char ButtonHandle::
get_ascii_equivalent() const {
  return has_ascii_equivalent() ? (char)_index : '\0';
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::none
//       Access: Public, Static
//  Description: Returns a special zero-valued ButtonHandle that is
//               used to indicate no button.
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle ButtonHandle::
none() {
  return _none;
}
