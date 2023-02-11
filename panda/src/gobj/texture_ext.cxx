/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texture_ext.cxx
 * @author rdb
 * @date 2016-08-08
 */

#include "texture_ext.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern Dtool_PyTypedObject Dtool_PointerToArray_unsigned_char;
extern Dtool_PyTypedObject Dtool_ConstPointerToArray_unsigned_char;
#endif

/**
 * Replaces the current system-RAM image with the new data.  If compression is
 * not CM_off, it indicates that the new data is already pre-compressed in the
 * indicated format.
 *
 * This does *not* affect keep_ram_image.
 */
void Extension<Texture>::
set_ram_image(PyObject *image, Texture::CompressionMode compression,
              size_t page_size) {
  nassertv(compression != Texture::CM_default);

  // Check if perhaps a PointerToArray object was passed in.
  if (DtoolInstance_Check(image)) {
    if (DtoolInstance_TYPE(image) == &Dtool_ConstPointerToArray_unsigned_char) {
      _this->set_ram_image(*(const CPTA_uchar *)DtoolInstance_VOID_PTR(image), compression, page_size);
      return;
    } else if (DtoolInstance_TYPE(image) == &Dtool_PointerToArray_unsigned_char) {
      _this->set_ram_image(*(const PTA_uchar *)DtoolInstance_VOID_PTR(image), compression, page_size);
      return;
    }
  }

  if (PyObject_CheckBuffer(image)) {
    // User passed a buffer object.
    Py_buffer view;
    if (PyObject_GetBuffer(image, &view, PyBUF_CONTIG_RO) == -1) {
      PyErr_SetString(PyExc_TypeError,
                      "Texture.set_ram_image() requires a contiguous buffer");
      return;
    }

    int component_width = _this->get_component_width();
    if (compression == Texture::CM_off) {
      if (view.itemsize != 1 && view.itemsize != component_width) {
        PyErr_SetString(PyExc_TypeError,
                        "buffer.itemsize does not match Texture component size");
        return;
      }

      if (view.len % component_width != 0) {
        PyErr_Format(PyExc_ValueError,
                    "byte buffer is not a multiple of %d bytes",
                    component_width);
        return;
      }
    } else {
      if (view.itemsize != 1) {
        PyErr_SetString(PyExc_TypeError,
                        "buffer.itemsize should be 1 for compressed images");
        return;
      }
    }

    PTA_uchar data = PTA_uchar::empty_array(view.len, Texture::get_class_type());
    memcpy(data.p(), view.buf, view.len);
    _this->set_ram_image(std::move(data), compression, page_size);

    PyBuffer_Release(&view);
    return;
  }

  Dtool_Raise_ArgTypeError(image, 0, "Texture.set_ram_image", "CPTA_uchar or buffer");
}

/**
 * Replaces the current system-RAM image with the new data, converting it
 * first if necessary from the indicated component-order format.  See
 * get_ram_image_as() for specifications about the format.  This method cannot
 * support compressed image data or sub-pages; use set_ram_image() for that.
 */
void Extension<Texture>::
set_ram_image_as(PyObject *image, const std::string &provided_format) {
  // Check if perhaps a PointerToArray object was passed in.
  if (DtoolInstance_Check(image)) {
    if (DtoolInstance_TYPE(image) == &Dtool_ConstPointerToArray_unsigned_char) {
      _this->set_ram_image_as(*(const CPTA_uchar *)DtoolInstance_VOID_PTR(image), provided_format);
      return;
    } else if (DtoolInstance_TYPE(image) == &Dtool_PointerToArray_unsigned_char) {
      _this->set_ram_image_as(*(const PTA_uchar *)DtoolInstance_VOID_PTR(image), provided_format);
      return;
    }
  }

  if (PyObject_CheckBuffer(image)) {
    // User passed a buffer object.
    Py_buffer view;
    if (PyObject_GetBuffer(image, &view, PyBUF_CONTIG_RO) == -1) {
      PyErr_SetString(PyExc_TypeError,
                      "Texture.set_ram_image_as() requires a contiguous buffer");
      return;
    }

    int component_width = _this->get_component_width();
    if (view.itemsize != 1 && view.itemsize != component_width) {
      PyErr_SetString(PyExc_TypeError,
                      "buffer.itemsize does not match Texture component size");
      return;
    }

    if (view.len % component_width != 0) {
      PyErr_Format(PyExc_ValueError,
                  "byte buffer is not a multiple of %d bytes",
                  component_width);
      return;
    }

    PTA_uchar data = PTA_uchar::empty_array(view.len, Texture::get_class_type());
    memcpy(data.p(), view.buf, view.len);
    _this->set_ram_image_as(std::move(data), provided_format);

    PyBuffer_Release(&view);
    return;
  }

  Dtool_Raise_ArgTypeError(image, 0, "Texture.set_ram_image_as", "CPTA_uchar or buffer");
}

/**
 * A special Python method that is invoked by copy.deepcopy(tex).  This makes
 * sure that the copy has a unique copy of the RAM image.
 */
PT(Texture) Extension<Texture>::
__deepcopy__(PyObject *memo) const {
  PT(Texture) copy = _this->make_copy();
  {
    Texture::CDWriter cdata(copy->_cycler, true);
    for (Texture::RamImage &image : cdata->_ram_images) {
      if (image._image.get_ref_count() > 1) {
        PTA_uchar new_image;
        new_image.v() = image._image.v();
        image._image = std::move(new_image);
      }
    }
    {
      Texture::RamImage &image = cdata->_simple_ram_image;
      if (image._image.get_ref_count() > 1) {
        PTA_uchar new_image;
        new_image.v() = image._image.v();
        image._image = std::move(new_image);
      }
    }
  }
  return copy;
}

#endif  // HAVE_PYTHON
