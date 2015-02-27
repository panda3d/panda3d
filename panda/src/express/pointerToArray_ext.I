// Filename: pointerToArray_ext.I
// Created by:  rdb (08Feb15)
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
//     Function: PointerToArray::__init__
//       Access: Published
//  Description: This special constructor accepts a Python list of
//               elements, or a Python string (or a bytes object,
//               in Python 3), or any object that supports the
//               Python buffer protocol.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void Extension<PointerToArray<Element> >::
__init__(PyObject *self, PyObject *source) {
#if PY_VERSION_HEX >= 0x02060000
  if (PyObject_CheckBuffer(source)) {
    // User passed a buffer object.
    Py_buffer view;
    if (PyObject_GetBuffer(source, &view, PyBUF_CONTIG_RO) == -1) {
      PyErr_SetString(PyExc_TypeError,
                      "PointerToArray constructor requires a contiguous buffer");
      return;
    }

    if (view.itemsize != 1 && view.itemsize != sizeof(Element)) {
      PyErr_SetString(PyExc_TypeError,
                      "buffer.itemsize does not match PointerToArray element size");
      return;
    }

    if (view.len % sizeof(Element) != 0) {
      PyErr_Format(PyExc_ValueError,
                   "byte buffer is not a multiple of %zu bytes",
                   sizeof(Element));
      return;
    }

    if (view.len > 0) {
      this->_this->resize(view.len / sizeof(Element));
      memcpy(this->_this->p(), view.buf, view.len);
    }

    PyBuffer_Release(&view);
    return;
  }
#endif

  if (!PySequence_Check(source)) {
    // If passed with a non-sequence, this isn't the right constructor.
    PyErr_SetString(PyExc_TypeError,
                    "PointerToArray constructor requires a sequence or buffer object");
    return;
  }

  // If we were passed a Python string, then instead of storing it
  // character-at-a-time, just load the whole string as a data
  // buffer.  Not sure if this case is still necessary - don't Python
  // str/bytes objects export the buffer protocol, as above?
#if PY_MAJOR_VERSION >= 3
  if (PyBytes_Check(source)) {
    int size = PyBytes_Size(source);
    if (size % sizeof(Element) != 0) {
      PyErr_Format(PyExc_ValueError,
                   "bytes object is not a multiple of %zu bytes",
                   sizeof(Element));
      return;
    }

    int num_elements = size / sizeof(Element);
    this->_this->insert(this->_this->begin(), num_elements, Element());

    // Hope there aren't any constructors or destructors involved
    // here.
    if (size != 0) {
      const char *data = PyBytes_AsString(source);
      memcpy(this->_this->p(), data, size);
    }
    return;
  }
#else
  if (PyString_CheckExact(source)) {
    int size = PyString_Size(source);
    if (size % sizeof(Element) != 0) {
      PyErr_Format(PyExc_ValueError,
                   "str object is not a multiple of %zu bytes",
                   sizeof(Element));
      return;
    }

    int num_elements = size / sizeof(Element);
    this->_this->insert(this->_this->begin(), num_elements, Element());

    // Hope there aren't any constructors or destructors involved
    // here.
    if (size != 0) {
      const char *data = PyString_AsString(source);
      memcpy(this->_this->p(), data, size);
    }
    return;
  }
#endif

  // Now construct the internal list by copying the elements
  // one-at-a-time from Python.
  PyObject *push_back = PyObject_GetAttrString(self, "push_back");
  if (push_back == NULL) {
    PyErr_BadArgument();
    return;
  }

  // We need to initialize the this pointer before we can call push_back.
  ((Dtool_PyInstDef *)self)->_ptr_to_object = (void *)this->_this;

  int size = PySequence_Size(source);
  for (int i = 0; i < size; ++i) {
    PyObject *item = PySequence_GetItem(source, i);
    if (item == NULL) {
      return;
    }
    PyObject *result = PyObject_CallFunctionObjArgs(push_back, item, NULL);
    Py_DECREF(item);
    if (result == NULL) {
      // Unable to add item--probably it wasn't of the appropriate type.
      PyErr_Print();
      PyErr_Format(PyExc_TypeError,
                   "Element %d in sequence passed to PointerToArray "
                   "constructor could not be added", i);
      return;
    }
    Py_DECREF(result);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::__getitem__
//       Access: Published
//  Description: Same as get_element(), this returns the nth element
//               of the array.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE const Element &Extension<PointerToArray<Element> >::
__getitem__(size_t n) const {
  return this->_this->get_element(n);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::__setitem__
//       Access: Published
//  Description: Same as set_element(), this replaces the nth element
//               of the array.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void Extension<PointerToArray<Element> >::
__setitem__(size_t n, const Element &value) {
  this->_this->set_element(n, value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::__init__
//       Access: Public
//  Description: This special constructor accepts a Python list of
//               elements, or a Python string (or a bytes object,
//               in Python 3).
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void Extension<ConstPointerToArray<Element> >::
__init__(PyObject *self, PyObject *source) {
  PointerToArray<Element> array;
  invoke_extension(&array).__init__(self, source);
  *(this->_this) = MOVE(array);
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::__getitem__
//       Access: Published
//  Description: Same as get_element(), this returns the nth element
//               of the array.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE const Element &Extension<ConstPointerToArray<Element> >::
__getitem__(size_t n) const {
  return (*this->_this)[n];
}

#if PY_VERSION_HEX >= 0x02060000
////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::__getbuffer__
//       Access: Published
//  Description: This is used to implement the buffer protocol, in
//               order to allow efficient access to the array data
//               through a Python multiview object.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE int Extension<PointerToArray<Element> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) {

  const char *format = get_format_code(Element);
  if (format == NULL) {
    // Not supported.
    return -1;
  }

  if (self != NULL) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void*) this->_this->p();
  view->len = this->_this->size() * sizeof(Element);
  view->readonly = 0;
  view->itemsize = sizeof(Element);
  view->format = NULL;
  if ((flags & PyBUF_FORMAT) == PyBUF_FORMAT) {
    view->format = (char*) format;
  }
  view->ndim = 1;
  view->shape = NULL;
  if ((flags & PyBUF_ND) == PyBUF_ND) {
    // This leaks, which sucks, but __releasebuffer__ doesn't give us
    // the same pointer, so we would need to store it elsewhere if we
    // wanted to delete it there.  Eh, it's just an int, who cares.
    view->shape = new Py_ssize_t(this->_this->size());
  }
  view->strides = NULL;
  if ((flags & PyBUF_STRIDES) == PyBUF_STRIDES) {
    view->strides = &(view->itemsize);
  }
  view->suboffsets = NULL;

  // Store a reference to ourselves on the Py_buffer object
  // as a reminder that we have increased our refcount.
  this->_this->ref();
  view->internal = (void*) this->_this;

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::__releasebuffer__
//       Access: Published
//  Description: Releases the buffer allocated by __getbuffer__.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void Extension<PointerToArray<Element> >::
__releasebuffer__(PyObject *self, Py_buffer *view) const {
  // Note: PyBuffer_Release automatically decrements view->obj.

  if (view->internal != NULL) {
    // Oh, right, let's not forget to unref this.
    unref_delete((const PointerToArray<Element> *)view->internal);
    view->internal = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::__getbuffer__
//       Access: Published
//  Description: This is used to implement the buffer protocol, in
//               order to allow efficient access to the array data
//               through a Python multiview object.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE int Extension<ConstPointerToArray<Element> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) const {

  if ((flags & PyBUF_WRITABLE) == PyBUF_WRITABLE) {
    PyErr_SetString(PyExc_BufferError,
                    "Object is not writable.");
    return -1;
  }

  const char *format = get_format_code(Element);
  if (format == NULL) {
    // Not supported.
    return -1;
  }

  if (self != NULL) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void*) this->_this->p();
  view->len = this->_this->size() * sizeof(Element);
  view->readonly = 1;
  view->itemsize = sizeof(Element);
  view->format = NULL;
  if ((flags & PyBUF_FORMAT) == PyBUF_FORMAT) {
    view->format = (char*) format;
  }
  view->ndim = 1;
  view->shape = NULL;
  if ((flags & PyBUF_ND) == PyBUF_ND) {
    // This leaks, which sucks, but __releasebuffer__ doesn't give us
    // the same pointer, so we would need to store it elsewhere if we
    // wanted to delete it there.  Eh, it's just an int, who cares.
    view->shape = new Py_ssize_t(this->_this->size());
  }
  view->strides = NULL;
  if ((flags & PyBUF_STRIDES) == PyBUF_STRIDES) {
    view->strides = &(view->itemsize);
  }
  view->suboffsets = NULL;

  // Store a reference to ourselves on the Py_buffer object
  // as a reminder that we have increased our refcount.
  this->_this->ref();
  view->internal = (void*) this->_this;

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::__releasebuffer__
//       Access: Published
//  Description: Releases the buffer allocated by __getbuffer__.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void Extension<ConstPointerToArray<Element> >::
__releasebuffer__(PyObject *self, Py_buffer *view) const {
  // Note: PyBuffer_Release automatically decrements obj->view.

  if (view->internal != NULL) {
    // Oh, right, let's not forget to unref this.
    unref_delete((const PointerToArray<Element> *)view->internal);
    view->internal = NULL;
  }
}
#endif  // PY_VERSION_HEX
