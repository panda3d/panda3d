// Filename: nurbsCurveDrawer.I
// Created by:  drose (05Mar01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::hilight
//       Access: Published
//  Description: Hilights a particular CV by showing it and its knot
//               in a different color.  Returns true if the CV exists
//               and has been drawn, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool NurbsCurveDrawer::
hilight(int n) {
  return hilight(n, 1.0, 1.0, 0.0);
}

