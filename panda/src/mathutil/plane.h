// Filename: plane.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef PLANE_H
#define PLANE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <luse.h>
#include <indent.h>

class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : Plane
// Description :
////////////////////////////////////////////////////////////////////
template<class NumType>
class Plane {
PUBLISHED:
  INLINE Plane(void);
  INLINE Plane(const Plane &copy);
  INLINE Plane(const LPoint3<NumType> &a, const LPoint3<NumType> &b,
	       const LPoint3<NumType> &c);
  INLINE Plane(const LVector3<NumType> &normal, 
	       const LPoint3<NumType> &point);

  INLINE Plane& operator = (const Plane& copy);

  INLINE Plane operator * (const LMatrix3<NumType> &mat) const;
  INLINE Plane operator * (const LMatrix4<NumType> &mat) const;

 
  LMatrix4<NumType>
  get_reflection_mat(void) const;

  INLINE LVector3<NumType> get_normal() const;
  LPoint3<NumType> get_point() const;
  INLINE NumType dist_to_plane(const LPoint3<NumType> &point) const; 
  INLINE bool intersects_line(LPoint3<NumType> &intersection_point,
			      const LPoint3<NumType> &p1,
			      const LPoint3<NumType> &p2) const;
  INLINE bool intersects_line(NumType &t,
			      const LPoint3<NumType> &from, 
			      const LVector3<NumType> &delta) const;
  
  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

public:
  INLINE void write_datagram(Datagram &dest);
  INLINE void read_datagram(DatagramIterator &source);

public:
  NumType _a, _b, _c, _d;
};

template<class NumType>
INLINE ostream &operator << (ostream &out, const Plane<NumType> &p) {
  p.output(out);
  return out;
}

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, Plane<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, Plane<double>)

typedef Plane<float> Planef;
typedef Plane<double> Planed;

#include "plane.I"


// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
