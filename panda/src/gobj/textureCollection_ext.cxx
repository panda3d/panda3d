// Filename: textureCollection_ext.cxx
// Created by:  rdb (11Feb15)
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

#include "textureCollection_ext.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_Texture;
#endif

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::__init__
//       Access: Published
//  Description: This special constructor accepts a Python list of
//               Textures.  Since this constructor accepts a generic
//               PyObject *, it should be the last constructor listed
//               in the class record.
////////////////////////////////////////////////////////////////////
void Extension<TextureCollection>::
__init__(PyObject *self, PyObject *sequence) {
  PyObject *fast = PySequence_Fast(sequence, "TextureCollection constructor requires a sequence");
  if (fast == NULL) {
    return;
  }

  Py_ssize_t size = PySequence_Fast_GET_SIZE(fast);
  _this->reserve(size);

  for (int i = 0; i < size; ++i) {
    PyObject *item = PySequence_Fast_GET_ITEM(fast, i);
    if (item == NULL) {
      return;
    }

    Texture *tex;
    DTOOL_Call_ExtractThisPointerForType(item, &Dtool_Texture, (void **)&tex);
    if (tex == (Texture *)NULL) {
      // Unable to add item--probably it wasn't of the appropriate type.
      ostringstream stream;
      stream << "Element " << i << " in sequence passed to TextureCollection constructor is not a Texture";
      string str = stream.str();
      PyErr_SetString(PyExc_TypeError, str.c_str());
      Py_DECREF(fast);
      return;
    } else {
      _this->add_texture(tex);
    }
  }

  Py_DECREF(fast);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::__reduce__
//       Access: Published
//  Description: This special Python method is implement to provide
//               support for the pickle module.
////////////////////////////////////////////////////////////////////
PyObject *Extension<TextureCollection>::
__reduce__(PyObject *self) const {
  // Here we will return a 4-tuple: (Class, (args), None, iterator),
  // where iterator is an iterator that will yield successive
  // Textures.

  // We should return at least a 2-tuple, (Class, (args)): the
  // necessary class object whose constructor we should call
  // (e.g. this), and the arguments necessary to reconstruct this
  // object.

  PyObject *this_class = PyObject_Type(self);
  if (this_class == NULL) {
    return NULL;
  }

  // Since a TextureCollection is itself an iterator, we can simply
  // pass it as the fourth tuple component.
  PyObject *result = Py_BuildValue("(O()OO)", this_class, Py_None, self);
  Py_DECREF(this_class);
  return result;
}

#endif  // HAVE_PYTHON
