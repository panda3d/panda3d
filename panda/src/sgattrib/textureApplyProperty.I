// Filename: textureApplyProperty.I
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyProperty::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TextureApplyProperty::
TextureApplyProperty(TextureApplyProperty::Mode mode) :
  _mode(mode)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyProperty::Constructor
//       Access: Public
//  Description: This should only be called in the Protected 
//               constructor of TextureApplyTransition.  It is made 
//               public, only for that to access, and is necessary
//               for creating an object in make_TextureApplyTransition
//               before any information has been read
////////////////////////////////////////////////////////////////////
INLINE TextureApplyProperty::
TextureApplyProperty(void)
{
}


////////////////////////////////////////////////////////////////////
//     Function: TextureApplyProperty::set_mode
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TextureApplyProperty::
set_mode(TextureApplyProperty::Mode mode) {
  _mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyProperty::get_mode
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TextureApplyProperty::Mode TextureApplyProperty::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyProperty::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int TextureApplyProperty::
compare_to(const TextureApplyProperty &other) const {
  return (int)_mode - (int)other._mode;
}
