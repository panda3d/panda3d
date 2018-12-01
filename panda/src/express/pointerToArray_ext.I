/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerToArray_ext.I
 * @author rdb
 * @date 2015-02-08
 */

/**
 * This is a helper function to set most attributes of a Py_buffer in a manner
 * that accommodates square matrices (in accordance with PEP 3118). It is tested
 * for use with NumPy. The resulting array will be of shape
 * (num_matrices, size, size) where size is the number of matrix rows (=columns)
 */
INLINE void set_matrix_view(Py_buffer &view, int flags, int length, int size, bool double_prec, bool read_only) {
  int item_size, mat_size;
  const char *format;

  if (double_prec) {
    item_size = sizeof(double);
    format = get_format_code(double);
  } else {
    item_size = sizeof(float);
    format = get_format_code(float);
  }

  if (size == 3 && !double_prec) {
    mat_size = sizeof(LMatrix3f);
  } else if (size == 3 && double_prec) {
    mat_size = sizeof(LMatrix3d);
  } else if (size == 4 && !double_prec) {
    mat_size = sizeof(UnalignedLMatrix4f);
  } else if (size == 4 && double_prec) {
    mat_size = sizeof(UnalignedLMatrix4d);
  } else {
    nassertv_always(false);
    return; // Make sure compiler knows control flow doesn't proceed.
  }

  view.len = length * mat_size;
  view.readonly = (read_only ? 1 : 0);
  view.itemsize = item_size;
  view.format = nullptr;
  if ((flags & PyBUF_FORMAT) == PyBUF_FORMAT) {
    view.format = (char*) format;
  }
  view.ndim = 3;
  view.shape = nullptr;
  if ((flags & PyBUF_ND) == PyBUF_ND) {
    // This leaks, which sucks, but __releasebuffer__ doesn't give us the same
    // pointer, so we would need to store it elsewhere if we wanted to delete
    // it there.  Eh, it's just an int, who cares.
    Py_ssize_t* shape = new Py_ssize_t[3];
    shape[0] = length;
    shape[1] = size;
    shape[2] = size;
    view.shape = shape;
  }
  view.strides = nullptr;
  if ((flags & PyBUF_STRIDES) == PyBUF_STRIDES) {
    Py_ssize_t* strides = new Py_ssize_t[3];
    strides[0] = mat_size;
    strides[1] = item_size * size;
    strides[2] = item_size;
    view.strides = strides;
  }
  view.suboffsets = nullptr;
}

/**
 * This special constructor accepts a Python list of elements, or a Python
 * string (or a bytes object, in Python 3), or any object that supports the
 * Python buffer protocol.
 */
