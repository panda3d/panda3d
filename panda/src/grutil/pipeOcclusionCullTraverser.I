// Filename: pipeOcclusionCullTraverser.I
// Created by:  drose (29May07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: PipeOcclusionCullTraverser::get_buffer
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GraphicsOutput *PipeOcclusionCullTraverser::
get_buffer() const {
  return _buffer;
}

////////////////////////////////////////////////////////////////////
//     Function: PipeOcclusionCullTraverser::set_occlusion_mask
//       Access: Public
//  Description: Specifies the DrawMask that should be set on
//               occlusion polygons for this scene.  This identifies
//               the polygons that are to be treated as occluders.
//               Polygons that do not have this draw mask set will not
//               be considered occluders.
////////////////////////////////////////////////////////////////////
INLINE void PipeOcclusionCullTraverser::
set_occlusion_mask(const DrawMask &occlusion_mask) {
  _occlusion_mask = occlusion_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: PipeOcclusionCullTraverser::get_occlusion_mask
//       Access: Public
//  Description: Returns the DrawMask for occlusion polygons.  See
//               set_occlusion_mask().
////////////////////////////////////////////////////////////////////
INLINE const DrawMask &PipeOcclusionCullTraverser::
get_occlusion_mask() const {
  return _occlusion_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: PipeOcclusionCullTraverser::PendingObject::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PipeOcclusionCullTraverser::PendingObject::
PendingObject(CullableObject *object) :
  _object(object)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PipeOcclusionCullTraverser::PendingObject::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PipeOcclusionCullTraverser::PendingObject::
~PendingObject() {
  // Tempting as it is, we shouldn't delete the CullableObject in the
  // destructor, since it has already been deleted.
  //  delete _object;
}
