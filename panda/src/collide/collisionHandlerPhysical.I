// Filename: collisionHandlerPhysical.I
// Created by:  drose (03Jul00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::ColliderDef::set_drive_interface
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerPhysical::ColliderDef::
set_drive_interface(DriveInterface *drive_interface) {
  _drive_interface = drive_interface;
  _arc.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::ColliderDef::set_arc
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerPhysical::ColliderDef::
set_arc(NodeRelation *arc) {
  _arc = arc;
  _drive_interface.clear();
}