template<class Element>
INLINE void Extension<PointerToArray<Element> >::
__init__(PyObject *self, PyObject *source) {
#if PY_VERSION_HEX >= 0x02060000
  if (PyObject_CheckBuffer(source)) {
#else
  if (PyString_CheckExact(source)) {
#endif
    // It's a byte sequence, or any object that exports the buffer protocol.
    this->set_data(source);
    return;
  }

  // Don't allow a unicode object even though it's a sequence.
  if (!PySequence_Check(source) || PyUnicode_CheckExact(source)) {
    // If passed with a non-sequence, this isn't the right constructor.
    PyErr_SetString(PyExc_TypeError,
                    "PointerToArray constructor requires a sequence or buffer object");
    return;
  }

  // Now construct the internal list by copying the elements one-at-a-time
  // from Python.
  PyObject *dict = DtoolInstance_TYPE(self)->_PyType.tp_dict;
  PyObject *push_back = PyDict_GetItemString(dict, "push_back");
  if (push_back == nullptr) {
    PyErr_BadArgument();
    return;
  }

  // We need to initialize the this pointer before we can call push_back.
  DtoolInstance_INIT_PTR(self, this->_this);

  Py_ssize_t size = PySequence_Size(source);
  this->_this->reserve(size);
  for (Py_ssize_t i = 0; i < size; ++i) {
    PyObject *item = PySequence_GetItem(source, i);
    if (item == nullptr) {
      return;
    }
    PyObject *result = PyObject_CallFunctionObjArgs(push_back, self, item, nullptr);
    Py_DECREF(item);
    if (result == nullptr) {
      // Unable to add item--probably it wasn't of the appropriate type.
      PyErr_Print();
      PyErr_Format(PyExc_TypeError,
                   "Element %zd in sequence passed to PointerToArray "
                   "constructor could not be added", i);
      return;
    }
    Py_DECREF(result);
  }
}

/**
 * Same as get_element(), this returns the nth element of the array.
 */
template<class Element>
INLINE const Element &Extension<PointerToArray<Element> >::
__getitem__(size_t n) const {
  return this->_this->get_element(n);
}

/**
 * Same as set_element(), this replaces the nth element of the array.
 */
template<class Element>
INLINE void Extension<PointerToArray<Element> >::
__setitem__(size_t n, const Element &value) {
  this->_this->set_element(n, value);
}

/**
 * This returns the entire contents of the vector as a block of raw data in a
 * string (or bytes object, in Python 3).
 *
 * @deprecated use memoryview(pta) or bytearray(pta) instead.
 */
template<class Element>
INLINE PyObject *Extension<PointerToArray<Element> >::
get_data() const {
#if PY_MAJOR_VERSION >= 3
  return PyBytes_FromStringAndSize((char *)this->_this->p(), sizeof(Element) * this->_this->size());
#else
  return PyString_FromStringAndSize((char *)this->_this->p(), sizeof(Element) * this->_this->size());
#endif
}

/**
 * This method exists mainly to access the data of the array easily from a
 * high-level language such as Python.
 *
 * This replaces the entire contents of the vector from a block of raw data
 * in a string (or bytes object, in Python 3).
 */
template<class Element>
INLINE void Extension<PointerToArray<Element> >::
set_data(PyObject *data) {
#if PY_VERSION_HEX >= 0x02060000
  if (PyObject_CheckBuffer(data)) {
    // User passed a buffer object.
    Py_buffer view;
    if (PyObject_GetBuffer(data, &view, PyBUF_CONTIG_RO) == -1) {
      PyErr_SetString(PyExc_TypeError,
                      "PointerToArray.set_data() requires a contiguous buffer");
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
    } else {
      this->_this->clear();
    }

    PyBuffer_Release(&view);
    return;
  }
#endif

  // In Python 2, there was also an older buffer protocol, supported by eg.
  // str and array objects.
#if PY_MAJOR_VERSION < 3
  // The old, deprecated buffer interface, as used by eg. the array module.
  const void *buffer;
  Py_ssize_t buffer_len;
  if (!PyUnicode_CheckExact(data) &&
      PyObject_AsReadBuffer(data, &buffer, &buffer_len) == 0) {
    if (buffer_len % sizeof(Element) != 0) {
      PyErr_Format(PyExc_ValueError,
                   "byte buffer is not a multiple of %zu bytes",
                   sizeof(Element));
      return;
    }

    if (buffer_len > 0) {
      this->_this->resize(buffer_len / sizeof(Element));
      memcpy(this->_this->p(), buffer, buffer_len);
    } else {
      this->_this->clear();
    }

    return;
  }
#endif

  Dtool_Raise_TypeError("PointerToArray.set_data() requires a buffer object");
}

/**
 * This returns the contents of a portion of the vector--from element (n)
 * through element (n + count - 1)--as a block of raw data in a string (or
 * bytes object, in Python 3).
 *
 * @deprecated use memoryview(pta) or bytearray(pta) instead.
 */
template<class Element>
INLINE PyObject *Extension<PointerToArray<Element> >::
get_subdata(size_t n, size_t count) const {
  n = std::min(n, this->_this->size());
  count = std::max(count, n);
  count = std::min(count, this->_this->size() - n);
#if PY_MAJOR_VERSION >= 3
  return PyBytes_FromStringAndSize((char *)(this->_this->p() + n), sizeof(Element) * count);
#else
  return PyString_FromStringAndSize((char *)(this->_this->p() + n), sizeof(Element) * count);
#endif
}

/**
 * Same as get_element(), this returns the nth element of the array.
 */
template<class Element>
INLINE const Element &Extension<ConstPointerToArray<Element> >::
__getitem__(size_t n) const {
  return (*this->_this)[n];
}

/**
 * This returns the entire contents of the vector as a block of raw data in a
 * string (or bytes object, in Python 3).
 *
 * @deprecated use memoryview(pta) or bytearray(pta) instead.
 */
template<class Element>
INLINE PyObject *Extension<ConstPointerToArray<Element> >::
get_data() const {
#if PY_MAJOR_VERSION >= 3
  return PyBytes_FromStringAndSize((char *)this->_this->p(), sizeof(Element) * this->_this->size());
#else
  return PyString_FromStringAndSize((char *)this->_this->p(), sizeof(Element) * this->_this->size());
#endif
}

/**
 * This returns the contents of a portion of the vector--from element (n)
 * through element (n + count - 1)--as a block of raw data in a string (or
 * bytes object, in Python 3).
 *
 * @deprecated use memoryview(pta) or bytearray(pta) instead.
 */
template<class Element>
INLINE PyObject *Extension<ConstPointerToArray<Element> >::
get_subdata(size_t n, size_t count) const {
  n = std::min(n, this->_this->size());
  count = std::max(count, n);
  count = std::min(count, this->_this->size() - n);
#if PY_MAJOR_VERSION >= 3
  return PyBytes_FromStringAndSize((char *)(this->_this->p() + n), sizeof(Element) * count);
#else
  return PyString_FromStringAndSize((char *)(this->_this->p() + n), sizeof(Element) * count);
#endif
}

/**
 * This is used to implement the buffer protocol, in order to allow efficient
 * access to the array data through a Python multiview object.
 */
template<class Element>
INLINE int Extension<PointerToArray<Element> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) {
#if PY_VERSION_HEX >= 0x02060000
  const char *format = get_format_code(Element);
  if (format == nullptr) {
    // Not supported.
    return -1;
  }

  if (self != nullptr) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void*) this->_this->p();
  view->len = this->_this->size() * sizeof(Element);
  view->readonly = 0;
  view->itemsize = sizeof(Element);
  view->format = nullptr;
  if ((flags & PyBUF_FORMAT) == PyBUF_FORMAT) {
    view->format = (char*) format;
  }
  view->ndim = 1;
  view->shape = nullptr;
  if ((flags & PyBUF_ND) == PyBUF_ND) {
    // This leaks, which sucks, but __releasebuffer__ doesn't give us the same
    // pointer, so we would need to store it elsewhere if we wanted to delete
    // it there.  Eh, it's just an int, who cares.
    view->shape = new Py_ssize_t(this->_this->size());
  }
  view->strides = nullptr;
  if ((flags & PyBUF_STRIDES) == PyBUF_STRIDES) {
    view->strides = &(view->itemsize);
  }
  view->suboffsets = nullptr;

  // Store a reference to ourselves on the Py_buffer object as a reminder that
  // we have increased our refcount.
  this->_this->ref();
  view->internal = (void*) this->_this;

  return 0;
