// Filename: lquaternion.h
// Created by:  frang (06Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __LQUATERNION_H__
#define __LQUATERNION_H__

#include "lmatrix.h"

////////////////////////////////////////////////////////////////////
//       Class : LQuaternionBase
// Description : This is the base quaternion class
////////////////////////////////////////////////////////////////////
template <class NumType>
class LQuaternionBase {
protected:
  INLINE LQuaternionBase<NumType> 
    multiply(const LQuaternionBase<NumType>&) const;

PUBLISHED:
  INLINE LQuaternionBase(void);
  INLINE LQuaternionBase(const LQuaternionBase<NumType> &);
  INLINE LQuaternionBase(NumType, NumType, NumType, NumType);
  virtual ~LQuaternionBase(void);

  static LQuaternionBase<NumType> pure_imaginary(const LVector3<NumType> &);

  INLINE LQuaternionBase<NumType>& operator =(const LQuaternionBase<NumType> &);
  INLINE bool operator ==(const LQuaternionBase<NumType> &) const;
  INLINE bool operator !=(const LQuaternionBase<NumType> &) const;

  INLINE LQuaternionBase<NumType> operator *(const LQuaternionBase<NumType> &);
  INLINE LQuaternionBase<NumType>& operator *=(const LQuaternionBase<NumType> &);

  INLINE LMatrix3<NumType> operator *(const LMatrix3<NumType> &);
  INLINE LMatrix4<NumType> operator *(const LMatrix4<NumType> &);

  INLINE bool almost_equal(const LQuaternionBase<NumType> &, NumType) const;
  INLINE bool almost_equal(const LQuaternionBase<NumType> &) const;

  INLINE void output(ostream&) const;

  INLINE void set(NumType, NumType, NumType, NumType);

  INLINE void set(const LMatrix3<NumType> &);
  INLINE void set(const LMatrix4<NumType> &);

  INLINE void extract_to_matrix(LMatrix3<NumType> &) const;
  INLINE void extract_to_matrix(LMatrix4<NumType> &) const;

  INLINE NumType get_r(void) const;
  INLINE NumType get_i(void) const;
  INLINE NumType get_j(void) const;
  INLINE NumType get_k(void) const;

  INLINE void set_r(NumType r);
  INLINE void set_i(NumType i);
  INLINE void set_j(NumType j);
  INLINE void set_k(NumType k);

  INLINE void normalize(void);

  static const LQuaternionBase<NumType> &ident_quat(void);

private:
  NumType _r, _i, _j, _k;
public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void);
private:
  static TypeHandle _type_handle;
};

template<class NumType>
INLINE ostream& operator<<(ostream& os, const LQuaternionBase<NumType>& q) {
  q.output(os);
  return os;
}

// matrix times quat
template<class NumType>
INLINE LMatrix3<NumType>
operator * (const LMatrix3<NumType> &m, const LQuaternionBase<NumType> &q);

template<class NumType>
INLINE LMatrix4<NumType>
operator * (const LMatrix4<NumType> &m, const LQuaternionBase<NumType> &q);

// pacify interrogate.
#ifdef CPPPARSER
BEGIN_PUBLISH
INLINE LMatrix3<float>
operator * (const LMatrix3<float> &m, const LQuaternionBase<float> &q);
INLINE LMatrix4<float>
operator * (const LMatrix4<float> &m, const LQuaternionBase<float> &q);
INLINE LMatrix3<double>
operator * (const LMatrix3<double> &m, const LQuaternionBase<double> &q);
INLINE LMatrix4<double>
operator * (const LMatrix4<double> &m, const LQuaternionBase<double> &q);
END_PUBLISH
#endif

// Cast to a different numeric type
template<class NumType, class NumType2>
INLINE LQuaternionBase<NumType2> lcast_to(NumType2*, const LQuaternionBase<NumType>&);

#include "lquaternion.I"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LQuaternionBase<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LQuaternionBase<double>)

#endif /* __LQUATERNION_H__ */
