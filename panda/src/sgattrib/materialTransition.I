// Filename: materialTransition.I
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MaterialTransition::
MaterialTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MaterialTransition::
MaterialTransition(const Material *material) : 
  OnOffTransition(TD_on),
  _value(material)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::off
//       Access: Public
//  Description: This named constructor returns a MaterialTransition
//               that disables materials.
////////////////////////////////////////////////////////////////////
INLINE MaterialTransition MaterialTransition::
off() {
  MaterialTransition result;
  result.set_off();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::set_on
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MaterialTransition::
set_on(const Material *material) {
  nassertv(material != (const Material *)NULL);
  _value = material;
  OnOffTransition::set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::get_material
//       Access: Public
//  Description: Returns the material that the MaterialTransition
//               represents.  It is only valid to call this if is_on()
//               has returned true.
////////////////////////////////////////////////////////////////////
INLINE const Material *MaterialTransition::
get_material() const {
  nassertr(is_on(), NULL);
  return _value;
}