#else
  return -1;
#endif
}

/**
 * This is used to implement the buffer protocol, in order to allow efficient
 * access to the array data through a Python memoryview object.
 */
template<>
INLINE int Extension<PointerToArray<LMatrix3f> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) {
#if PY_VERSION_HEX >= 0x02060000
  if (self != nullptr) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void*) this->_this->p();
  set_matrix_view(*view, flags, this->_this->size(), 3, false, false);

  // Store a reference to ourselves on the Py_buffer object as a reminder that
  // we have increased our refcount.
  this->_this->ref();
  view->internal = (void*) this->_this;

  return 0;
#else
  return -1;
#endif
}

/**
 * This is used to implement the buffer protocol, in order to allow efficient
 * access to the array data through a Python memoryview object.
 */
template<>
INLINE int Extension<PointerToArray<LMatrix3d> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) {
#if PY_VERSION_HEX >= 0x02060000
  if (self != nullptr) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void*) this->_this->p();
  set_matrix_view(*view, flags, this->_this->size(), 3, true, false);

  // Store a reference to ourselves on the Py_buffer object as a reminder that
  // we have increased our refcount.
  this->_this->ref();
  view->internal = (void*) this->_this;

  return 0;
#else
  return -1;
#endif
}

/**
 * This is used to implement the buffer protocol, in order to allow efficient
 * access to the array data through a Python memoryview object.
 */
