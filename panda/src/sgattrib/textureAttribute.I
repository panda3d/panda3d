// Filename: textureAttribute.I
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: TextureAttribute::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TextureAttribute::
TextureAttribute() {
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttribute::set_on
//       Access: Public
//  Description: Changes the TextureAttribute to render the particular
//               texture.
////////////////////////////////////////////////////////////////////
INLINE void TextureAttribute::
set_on(Texture *texture) {
  nassertv(texture != (Texture *)NULL);
  _value = texture;
  OnOffAttribute::set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttribute::get_texture
//       Access: Public
//  Description: Returns the texture that the TextureAttribute
//               represents.  It is only valid to call this if is_on()
//               has returned true.
////////////////////////////////////////////////////////////////////
INLINE PT(Texture) TextureAttribute::
get_texture() const {
  nassertr(is_on(), NULL);
  return _value;
}
