// Filename: lmatrix3_ext_src.h
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
//       Class : Extension<LMatrix3::Row>
// Description : This class defines the extension methods for
//               LMatrix3::Row, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<FLOATNAME(LMatrix3)::Row> : public ExtensionBase<FLOATNAME(LMatrix3)::Row> {
public:
  INLINE_LINMATH void __setitem__(int i, FLOATTYPE v);
};

////////////////////////////////////////////////////////////////////
//       Class : Extension<LMatrix3>
// Description : This class defines the extension methods for
//               LMatrix3, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<FLOATNAME(LMatrix3)> : public ExtensionBase<FLOATNAME(LMatrix3)> {
public:
  INLINE_LINMATH PyObject *__reduce__(PyObject *self) const;
  INLINE_LINMATH void python_repr(ostream &out, const string &class_name) const;
};

#include "lmatrix3_ext_src.I"