template<>
INLINE int Extension<PointerToArray<UnalignedLMatrix4f> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) {
#if PY_VERSION_HEX >= 0x02060000
  if (self != nullptr) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void*) this->_this->p();
  set_matrix_view(*view, flags, this->_this->size(), 4, false, false);

  // Store a reference to ourselves on the Py_buffer object as a reminder that
  // we have increased our refcount.
  this->_this->ref();
  view->internal = (void*) this->_this;

  return 0;
#else
  return -1;
#endif
}

/**
 * This is used to implement the buffer protocol, in order to allow efficient
 * access to the array data through a Python memoryview object.
 */
template<>
INLINE int Extension<PointerToArray<UnalignedLMatrix4d> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) {
#if PY_VERSION_HEX >= 0x02060000
  if (self != nullptr) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void*) this->_this->p();
  set_matrix_view(*view, flags, this->_this->size(), 4, true, false);

  // Store a reference to ourselves on the Py_buffer object as a reminder that
  // we have increased our refcount.
  this->_this->ref();
  view->internal = (void*) this->_this;

  return 0;
#else
  return -1;
#endif
}

/**
 * Releases the buffer allocated by __getbuffer__.
 */
template<class Element>
INLINE void Extension<PointerToArray<Element> >::
__releasebuffer__(PyObject *self, Py_buffer *view) const {
#if PY_VERSION_HEX >= 0x02060000
  // Note: PyBuffer_Release automatically decrements view->obj.
  if (view->internal != nullptr) {
    // Oh, right, let's not forget to unref this.
    unref_delete((const PointerToArray<Element> *)view->internal);
    view->internal = nullptr;
  }
#endif
}

/**
 * This is used to implement the buffer protocol, in order to allow efficient
 * access to the array data through a Python multiview object.
 */
template<class Element>
INLINE int Extension<ConstPointerToArray<Element> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) const {
#if PY_VERSION_HEX >= 0x02060000
  if ((flags & PyBUF_WRITABLE) == PyBUF_WRITABLE) {
    PyErr_SetString(PyExc_BufferError,
                    "Object is not writable.");
    return -1;
  }

  const char *format = get_format_code(Element);
  if (format == nullptr) {
    // Not supported.
    return -1;
  }

  if (self != nullptr) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void*) this->_this->p();
  view->len = this->_this->size() * sizeof(Element);
  view->readonly = 1;
  view->itemsize = sizeof(Element);
  view->format = nullptr;
  if ((flags & PyBUF_FORMAT) == PyBUF_FORMAT) {
    view->format = (char*) format;
  }
  view->ndim = 1;
  view->shape = nullptr;
  if ((flags & PyBUF_ND) == PyBUF_ND) {
    // This leaks, which sucks, but __releasebuffer__ doesn't give us the same
    // pointer, so we would need to store it elsewhere if we wanted to delete
    // it there.  Eh, it's just an int, who cares.
    view->shape = new Py_ssize_t(this->_this->size());
  }
  view->strides = nullptr;
  if ((flags & PyBUF_STRIDES) == PyBUF_STRIDES) {
    view->strides = &(view->itemsize);
  }
  view->suboffsets = nullptr;

  // Store a reference to ourselves on the Py_buffer object as a reminder that
  // we have increased our refcount.
  this->_this->ref();
  view->internal = (void*) this->_this;

  return 0;
