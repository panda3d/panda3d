// Filename: cLwoPolygons.I
// Created by:  drose (25Apr01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CLwoPolygons::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CLwoPolygons::
CLwoPolygons(LwoToEggConverter *converter, const LwoPolygons *polygons,
             CLwoPoints *points) : 
  _converter(converter),
  _polygons(polygons),
  _points(points)
{
  _tags = (LwoTags *)NULL;
  _surf_ptags = (LwoPolygonTags *)NULL;
}
