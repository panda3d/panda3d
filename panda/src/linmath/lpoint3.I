// Filename: lpoint3.I
// Created by:  drose (25Sep99)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// 	 Class : LPoint3
// Description : This is a three-component point in space (as opposed
//               to a three-component vector, which represents a
//               direction and a distance).  Some of the methods are
//               slightly different between LPoint3 and LVector3; in
//               particular, subtraction of two points yields a
//               vector, while addition of a vector and a point yields
//               a point.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA FLOATNAME(LPoint3) : public FLOATNAME(LVecBase3) {
PUBLISHED:
  INLINE FLOATNAME(LPoint3)();
  INLINE FLOATNAME(LPoint3)(const FLOATNAME(LVecBase3) &copy);
  INLINE FLOATNAME(LPoint3) &operator = (const FLOATNAME(LVecBase3) &copy);
  INLINE FLOATNAME(LPoint3) &operator = (FLOATTYPE1 fill_value);
  INLINE FLOATNAME(LPoint3)(FLOATTYPE1 fill_value);
  INLINE FLOATNAME(LPoint3)(FLOATTYPE1 x, FLOATTYPE1 y, FLOATTYPE1 z);

  INLINE static FLOATNAME(LPoint3) zero();
  INLINE static FLOATNAME(LPoint3) unit_x();
  INLINE static FLOATNAME(LPoint3) unit_y();
  INLINE static FLOATNAME(LPoint3) unit_z();

  INLINE FLOATNAME(LPoint3) operator - () const;

  INLINE FLOATNAME(LVecBase3)
  operator + (const FLOATNAME(LVecBase3) &other) const;
  INLINE FLOATNAME(LPoint3)
  operator + (const FLOATNAME(LVector3) &other) const;

  INLINE FLOATNAME(LVecBase3)
  operator - (const FLOATNAME(LVecBase3) &other) const;
  INLINE FLOATNAME(LVector3)
  operator - (const FLOATNAME(LPoint3) &other) const;
  INLINE FLOATNAME(LPoint3)
  operator - (const FLOATNAME(LVector3) &other) const;

  INLINE FLOATNAME(LPoint3) cross(const FLOATNAME(LVecBase3) &other) const;
  INLINE FLOATNAME(LPoint3) operator * (FLOATTYPE1 scalar) const;
  INLINE FLOATNAME(LPoint3) operator / (FLOATTYPE1 scalar) const;

  // Some special named constructors for LPoint3.

  INLINE static FLOATNAME(LPoint3) origin(CoordinateSystem cs = CS_default);
  INLINE static FLOATNAME(LPoint3) rfu(FLOATTYPE1 right,
				     FLOATTYPE1 fwd,
				     FLOATTYPE1 up,
				     CoordinateSystem cs = CS_default);
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
 
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3)::
FLOATNAME(LPoint3)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3)::
FLOATNAME(LPoint3)(const FLOATNAME(LVecBase3) &copy) : FLOATNAME(LVecBase3)(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3) &FLOATNAME(LPoint3)::
operator = (const FLOATNAME(LVecBase3) &copy) {
  FLOATNAME(LVecBase3)::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Copy Fill Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3) &FLOATNAME(LPoint3)::
operator = (FLOATTYPE1 fill_value) {
  FLOATNAME(LVecBase3)::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3)::
FLOATNAME(LPoint3)(FLOATTYPE1 fill_value) :
  FLOATNAME(LVecBase3)(fill_value) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3)::
FLOATNAME(LPoint3)(FLOATTYPE1 x, FLOATTYPE1 y, FLOATTYPE1 z) : 
  FLOATNAME(LVecBase3)(x, y, z) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::zero Named Constructor
//       Access: Public
//  Description: Returns a zero point.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
zero() {
  return FLOATNAME(LPoint3)(0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X point.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
unit_x() {
  return FLOATNAME(LPoint3)(1.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y point.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
unit_y() {
  return FLOATNAME(LPoint3)(0.0, 1.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::unit_z Named Constructor
//       Access: Public
//  Description: Returns a unit Z point.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
unit_z() {
  return FLOATNAME(LPoint3)(0.0, 0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::unary -
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
operator - () const {
  return FLOATNAME(LVecBase3)::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::point + vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase3) FLOATNAME(LPoint3)::
operator + (const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::point + vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
operator + (const FLOATNAME(LVector3) &other) const {
  return FLOATNAME(LVecBase3)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::point - vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase3) FLOATNAME(LPoint3)::
operator - (const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::point - point
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LPoint3)::
operator - (const FLOATNAME(LPoint3) &other) const {
  return FLOATNAME(LVecBase3)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::point - vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
operator - (const FLOATNAME(LVector3) &other) const {
  return FLOATNAME(LVecBase3)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::cross
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
cross(const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::cross(other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::operator * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
operator * (FLOATTYPE1 scalar) const {
  return FLOATNAME(LPoint3)(FLOATNAME(LVecBase3)::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::operator / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
operator / (FLOATTYPE1 scalar) const {
  return FLOATNAME(LPoint3)(FLOATNAME(LVecBase3)::operator / (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::origin
//       Access: Public, Static
//  Description: Returns the origin of the indicated coordinate
//               system.  This is always 0, 0, 0 with all of our
//               existing coordinate systems; it's hard to imagine it
//               ever being different.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
origin(CoordinateSystem) {
  return FLOATNAME(LPoint3)(0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::rfu
//       Access: Public, Static
//  Description: Returns a point described by right, forward, up
//               displacements from the origin, wherever that maps to
//               in the given coordinate system.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
rfu(FLOATTYPE1 right_v, FLOATTYPE1 fwd_v, FLOATTYPE1 up_v,
    CoordinateSystem cs) {
  return origin(cs) + 
    FLOATNAME(LVector3)::rfu(right_v, fwd_v, up_v, cs);
}