#else
  return -1;
#endif
}

/**
 * Specialization on __getbuffer__ for LMatrix3f.
 */
template<>
INLINE int Extension<ConstPointerToArray<LMatrix3f> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) const {
#if PY_VERSION_HEX >= 0x02060000
  if ((flags & PyBUF_WRITABLE) == PyBUF_WRITABLE) {
    PyErr_SetString(PyExc_BufferError,
                    "Object is not writable.");
    return -1;
  }
  if (self != nullptr) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void*) this->_this->p();
  set_matrix_view(*view, flags, this->_this->size(), 3, false, true);

  // Store a reference to ourselves on the Py_buffer object as a reminder that
  // we have increased our refcount.
  this->_this->ref();
  view->internal = (void*) this->_this;

  return 0;
#else
  return -1;
#endif
}

/**
 * Specialization on __getbuffer__ for LMatrix3d.
 */
template<>
INLINE int Extension<ConstPointerToArray<LMatrix3d> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) const {
#if PY_VERSION_HEX >= 0x02060000
  if ((flags & PyBUF_WRITABLE) == PyBUF_WRITABLE) {
    PyErr_SetString(PyExc_BufferError,
                    "Object is not writable.");
    return -1;
  }
  if (self != nullptr) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void*) this->_this->p();
  set_matrix_view(*view, flags, this->_this->size(), 3, true, true);

  // Store a reference to ourselves on the Py_buffer object as a reminder that
  // we have increased our refcount.
  this->_this->ref();
  view->internal = (void*) this->_this;

  return 0;
#else
  return -1;
#endif
}

/**
 * Specialization on __getbuffer__ for UnalignedLMatrix4f.
 */
template<>
INLINE int Extension<ConstPointerToArray<UnalignedLMatrix4f> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) const {
#if PY_VERSION_HEX >= 0x02060000
  if ((flags & PyBUF_WRITABLE) == PyBUF_WRITABLE) {
    PyErr_SetString(PyExc_BufferError,
                    "Object is not writable.");
    return -1;
  }
  if (self != nullptr) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void*) this->_this->p();
  set_matrix_view(*view, flags, this->_this->size(), 4, false, true);

  // Store a reference to ourselves on the Py_buffer object as a reminder that
  // we have increased our refcount.
  this->_this->ref();
  view->internal = (void*) this->_this;

  return 0;
#else
  return -1;
#endif
}

/**
 * Specialization on __getbuffer__ for UnalignedLMatrix4d.
 */
template<>
INLINE int Extension<ConstPointerToArray<UnalignedLMatrix4d> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) const {
#if PY_VERSION_HEX >= 0x02060000
  if ((flags & PyBUF_WRITABLE) == PyBUF_WRITABLE) {
    PyErr_SetString(PyExc_BufferError,
                    "Object is not writable.");
    return -1;
  }
  if (self != nullptr) {
    Py_INCREF(self);
  }
  view->obj = self;
  view->buf = (void*) this->_this->p();
  set_matrix_view(*view, flags, this->_this->size(), 4, true, true);

  // Store a reference to ourselves on the Py_buffer object as a reminder that
  // we have increased our refcount.
  this->_this->ref();
  view->internal = (void*) this->_this;

  return 0;
#else
  return -1;
#endif
}

/**
 * Releases the buffer allocated by __getbuffer__.
 */
template<class Element>
INLINE void Extension<ConstPointerToArray<Element> >::
__releasebuffer__(PyObject *self, Py_buffer *view) const {
#if PY_VERSION_HEX >= 0x02060000
  // Note: PyBuffer_Release automatically decrements obj->view.
  if (view->internal != nullptr) {
    // Oh, right, let's not forget to unref this.
    unref_delete((const PointerToArray<Element> *)view->internal);
    view->internal = nullptr;
  }
#endif
}
