// Filename: perspectiveProjection.I
// Created by:  drose (18Feb99)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: PerspectiveProjection::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PerspectiveProjection::
PerspectiveProjection(const Frustumf &frustum) : _frustum(frustum) {
}


////////////////////////////////////////////////////////////////////
//     Function: PerspectiveProjection::get_frustum
//       Access: Public
//  Description: Returns the frustum that defines the perspective
//               projection.
////////////////////////////////////////////////////////////////////
INLINE const Frustumf &PerspectiveProjection::
get_frustum() const {
  return _frustum;
}

INLINE void PerspectiveProjection::
set_frustum(const Frustumf &frust) {
  _frustum = frust;
}

