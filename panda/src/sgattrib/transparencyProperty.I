// Filename: transparencyProperty.I
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: TransparencyProperty::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TransparencyProperty::
TransparencyProperty(TransparencyProperty::Mode mode) :
  _mode(mode)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyProperty::Constructor
//       Access: Public
//  Description: This should only be called in the Protected 
//               constructor of TransparencyTransition.  It is made 
//               public, only for that to access, and is necessary
//               for creating an object in make_TransparencyTransition
//               before any information has been read
////////////////////////////////////////////////////////////////////
INLINE TransparencyProperty::
TransparencyProperty(void)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyProperty::set_mode
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TransparencyProperty::
set_mode(TransparencyProperty::Mode mode) {
  _mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyProperty::get_mode
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TransparencyProperty::Mode TransparencyProperty::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: TransparencyProperty::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int TransparencyProperty::
compare_to(const TransparencyProperty &other) const {
  return (int)_mode - (int)other._mode;
}
