// Filename: cullFaceProperty.I
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: CullFaceProperty::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullFaceProperty::
CullFaceProperty(Mode mode) :
  _mode(mode)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceProperty::Constructor
//       Access: Public
//  Description: This should only be called in the Protected 
//               constructor of CullFaceTransition.  It is made 
//               public, only for that to access, and is necessary
//               for creating an object in make_CullFaceTransition
//               before any information has been read
////////////////////////////////////////////////////////////////////
INLINE CullFaceProperty::
CullFaceProperty(void)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceProperty::set_mode
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CullFaceProperty::
set_mode(CullFaceProperty::Mode mode) {
  _mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceProperty::get_mode
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullFaceProperty::Mode CullFaceProperty::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceProperty::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int CullFaceProperty::
compare_to(const CullFaceProperty &other) const {
  return (int)_mode - (int)other._mode;
}

