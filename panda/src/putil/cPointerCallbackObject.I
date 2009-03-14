// Filename: cPointerCallbackObject.I
// Created by:  drose (13Mar09)
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
//     Function: CPointerCallbackObject::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CPointerCallbackObject::
CPointerCallbackObject(CPointerCallbackObject::CallbackFunction *func, void *data) :
  _func(func),
  _data(data)
{
}
