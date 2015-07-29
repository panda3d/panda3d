// Filename: lmatrix4_ext_src.h
// Created by:  rdb (12Sep13)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//       Class : Extension<LMatrix4>
// Description : This class defines the extension methods for
//               LMatrix4, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<FLOATNAME(LMatrix4)> : public ExtensionBase<FLOATNAME(LMatrix4)> {
public:
  INLINE_LINMATH PyObject *__reduce__(PyObject *self) const;
  INLINE_LINMATH string __repr__() const;
};

#include "lmatrix4_ext_src.I"
