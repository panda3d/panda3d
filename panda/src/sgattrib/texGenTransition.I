// Filename: texGenTransition.I
// Created by:  drose (24Sep99)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: TexGenTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TexGenTransition::
TexGenTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenTransition::texture_projector
//       Access: Public
//  Description: This named constructor creates a transition to
//               project textures through a perpective frustum.
////////////////////////////////////////////////////////////////////
INLINE TexGenTransition TexGenTransition::
texture_projector() {
  TexGenTransition result;
  result.set_texture_projector();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenTransition::sphere_map
//       Access: Public
//  Description: This named constructor creates a transition to
//               project textures as sphere maps.
////////////////////////////////////////////////////////////////////
INLINE TexGenTransition TexGenTransition::
sphere_map() {
  TexGenTransition result;
  result.set_sphere_map();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenTransition::set_none
//       Access: Public
//  Description: Sets the transition to disable texture coordinate
//               generation.
////////////////////////////////////////////////////////////////////
INLINE void TexGenTransition::
set_none() {
  _value = TexGenProperty();
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenTransition::set_texture_projector
//       Access: Public
//  Description: Sets the transition to project textures through a
//               perpective frustum.
////////////////////////////////////////////////////////////////////
INLINE void TexGenTransition::
set_texture_projector() {
  _value = TexGenProperty::texture_projector();
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenTransition::set_sphere_map
//       Access: Public
//  Description: Sets the transition to project textures as sphere
//               maps.
////////////////////////////////////////////////////////////////////
INLINE void TexGenTransition::
set_sphere_map() {
  _value = TexGenProperty::sphere_map();
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenTransition::get_mode
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TexGenProperty::Mode TexGenTransition::
get_mode() const {
  return _value.get_mode();
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenTransition::get_plane
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE LMatrix4f TexGenTransition::
get_plane() const {
  return _value.get_plane();
}
