// Filename: collisionPolygon.I
// Created by:  drose (25Apr00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CollisionPolygon::
CollisionPolygon(const LPoint3f &a, const LPoint3f &b,
		 const LPoint3f &c) {
  LPoint3f array[3];
  array[0] = a;
  array[1] = b;
  array[2] = c;
  setup_points(array, array + 3);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CollisionPolygon::
CollisionPolygon(const LPoint3f &a, const LPoint3f &b,
		 const LPoint3f &c, const LPoint3f &d) {
  LPoint3f array[4];
  array[0] = a;
  array[1] = b;
  array[2] = c;
  array[3] = d;
  setup_points(array, array + 4);
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CollisionPolygon::
CollisionPolygon(const LPoint3f *begin, const LPoint3f *end) {
  setup_points(begin, end);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CollisionPolygon::
CollisionPolygon(void) {
}
