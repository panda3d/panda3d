// Filename: plane.I
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : FLOATNAME(Plane)
// Description :
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA FLOATNAME(Plane) {
PUBLISHED:
  INLINE FLOATNAME(Plane)(void);
  INLINE FLOATNAME(Plane)(const FLOATNAME(Plane) &copy);
  INLINE FLOATNAME(Plane)(const FLOATNAME(LPoint3) &a, const FLOATNAME(LPoint3) &b,
	       const FLOATNAME(LPoint3) &c);
  INLINE FLOATNAME(Plane)(const FLOATNAME(LVector3) &normal, 
	       const FLOATNAME(LPoint3) &point);

  INLINE FLOATNAME(Plane)& operator = (const FLOATNAME(Plane)& copy);

  INLINE FLOATNAME(Plane) operator * (const FLOATNAME(LMatrix3) &mat) const;
  INLINE FLOATNAME(Plane) operator * (const FLOATNAME(LMatrix4) &mat) const;
 
  INLINE FLOATNAME(LMatrix4) get_reflection_mat(void) const;

  INLINE FLOATNAME(LVector3) get_normal() const;
  INLINE FLOATNAME(LPoint3) get_point() const;
  INLINE FLOATTYPE1 dist_to_plane(const FLOATNAME(LPoint3) &point) const; 
  INLINE bool intersects_line(FLOATNAME(LPoint3) &intersection_point,
			      const FLOATNAME(LPoint3) &p1,
			      const FLOATNAME(LPoint3) &p2) const;
  INLINE bool intersects_line(FLOATTYPE1 &t,
			      const FLOATNAME(LPoint3) &from, 
			      const FLOATNAME(LVector3) &delta) const;
  
  INLINE void output(ostream &out) const;
  INLINE void write(ostream &out, int indent_level = 0) const;

public:
  INLINE void write_datagram(Datagram &dest);
  INLINE void read_datagram(DatagramIterator &source);

public:
  FLOATTYPE1 _a, _b, _c, _d;
};

INLINE ostream &operator << (ostream &out, const FLOATNAME(Plane) &p) {
  p.output(out);
  return out;
}

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

INLINE FLOATNAME(Plane)::
FLOATNAME(Plane)(void) {
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

INLINE FLOATNAME(Plane)::
FLOATNAME(Plane)(const FLOATNAME(Plane) &copy) :
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

INLINE FLOATNAME(Plane)::
FLOATNAME(Plane)(const FLOATNAME(LPoint3) &a, const FLOATNAME(LPoint3) &b, 
      const FLOATNAME(LPoint3) &c) {
  FLOATNAME(LVector3) u = b - a;
  FLOATNAME(LVector3) v = c - a;
  FLOATNAME(LVector3) p = normalize(cross(u, v));

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

INLINE FLOATNAME(Plane)::
FLOATNAME(Plane)(const FLOATNAME(LVector3) &normal, const FLOATNAME(LPoint3) &point) {
  FLOATNAME(LVector3) p = normalize(normal);

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

INLINE FLOATNAME(Plane)& FLOATNAME(Plane)::
operator = (const FLOATNAME(Plane)& p) {
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

INLINE FLOATNAME(Plane) FLOATNAME(Plane)::
operator * (const FLOATNAME(LMatrix3) &mat) const {
  FLOATNAME(LVector3) new_normal = get_normal() * mat;
  FLOATNAME(LPoint3) new_point = get_point() * mat;
  return FLOATNAME(Plane)(new_normal, new_point);
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::Operator * LMatrix4
//       Access: Public
//  Description: Transforms the plane by the indicated matrix.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(Plane) FLOATNAME(Plane)::
operator * (const FLOATNAME(LMatrix4) &mat) const {
  FLOATNAME(LVector3) new_normal = get_normal() * mat;
  FLOATNAME(LPoint3) new_point = get_point() * mat;
  return FLOATNAME(Plane)(new_normal, new_point);
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::get_reflection_mat
//       Access: Public
//  Description: This computes a transform matrix that performs the
//               perspective transform defined by the frustum,
//               accordinate to the indicated coordinate system.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LMatrix4) FLOATNAME(Plane)::
get_reflection_mat(void) const {

  FLOATTYPE1 aa = _a * _a; FLOATTYPE1 ab = _a * _b; FLOATTYPE1 ac = _a * _c;
  FLOATTYPE1 ad = _a * _d;
  FLOATTYPE1 bb = _b * _b; FLOATTYPE1 bc = _b * _c; FLOATTYPE1 bd = _b * _d;
  FLOATTYPE1 cc = _c * _c; FLOATTYPE1 cd = _c * _d;

  return FLOATNAME(LMatrix4)(  1-2*aa,  -2*ab,  -2*ac,     0,
                              -2*ab, 1-2*bb,  -2*bc,     0, 
                              -2*ac,  -2*bc, 1-2*cc,     0,
                              -2*ad,  -2*bd,  -2*cd,     1  );
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::get_normal
//       Access: Public
//  Description: Returns the surface normal of the plane.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(Plane)::
get_normal() const {
  return FLOATNAME(LVector3)(_a, _b, _c);
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::get_point
//       Access: Public
//  Description: Returns an arbitrary point in the plane.  This can be
//               used along with the normal returned by get_normal()
//               to reconstruct the plane.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3) FLOATNAME(Plane)::
get_point() const {
  // Choose the denominator based on the largest axis in the normal.
  if (cabs(_a) >= cabs(_b) && cabs(_a) >= cabs(_c)) {
    nassertr(_a != 0.0, FLOATNAME(LPoint3)(0.0, 0.0, 0.0));
    return FLOATNAME(LPoint3)(-_d / _a, 0.0, 0.0);
  } else if (cabs(_b) >= cabs(_c)) {
    nassertr(_b != 0.0, FLOATNAME(LPoint3)(0.0, 0.0, 0.0));
    return FLOATNAME(LPoint3)(0.0, -_d / _b, 0.0);
  } else {
    nassertr(_c != 0.0, FLOATNAME(LPoint3)(0.0, 0.0, 0.0));
    return FLOATNAME(LPoint3)(0.0, 0.0, -_d / _c);
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

INLINE FLOATTYPE1 FLOATNAME(Plane)::
dist_to_plane(const FLOATNAME(LPoint3) &point) const {
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

INLINE bool FLOATNAME(Plane)::
intersects_line(FLOATNAME(LPoint3) &intersection_point,
		const FLOATNAME(LPoint3) &p1,
		const FLOATNAME(LPoint3) &p2) const {
  FLOATTYPE1 t;
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

INLINE bool FLOATNAME(Plane)::
intersects_line(FLOATTYPE1 &t, 
		const FLOATNAME(LPoint3) &from, 
		const FLOATNAME(LVector3) &delta) const {
  FLOATTYPE1 denom = dot(get_normal(), delta);
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

INLINE void FLOATNAME(Plane)::
output(ostream &out) const {
  out << "Plane(" << _a << " " << _b << " " << _c << " " << _d << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(Plane)::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::write_datagram
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(Plane)::
write_datagram(Datagram &dest) 
{
  dest.add_float32(_a);
  dest.add_float32(_b);
  dest.add_float32(_c);
  dest.add_float32(_d);
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::read_datagram
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////


INLINE void FLOATNAME(Plane)::
read_datagram(DatagramIterator &source) 
{
  _a = source.get_float32();
  _b = source.get_float32();
  _c = source.get_float32();
  _d = source.get_float32();
}
