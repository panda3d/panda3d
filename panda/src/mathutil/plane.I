// Filename: plane.I
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "plane.h"
#include "mathutil.h"

#include <nearly_zero.h>
#include <cmath.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: Plane::Constructor
//       Access: Public
//  Description: Creates a default plane.  This plane happens to
//               intersect the origin, perpendicular to the Z axis.
//               It's not clear how useful a default plane is.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE Plane<NumType>::
Plane(void) {
  _a = 0.0; 
  _b = 0.0; 
  _c = 1.0; 
  _d = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE Plane<NumType>::
Plane(const Plane<NumType> &copy) :
  _a(copy._a),
  _b(copy._b),
  _c(copy._c),
  _d(copy._d)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::Constructor
//       Access: Public
//  Description: Constructs a plane given three counter-clockwise
//               points, as seen from the front of the plane (that is,
//               viewed from the end of the normal vector, looking
//               down).
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE Plane<NumType>::
Plane(const LPoint3<NumType> &a, const LPoint3<NumType> &b, 
      const LPoint3<NumType> &c) {
  LVector3<NumType> u = b - a;
  LVector3<NumType> v = c - a;
  LVector3<NumType> p = normalize(cross(u, v));

  _a = p[0];
  _b = p[1];
  _c = p[2];
  _d = -dot(p, a);
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::Constructor
//       Access: Public
//  Description: Constructs a plane given a surface normal vector and
//               a point within the plane.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE Plane<NumType>::
Plane(const LVector3<NumType> &normal, const LPoint3<NumType> &point) {
  LVector3<NumType> p = normalize(normal);

  _a = p[0];
  _b = p[1]; 
  _c = p[2];
  _d = -dot(p, point);
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::Operator = 
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE Plane<NumType>& Plane<NumType>::
operator = (const Plane<NumType>& p) {
  _a = p._a; 
  _b = p._b;
  _c = p._c;
  _d = p._d;  
  return (*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::Operator * LMatrix3
//       Access: Public
//  Description: Transforms the plane by the indicated matrix.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE Plane<NumType> Plane<NumType>::
operator * (const LMatrix3<NumType> &mat) const {
  LVector3<NumType> new_normal = get_normal() * mat;
  LPoint3<NumType> new_point = get_point() * mat;
  return Plane<NumType>(new_normal, new_point);
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::Operator * LMatrix4
//       Access: Public
//  Description: Transforms the plane by the indicated matrix.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE Plane<NumType> Plane<NumType>::
operator * (const LMatrix4<NumType> &mat) const {
  LVector3<NumType> new_normal = get_normal() * mat;
  LPoint3<NumType> new_point = get_point() * mat;
  return Plane<NumType>(new_normal, new_point);
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::get_reflection_mat
//       Access: Public
//  Description: This computes a transform matrix that performs the
//               perspective transform defined by the frustum,
//               accordinate to the indicated coordinate system.
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix4<NumType> Plane<NumType>::
get_reflection_mat(void) const {

  NumType aa = _a * _a; NumType ab = _a * _b; NumType ac = _a * _c;
  NumType ad = _a * _d;
  NumType bb = _b * _b; NumType bc = _b * _c; NumType bd = _b * _d;
  NumType cc = _c * _c; NumType cd = _c * _d;

  return LMatrix4<NumType>(  1-2*aa,  -2*ab,  -2*ac,     0,
                              -2*ab, 1-2*bb,  -2*bc,     0, 
                              -2*ac,  -2*bc, 1-2*cc,     0,
                              -2*ad,  -2*bd,  -2*cd,     1  );
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::get_normal
//       Access: Public
//  Description: Returns the surface normal of the plane.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> Plane<NumType>::
get_normal() const {
  return LVector3<NumType>(_a, _b, _c);
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::get_point
//       Access: Public
//  Description: Returns an arbitrary point in the plane.  This can be
//               used along with the normal returned by get_normal()
//               to reconstruct the plane.
////////////////////////////////////////////////////////////////////
template<class NumType>
LPoint3<NumType> Plane<NumType>::
get_point() const {
  // Choose the denominator based on the largest axis in the normal.
  if (cabs(_a) >= cabs(_b) && cabs(_a) >= cabs(_c)) {
    nassertr(_a != 0.0, LPoint3<NumType>(0.0, 0.0, 0.0));
    return LPoint3<NumType>(-_d / _a, 0.0, 0.0);
  } else if (cabs(_b) >= cabs(_c)) {
    nassertr(_b != 0.0, LPoint3<NumType>(0.0, 0.0, 0.0));
    return LPoint3<NumType>(0.0, -_d / _b, 0.0);
  } else {
    nassertr(_c != 0.0, LPoint3<NumType>(0.0, 0.0, 0.0));
    return LPoint3<NumType>(0.0, 0.0, -_d / _c);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::dist_to_plane
//       Access: Public
//  Description: Returns the straight-line shortest distance from the
//               point to the plane.  The returned value is positive
//               if the point is in front of the plane (on the side
//               with the normal), or negative in the point is behind
//               the plane (on the opposite side from the normal).
//               It's zero if the point is exactly in the plane.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType Plane<NumType>::
dist_to_plane(const LPoint3<NumType> &point) const {
  return (_a * point[0] + _b * point[1] + _c * point[2] + _d);
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::intersects_line
//       Access: Public
//  Description: Returns true if the plane intersects the infinite
//               line passing through points p1 and p2, false if the
//               line is parallel.  The points p1 and p2 are used only
//               to define the Euclidean line; they have no other
//               bearing on the intersection test.  If true, sets
//               intersection_point to the point of intersection.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool Plane<NumType>::
intersects_line(LPoint3<NumType> &intersection_point,
		const LPoint3<NumType> &p1,
		const LPoint3<NumType> &p2) const {
  NumType t;
  if (!intersects_line(t, p1, p2 - p1)) {
    return false;
  }
  intersection_point = p1 + t * (p2 - p1);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::intersects_line
//       Access: Public
//  Description: This flavor of intersects_line() returns a bit more
//               information about the nature of the intersecting
//               point.  The line is defined via the parametric
//               equation from + t * delta for all real values of t.
//
//               If there is no intersection with the plane, the
//               function returns false and leaves t undefined.  If
//               there is an intersection with the plane, the function
//               returns true and sets t to the parametric value that
//               defines the point of intersection.  That is, t == 0.0
//               implies that the intersection occurred exactly at
//               point from, and t == 1.0 implies at point from +
//               delta, with other values of t accordingly.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool Plane<NumType>::
intersects_line(NumType &t, 
		const LPoint3<NumType> &from, 
		const LVector3<NumType> &delta) const {
  NumType denom = dot(get_normal(), delta);
  if (IS_NEARLY_ZERO(denom)) {
    return false;
  }
  
  t = -(dist_to_plane(from) / denom);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void Plane<NumType>::
output(ostream &out) const {
  out << "Plane(" << _a << " " << _b << " " << _c << " " << _d << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void Plane<NumType>::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::write_datagram
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void Plane<NumType>::
write_datagram(Datagram &dest) 
{
  dest.add_float64(_a);
  dest.add_float64(_b);
  dest.add_float64(_c);
  dest.add_float64(_d);
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::read_datagram
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void Plane<NumType>::
read_datagram(DatagramIterator &source) 
{
  _a = source.get_float64();
  _b = source.get_float64();
  _c = source.get_float64();
  _d = source.get_float64();
}
