// Filename: materialAttribute.I
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttribute::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MaterialAttribute::
MaterialAttribute() {
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttribute::set_on
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MaterialAttribute::
set_on(Material *material) {
  nassertv(material != (Material *)NULL);
  _value = material;
  OnOffAttribute::set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttribute::get_material
//       Access: Public
//  Description: Returns the material that the MaterialAttribute
//               represents.  It is only valid to call this if is_on()
//               has returned true.
////////////////////////////////////////////////////////////////////
INLINE PT(Material) MaterialAttribute::
get_material() const {
  nassertr(is_on(), NULL);
  return _value;
}
