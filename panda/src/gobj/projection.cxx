// Filename: projection.cxx
// Created by:  drose (18Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "projection.h"

TypeHandle Projection::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Projection::make_geometry
//       Access: Public, Virtual
//  Description: Allocates and returns a new Geom that can be rendered
//               to show a visible representation of the frustum used
//               for this kind of projection, if it makes sense to do
//               so.  If a visible representation cannot be created,
//               returns NULL.
////////////////////////////////////////////////////////////////////
Geom *Projection::
make_geometry(const Colorf &, CoordinateSystem) const {
  return (Geom *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Projection::make_bounds
//       Access: Public, Virtual
//  Description: Allocates and returns a new BoundingVolume that
//               encloses the frustum used for this kind of
//               projection, if possible.  If a suitable bounding
//               volume cannot be created, returns NULL.
////////////////////////////////////////////////////////////////////
BoundingVolume *Projection::
make_bounds(CoordinateSystem) const {
  return (BoundingVolume *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Projection::extrude
//       Access: Public, Virtual
//  Description: Given a 2-d point in the range (-1,1) in both
//               dimensions, where (0,0) is the center of the
//               projection and (-1,-1) is the lower-left corner,
//               compute the corresponding vector in space that maps
//               to this point, if such a vector can be determined.
//               Returns true if the vector is defined (in which case
//               origin and direction are set to define the vector),
//               or false otherwise.
////////////////////////////////////////////////////////////////////
bool Projection::
extrude(const LPoint2f &, LPoint3f &, LVector3f &, CoordinateSystem) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Projection::project
//       Access: Public, Virtual
//  Description: Given a 3-d point in space, determine the 2-d point
//               this maps to, in the range (-1,1) in both dimensions,
//               where (0,0) is the center of the projection and
//               (-1,-1) is the lower-left corner.  Returns true if
//               the 3-d point is in front of the projection and
//               within the viewing frustum (in which case point2d is
//               filled in), or false otherwise.
////////////////////////////////////////////////////////////////////
bool Projection::
project(const LPoint3f &, LPoint2f &, CoordinateSystem) const {
  return false;
}
