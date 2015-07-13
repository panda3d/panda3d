// Filename: lvector3_ext_src.h
// Created by:  rdb (13Sep13)
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
//       Class : Extension<LVector3>
// Description : This class defines the extension methods for
//               LVector3, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<FLOATNAME(LVector3)> : public ExtensionBase<FLOATNAME(LVector3)> {
public:
  INLINE_LINMATH PyObject *__getattr__(PyObject *self, const string &attr_name) const;
  INLINE_LINMATH int __setattr__(PyObject *self, const string &attr_name, PyObject *assign);
  INLINE_LINMATH string __repr__() const;
};

#include "lvector3_ext_src.I"
