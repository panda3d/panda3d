// Filename: textureTransition.I
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TextureTransition::
TextureTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TextureTransition::
TextureTransition(Texture *texture) : 
  OnOffTransition(TD_on),
  _value(texture)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::off
//       Access: Public
//  Description: This named constructor returns a TextureTransition that
//               turns off texturing, thus rendering geometry
//               untextured.
////////////////////////////////////////////////////////////////////
INLINE TextureTransition TextureTransition::
off() {
  TextureTransition result;
  result.set_off();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::set_on
//       Access: Public
//  Description: Changes the TextureTransition to render the particular
//               texture.
////////////////////////////////////////////////////////////////////
INLINE void TextureTransition::
set_on(Texture *texture) {
  nassertv(texture != (Texture *)NULL);
  _value = texture;
  OnOffTransition::set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureTransition::get_texture
//       Access: Public
//  Description: Returns the texture that the TextureTransition
//               represents.  It is only valid to call this if is_on()
//               has returned true.
////////////////////////////////////////////////////////////////////
INLINE PT(Texture) TextureTransition::
get_texture() const {
  nassertr(is_on(), NULL);
  return _value;
}
