// Filename: lquaternion_src.h
// Created by:  frang (06Jun00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : FLOATNAME(LQuaternion)
// Description : This is the base quaternion class
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LQuaternion) : public FLOATNAME(LVecBase4) {
protected:
  INLINE FLOATNAME(LQuaternion) 
    multiply(const FLOATNAME(LQuaternion)&) const;

PUBLISHED:
  INLINE FLOATNAME(LQuaternion)();
  INLINE FLOATNAME(LQuaternion)(const FLOATNAME(LQuaternion) &);
  INLINE FLOATNAME(LQuaternion)(FLOATTYPE, FLOATTYPE, FLOATTYPE, FLOATTYPE);

  static FLOATNAME(LQuaternion) pure_imaginary(const FLOATNAME(LVector3) &);

  INLINE FLOATNAME(LQuaternion) operator *(const FLOATNAME(LQuaternion) &);
  INLINE FLOATNAME(LQuaternion)& operator *=(const FLOATNAME(LQuaternion) &);

  INLINE FLOATNAME(LMatrix3) operator *(const FLOATNAME(LMatrix3) &);
  INLINE FLOATNAME(LMatrix4) operator *(const FLOATNAME(LMatrix4) &);

  INLINE bool almost_equal(const FLOATNAME(LQuaternion) &, FLOATTYPE) const;
  INLINE bool almost_equal(const FLOATNAME(LQuaternion) &) const;

  INLINE void output(ostream&) const;

  void extract_to_matrix(FLOATNAME(LMatrix3) &m) const;
  void extract_to_matrix(FLOATNAME(LMatrix4) &m) const;

  void set_from_matrix(const FLOATNAME(LMatrix3) &m);
  INLINE void set_from_matrix(const FLOATNAME(LMatrix4) &m);
  void set_hpr(const FLOATNAME(LVecBase3) &hpr);
  FLOATNAME(LVecBase3) get_hpr() const;

  INLINE FLOATTYPE get_r() const;
  INLINE FLOATTYPE get_i() const;
  INLINE FLOATTYPE get_j() const;
  INLINE FLOATTYPE get_k() const;

  INLINE void set_r(FLOATTYPE r);
  INLINE void set_i(FLOATTYPE i);
  INLINE void set_j(FLOATTYPE j);
  INLINE void set_k(FLOATTYPE k);

  INLINE bool normalize();

  INLINE static const FLOATNAME(LQuaternion) &ident_quat();

private:
  static const FLOATNAME(LQuaternion) _ident_quat;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
private:
  static TypeHandle _type_handle;
};


INLINE ostream& operator<<(ostream& os, const FLOATNAME(LQuaternion)& q) {
  q.output(os);
  return os;
}

BEGIN_PUBLISH
INLINE FLOATNAME(LMatrix3)
operator * (const FLOATNAME(LMatrix3) &m, const FLOATNAME(LQuaternion) &q);
INLINE FLOATNAME(LMatrix4)
operator * (const FLOATNAME(LMatrix4) &m, const FLOATNAME(LQuaternion) &q);
END_PUBLISH

#include "lquaternion_src.I"
