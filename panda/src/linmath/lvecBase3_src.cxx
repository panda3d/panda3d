// Filename: lvecBase3_src.cxx
// Created by:  drose (08Mar00)
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


TypeHandle FLOATNAME(LVecBase3)::_type_handle;

const FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::_zero =
  FLOATNAME(LVecBase3)(0.0f, 0.0f, 0.0f);
const FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::_unit_x =
  FLOATNAME(LVecBase3)(1.0f, 0.0f, 0.0f);
const FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::_unit_y =
  FLOATNAME(LVecBase3)(0.0f, 1.0f, 0.0f);
const FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::_unit_z =
  FLOATNAME(LVecBase3)(0.0f, 0.0f, 1.0f);


#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::__reduce__
//       Access: Published
//  Description: This special Python method is implement to provide
//               support for the pickle module.
////////////////////////////////////////////////////////////////////
PyObject *FLOATNAME(LVecBase3)::
__reduce__(PyObject *self) const {
  // We should return at least a 2-tuple, (Class, (args)): the
  // necessary class object whose constructor we should call
  // (e.g. this), and the arguments necessary to reconstruct this
  // object.
  PyObject *this_class = PyObject_Type(self);
  if (this_class == NULL) {
    return NULL;
  }

  PyObject *result = Py_BuildValue("(O(fff))", this_class, 
                                   (*this)[0], (*this)[1], (*this)[2]);
  Py_DECREF(this_class);
  return result;
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LVecBase3)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    string name = "LVecBase3";
    name += FLOATTOKEN;
    register_type(_type_handle, name);
  }
}
