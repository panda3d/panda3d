/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lsimpleMatrix.h
 * @author drose
 * @date 2011-12-15
 */

#ifndef LSIMPLEMATRIX_H
#define LSIMPLEMATRIX_H

#include "pandabase.h"

#ifdef HAVE_EIGEN
#include <Eigen/Dense>
#endif

/**
 * This class provides an underlying storage of the various linear-algebra
 * classes (e.g.  LVecBase3, LMatrix4) in the absence of the Eigen linear
 * algebra library.
 */
template <class FloatType, int NumRows, int NumCols>
class LSimpleMatrix {
public:
  INLINE const FloatType &operator () (int row, int col) const;
  INLINE FloatType &operator () (int row, int col);
  INLINE const FloatType &operator () (int col) const;
  INLINE FloatType &operator () (int col);

private:
  FloatType _array[NumRows][NumCols];
};

#include "lsimpleMatrix.I"

// Now, do we actually use LSimpleMatrix, or do we use Eigen::Matrix?
#ifdef HAVE_EIGEN
#define UNALIGNED_LINMATH_MATRIX(FloatType, NumRows, NumCols) Eigen::Matrix<FloatType, NumRows, NumCols, Eigen::DontAlign | Eigen::RowMajor>

#ifdef LINMATH_ALIGN
#define LINMATH_MATRIX(FloatType, NumRows, NumCols) Eigen::Matrix<FloatType, NumRows, NumCols, Eigen::RowMajor>
#else  // LINMATH_ALIGN
#define LINMATH_MATRIX(FloatType, NumRows, NumCols) UNALIGNED_LINMATH_MATRIX(FloatType, NumRows, NumCols)
#endif  // LINMATH_ALIGN

#else  // HAVE_EIGEN
#define UNALIGNED_LINMATH_MATRIX(FloatType, NumRows, NumCols) LSimpleMatrix<FloatType, NumRows, NumCols>
#define LINMATH_MATRIX(FloatType, NumRows, NumCols) UNALIGNED_LINMATH_MATRIX(FloatType, NumRows, NumCols)
#endif  // HAVE_EIGEN

// This is as good a place as any to define this alignment macro.
#if defined(LINMATH_ALIGN) && defined(HAVE_EIGEN) && defined(__AVX__) && defined(STDFLOAT_DOUBLE)
#define ALIGN_LINMATH ALIGN_32BYTE
#elif defined(LINMATH_ALIGN)
#define ALIGN_LINMATH ALIGN_16BYTE
#else
#define ALIGN_LINMATH
#endif  // LINMATH_ALIGN

#endif
