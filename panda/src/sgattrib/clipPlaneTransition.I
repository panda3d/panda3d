// Filename: clipPlaneTransition.I
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ClipPlaneTransition::
ClipPlaneTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::all_off
//       Access: Public
//  Description: This named constructor returns a ClipPlaneTransition
//               that's preconfigured to turn off all planes below it.
////////////////////////////////////////////////////////////////////
INLINE ClipPlaneTransition ClipPlaneTransition::
all_off() {
  ClipPlaneTransition t;
  t.set_default_dir(TD_off);
  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::set_identity
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ClipPlaneTransition::
set_identity(const PT(PlaneNode) &property) {
  MultiNodeTransition::set_identity(property.p());
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::set_on
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ClipPlaneTransition::
set_on(const PT(PlaneNode) &property) {
  MultiNodeTransition::set_on(property.p());
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::set_off
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ClipPlaneTransition::
set_off(const PT(PlaneNode) &property) {
  MultiNodeTransition::set_off(property.p());
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::is_identity
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ClipPlaneTransition::
is_identity(const PT(PlaneNode) &property) const {
  return MultiNodeTransition::is_identity(property.p());
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::is_on
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ClipPlaneTransition::
is_on(const PT(PlaneNode) &property) const {
  return MultiNodeTransition::is_on(property.p());
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::is_off
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ClipPlaneTransition::
is_off(const PT(PlaneNode) &property) const {
  return MultiNodeTransition::is_off(property.p());
}
