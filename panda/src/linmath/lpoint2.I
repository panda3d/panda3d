// Filename: lpoint2.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
// 	 Class : LPoint2
// Description : This is a two-component point in space.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA FLOATNAME(LPoint2) : public FLOATNAME(LVecBase2) {
PUBLISHED:
  INLINE FLOATNAME(LPoint2)();
  INLINE FLOATNAME(LPoint2)(const FLOATNAME(LVecBase2) &copy);
  INLINE FLOATNAME(LPoint2) &operator = (const FLOATNAME(LVecBase2) &copy);
  INLINE FLOATNAME(LPoint2) &operator = (FLOATTYPE1 fill_value);
  INLINE FLOATNAME(LPoint2)(FLOATTYPE1 fill_value);
  INLINE FLOATNAME(LPoint2)(FLOATTYPE1 x, FLOATTYPE1 y);

  INLINE static FLOATNAME(LPoint2) zero();
  INLINE static FLOATNAME(LPoint2) unit_x();
  INLINE static FLOATNAME(LPoint2) unit_y();

  INLINE FLOATNAME(LPoint2) operator - () const;

  INLINE FLOATNAME(LVecBase2)
  operator + (const FLOATNAME(LVecBase2) &other) const;
  INLINE FLOATNAME(LPoint2)
  operator + (const FLOATNAME(LVector2) &other) const;

  INLINE FLOATNAME(LVecBase2)
  operator - (const FLOATNAME(LVecBase2) &other) const;
  INLINE FLOATNAME(LVector2)
  operator - (const FLOATNAME(LPoint2) &other) const;
  INLINE FLOATNAME(LPoint2)
  operator - (const FLOATNAME(LVector2) &other) const;

  INLINE FLOATNAME(LPoint2) operator * (FLOATTYPE1 scalar) const;
  INLINE FLOATNAME(LPoint2) operator / (FLOATTYPE1 scalar) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
 
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint2)::
FLOATNAME(LPoint2)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint2)::
FLOATNAME(LPoint2)(const FLOATNAME(LVecBase2) &copy) : FLOATNAME(LVecBase2)(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint2) &FLOATNAME(LPoint2)::
operator = (const FLOATNAME(LVecBase2) &copy) {
  FLOATNAME(LVecBase2)::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Copy Fill Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint2) &FLOATNAME(LPoint2)::
operator = (FLOATTYPE1 fill_value) {
  FLOATNAME(LVecBase2)::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint2)::
FLOATNAME(LPoint2)(FLOATTYPE1 fill_value) :
  FLOATNAME(LVecBase2)(fill_value) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint2)::
FLOATNAME(LPoint2)(FLOATTYPE1 x, FLOATTYPE1 y) : 
  FLOATNAME(LVecBase2)(x, y) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::zero Named Constructor
//       Access: Public
//  Description: Returns a zero point.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
zero() {
  return FLOATNAME(LPoint2)(0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X point.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
unit_x() {
  return FLOATNAME(LPoint2)(1.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y point.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
unit_y() {
  return FLOATNAME(LPoint2)(0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::unary -
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
operator - () const {
  return FLOATNAME(LVecBase2)::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::point + vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2) FLOATNAME(LPoint2)::
operator + (const FLOATNAME(LVecBase2) &other) const {
  return FLOATNAME(LVecBase2)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::point + vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
operator + (const FLOATNAME(LVector2) &other) const {
  return FLOATNAME(LVecBase2)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::point - vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2) FLOATNAME(LPoint2)::
operator - (const FLOATNAME(LVecBase2) &other) const {
  return FLOATNAME(LVecBase2)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::point - point
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2) FLOATNAME(LPoint2)::
operator - (const FLOATNAME(LPoint2) &other) const {
  return FLOATNAME(LVecBase2)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::point - vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
operator - (const FLOATNAME(LVector2) &other) const {
  return FLOATNAME(LVecBase2)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::operator * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
operator * (FLOATTYPE1 scalar) const {
  return FLOATNAME(LPoint2)(FLOATNAME(LVecBase2)::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::operator / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint2) FLOATNAME(LPoint2)::
operator / (FLOATTYPE1 scalar) const {
  return FLOATNAME(LPoint2)(FLOATNAME(LVecBase2)::operator / (scalar));
}


