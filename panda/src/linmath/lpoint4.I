// Filename: lpoint4.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// 	 Class : LPoint4
// Description : This is a four-component point in space.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA FLOATNAME(LPoint4) : public FLOATNAME(LVecBase4) {
PUBLISHED:
  INLINE FLOATNAME(LPoint4)();
  INLINE FLOATNAME(LPoint4)(const FLOATNAME(LVecBase4) &copy);
  INLINE FLOATNAME(LPoint4) &operator = (const FLOATNAME(LVecBase4) &copy);
  INLINE FLOATNAME(LPoint4) &operator = (FLOATTYPE1 fill_value);
  INLINE FLOATNAME(LPoint4)(FLOATTYPE1 fill_value);
  INLINE FLOATNAME(LPoint4)(FLOATTYPE1 x, FLOATTYPE1 y, FLOATTYPE1 z, FLOATTYPE1 w);

  INLINE static FLOATNAME(LPoint4) zero();
  INLINE static FLOATNAME(LPoint4) unit_x();
  INLINE static FLOATNAME(LPoint4) unit_y();
  INLINE static FLOATNAME(LPoint4) unit_z();
  INLINE static FLOATNAME(LPoint4) unit_w();

  INLINE FLOATNAME(LPoint4) operator - () const;

  INLINE FLOATNAME(LVecBase4)
  operator + (const FLOATNAME(LVecBase4) &other) const;
  INLINE FLOATNAME(LPoint4)
  operator + (const FLOATNAME(LVector4) &other) const;

  INLINE FLOATNAME(LVecBase4)
  operator - (const FLOATNAME(LVecBase4) &other) const;
  INLINE FLOATNAME(LVector4)
  operator - (const FLOATNAME(LPoint4) &other) const;
  INLINE FLOATNAME(LPoint4)
  operator - (const FLOATNAME(LVector4) &other) const;

  INLINE FLOATNAME(LPoint4) operator * (FLOATTYPE1 scalar) const;
  INLINE FLOATNAME(LPoint4) operator / (FLOATTYPE1 scalar) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
 
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint4)::
FLOATNAME(LPoint4)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint4)::
FLOATNAME(LPoint4)(const FLOATNAME(LVecBase4) &copy) : FLOATNAME(LVecBase4)(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint4) &FLOATNAME(LPoint4)::
operator = (const FLOATNAME(LVecBase4) &copy) {
  FLOATNAME(LVecBase4)::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Copy Fill Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint4) &FLOATNAME(LPoint4)::
operator = (FLOATTYPE1 fill_value) {
  FLOATNAME(LVecBase4)::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint4)::
FLOATNAME(LPoint4)(FLOATTYPE1 fill_value) :
  FLOATNAME(LVecBase4)(fill_value) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint4)::
FLOATNAME(LPoint4)(FLOATTYPE1 x, FLOATTYPE1 y, FLOATTYPE1 z, FLOATTYPE1 w) : 
  FLOATNAME(LVecBase4)(x, y, z, w) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::zero Named Constructor
//       Access: Public
//  Description: Returns a zero point.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
zero() {
  return FLOATNAME(LPoint4)(0.0, 0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X point.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
unit_x() {
  return FLOATNAME(LPoint4)(1.0, 0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y point.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
unit_y() {
  return FLOATNAME(LPoint4)(0.0, 1.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::unit_z Named Constructor
//       Access: Public
//  Description: Returns a unit Z point.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
unit_z() {
  return FLOATNAME(LPoint4)(0.0, 0.0, 1.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::unit_w Named Constructor
//       Access: Public
//  Description: Returns a unit W point.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
unit_w() {
  return FLOATNAME(LPoint4)(0.0, 0.0, 0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::unary -
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
operator - () const {
  return FLOATNAME(LVecBase4)::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::point + vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase4) FLOATNAME(LPoint4)::
operator + (const FLOATNAME(LVecBase4) &other) const {
  return FLOATNAME(LVecBase4)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::point + vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
operator + (const FLOATNAME(LVector4) &other) const {
  return FLOATNAME(LVecBase4)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::point - vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase4) FLOATNAME(LPoint4)::
operator - (const FLOATNAME(LVecBase4) &other) const {
  return FLOATNAME(LVecBase4)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::point - point
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4) FLOATNAME(LPoint4)::
operator - (const FLOATNAME(LPoint4) &other) const {
  return FLOATNAME(LVecBase4)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::point - vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
operator - (const FLOATNAME(LVector4) &other) const {
  return FLOATNAME(LVecBase4)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::operator * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
operator * (FLOATTYPE1 scalar) const {
  return FLOATNAME(LPoint4)(FLOATNAME(LVecBase4)::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::operator / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint4) FLOATNAME(LPoint4)::
operator / (FLOATTYPE1 scalar) const {
  return FLOATNAME(LPoint4)(FLOATNAME(LVecBase4)::operator / (scalar));
}

