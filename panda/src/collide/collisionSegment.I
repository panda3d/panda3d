// Filename: collisionSegment.I
// Created by:  drose (30Jan01)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::Default Constructor
//       Access: Public
//  Description: Creates an invalid segment.  This isn't terribly useful;
//               it's expected that the user will subsequently adjust
//               the segment via set_origin()/set_direction() or
//               set_projection().
////////////////////////////////////////////////////////////////////
INLINE CollisionSegment::
CollisionSegment() :
  _a(LPoint3f(0.0, 0.0, 0.0)), 
  _b(LPoint3f(0.0, 0.0, 0.0)) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CollisionSegment::
CollisionSegment(const LPoint3f &a, const LPoint3f &b) :
  _a(a), _b(b) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CollisionSegment::
CollisionSegment(float ax, float ay, float az,
		 float bx, float by, float bz) :
  _a(ax, ay, az), _b(bx, by, bz)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CollisionSegment::
CollisionSegment(const CollisionSegment &copy) :
  CollisionSolid(copy),
  _a(copy._a), 
  _b(copy._b)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::set_point_a
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CollisionSegment::
set_point_a(const LPoint3f &a) {
  _a = a;
  mark_bound_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::set_point_a
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CollisionSegment::
set_point_a(float x, float y, float z) {
  set_point_a(LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::get_point_a
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &CollisionSegment::
get_point_a() const {
  return _a;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::set_point_b
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CollisionSegment::
set_point_b(const LPoint3f &b) {
  _b = b;
  mark_bound_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::set_point_b
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CollisionSegment::
set_point_b(float x, float y, float z) {
  set_point_b(LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::get_point_b
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &CollisionSegment::
get_point_b() const {
  return _b;
}
