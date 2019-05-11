/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexColumn.cxx
 * @author drose
 * @date 2005-03-06
 */

#include "geomVertexColumn.h"
#include "geomVertexData.h"
#include "bamReader.h"
#include "bamWriter.h"

using std::max;
using std::min;

/**
 *
 */
void GeomVertexColumn::
operator = (const GeomVertexColumn &copy) {
  _name = copy._name;
  _num_components = copy._num_components;
  _numeric_type = copy._numeric_type;
  _contents = copy._contents;
  _start = copy._start;
  _column_alignment = copy._column_alignment;
  _num_elements = copy._num_elements;
  _element_stride = copy._element_stride;

  setup();
}

/**
 * Replaces the name of an existing column.  This is only legal on an
 * unregistered format (i.e.  when constructing the format initially).
 */
void GeomVertexColumn::
set_name(InternalName *name) {
  _name = name;
  setup();
}

/**
 * Changes the number of components of an existing column.  This is only legal
 * on an unregistered format (i.e.  when constructing the format initially).
 */
void GeomVertexColumn::
set_num_components(int num_components) {
  _num_components = num_components;
  setup();
}

/**
 * Changes the numeric type an existing column.  This is only legal on an
 * unregistered format (i.e.  when constructing the format initially).
 */
void GeomVertexColumn::
set_numeric_type(NumericType numeric_type) {
  _numeric_type = numeric_type;
  setup();
}

/**
 * Changes the semantic meaning of an existing column.  This is only legal on
 * an unregistered format (i.e.  when constructing the format initially).
 */
void GeomVertexColumn::
set_contents(Contents contents) {
  _contents = contents;
  setup();
}

/**
 * Changes the start byte of an existing column.  This is only legal on an
 * unregistered format (i.e.  when constructing the format initially).
 */
void GeomVertexColumn::
set_start(int start) {
  _start = start;
  setup();
}

/**
 * Changes the column alignment of an existing column.  This is only legal on
 * an unregistered format (i.e.  when constructing the format initially).
 */
void GeomVertexColumn::
set_column_alignment(int column_alignment) {
  _column_alignment = column_alignment;
  setup();
}

/**
 *
 */
void GeomVertexColumn::
output(std::ostream &out) const {
  out << *get_name() << "(" << get_num_components();
  switch (get_numeric_type()) {
  case NT_uint8:
  case NT_int8:
    out << "b";
    break;

  case NT_uint16:
  case NT_int16:
    out << "s";
    break;

  case NT_uint32:
  case NT_int32:
    out << "l";
    break;

  case NT_packed_dcba:
    out << "p-";
    break;

  case NT_packed_dabc:
    out << "p";
    break;

  case NT_float32:
    out << "f";
    break;

  case NT_float64:
    out << "d";
    break;

  case NT_stdfloat:
  case NT_packed_ufloat:
    out << "?";
    break;
  }

  out << ")";

  if (_num_elements > 1) {
    out << "[" << _num_elements << "]";
  }
}

/**
 * Called once at construction time (or at bam-reading time) to initialize the
 * internal dependent values.
 */
void GeomVertexColumn::
setup() {
  nassertv(_num_components > 0 && _start >= 0);

  _num_values = _num_components;

  if (_numeric_type == NT_stdfloat) {
    if (vertices_float64) {
      _numeric_type = NT_float64;
    } else {
      _numeric_type = NT_float32;
    }
  }

  switch (_numeric_type) {
  case NT_uint16:
  case NT_int16:
    _component_bytes = 2;  // sizeof(uint16_t)
    break;

  case NT_uint32:
  case NT_int32:
    _component_bytes = 4;  // sizeof(uint32_t)
    break;

  case NT_uint8:
  case NT_int8:
    _component_bytes = 1;
    break;

  case NT_packed_dcba:
  case NT_packed_dabc:
    _component_bytes = 4;  // sizeof(uint32_t)
    _num_values *= 4;
    break;

  case NT_float32:
    _component_bytes = 4;  // sizeof(PN_float32)
    break;

  case NT_float64:
    _component_bytes = 8;  // sizeof(PN_float64)
    break;

  case NT_stdfloat:
    nassertv(false);
    break;

  case NT_packed_ufloat:
    _component_bytes = 4;  // sizeof(uint32_t)
    _num_values *= 3;
    break;
  }

  if (_num_elements == 0) {
    // Matrices are assumed to be square.
    if (_contents == C_matrix) {
      _num_elements = _num_components;
    } else {
      _num_elements = 1;
    }
  }

  if (_column_alignment < 1) {
    // The default column alignment is to align to the individual numeric
    // components, or to vertex_column_alignment, whichever is greater.
    _column_alignment = max(_component_bytes, (int)vertex_column_alignment);
  }

  // Enforce the column alignment requirements on the _start byte.
  _start = ((_start + _column_alignment - 1) / _column_alignment) * _column_alignment;

  if (_element_stride < 1) {
    _element_stride = _component_bytes * _num_components;
  }
  _total_bytes = _element_stride * _num_elements;

  if (_packer != nullptr) {
    delete _packer;
  }

  _packer = make_packer();
  _packer->_column = this;
}

/**
 * Returns a newly-allocated Packer object suitable for packing and unpacking
 * this column.  The _column member of the packer is not filled in.
 */
GeomVertexColumn::Packer *GeomVertexColumn::
make_packer() const {
  switch (get_contents()) {
  case C_point:
  case C_clip_point:
  case C_texcoord:
    // These types are read as a 4-d homogeneous point.
    switch (get_numeric_type()) {
    case NT_float32:
      if (sizeof(float) == sizeof(PN_float32)) {
        // Use the native float type implementation for a tiny bit more
        // optimization.
        switch (get_num_components()) {
        case 2:
          return new Packer_point_nativefloat_2;
        case 3:
          return new Packer_point_nativefloat_3;
        case 4:
          // Reader may assume 16-byte alignment.
          if ((get_start() & 0xf) == 0) {
            return new Packer_point_nativefloat_4;
          } else {
            return new Packer_point_float32_4;
          }
        }
      } else {
        switch (get_num_components()) {
        case 2:
          return new Packer_point_float32_2;
        case 3:
          return new Packer_point_float32_3;
        case 4:
          return new Packer_point_float32_4;
        }
      }
      break;
    case NT_float64:
      if (sizeof(double) == sizeof(PN_float64)) {
        // Use the native float type implementation for a tiny bit more
        // optimization.
        switch (get_num_components()) {
        case 2:
          return new Packer_point_nativedouble_2;
        case 3:
          return new Packer_point_nativedouble_3;
        case 4:
          return new Packer_point_nativedouble_4;
        }
      } else {
        switch (get_num_components()) {
        case 2:
          return new Packer_point_float64_2;
        case 3:
          return new Packer_point_float64_3;
        case 4:
          return new Packer_point_float64_4;
        }
      }
      break;
    default:
      break;
    }
    return new Packer_point;

  case C_color:
    if (get_num_values() == 4) {
      switch (get_numeric_type()) {
      case NT_uint8:
        return new Packer_rgba_uint8_4;

      case NT_packed_dabc:
        return new Packer_argb_packed;

      case NT_float32:
        if (sizeof(float) == sizeof(PN_float32) &&
            (get_start() & 0xf) == 0) {
          // Use the native float type implementation for a tiny bit more
          // optimization.
          return new Packer_rgba_nativefloat_4;
        } else {
          return new Packer_rgba_float32_4;
        }

      default:
        break;
      }
    } else if (get_num_values() != 3) {
      gobj_cat.error()
        << "GeomVertexColumn with contents C_color must have 3 or 4 components!\n";
    }
    return new Packer_color;

  case C_normal:
    if (get_num_values() != 3 && get_num_values() != 4) {
      gobj_cat.error()
        << "GeomVertexColumn with contents C_normal must have 3 or 4 components!\n";
    }

  default:
    // Otherwise, we just read it as a generic value.
    switch (get_numeric_type()) {
    case NT_float32:
      switch (get_num_components()) {
      case 3:
        if (sizeof(float) == sizeof(PN_float32)) {
          // Use the native float type implementation for a tiny bit more
          // optimization.
          return new Packer_nativefloat_3;
        } else {
          return new Packer_float32_3;
        }

      default:
        break;
      }
      break;
    case NT_float64:
      switch (get_num_components()) {
      case 3:
        if (sizeof(double) == sizeof(PN_float64)) {
          // Use the native float type implementation for a tiny bit more
          // optimization.
          return new Packer_nativedouble_3;
        } else {
          return new Packer_float64_3;
        }

      default:
        break;
      }
      break;
    default:
      break;
    }
    return new Packer;
  }
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void GeomVertexColumn::
write_datagram(BamWriter *manager, Datagram &dg) {
  manager->write_pointer(dg, _name);
  dg.add_uint8(_num_components);
  dg.add_uint8(_numeric_type);

  if (_contents == C_normal && manager->get_file_minor_ver() < 38) {
    // Panda 1.9 did not have C_normal.
    dg.add_uint8(C_vector);
  } else {
    dg.add_uint8(_contents);
  }

  dg.add_uint16(_start);

  if (manager->get_file_minor_ver() >= 29) {
    dg.add_uint8(_column_alignment);
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int GeomVertexColumn::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = 0;

  _name = DCAST(InternalName, p_list[pi++]);

  // Make sure that old .bam files are corrected to have C_normal normal
  // columns rather than C_vector.
  if (manager->get_file_minor_ver() < 38 &&
      _name == InternalName::get_normal() && _contents == C_vector) {
    _contents = C_normal;
  }

  return pi;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new GeomVertexColumn.
 */
void GeomVertexColumn::
fillin(DatagramIterator &scan, BamReader *manager) {
  manager->read_pointer(scan);

  _num_components = scan.get_uint8();
  _numeric_type = (NumericType)scan.get_uint8();
  _contents = (Contents)scan.get_uint8();
  _start = scan.get_uint16();

  _column_alignment = 1;
  if (manager->get_file_minor_ver() >= 29) {
    _column_alignment = scan.get_uint8();
  }

  _num_elements = 0;
  _element_stride = 0;

  setup();
}

/**
 *
 */
GeomVertexColumn::Packer::
~Packer() {
}

/**
 *
 */
float GeomVertexColumn::Packer::
get_data1f(const unsigned char *pointer) {
  switch (_column->get_numeric_type()) {
  case NT_uint8:
    return *(const uint8_t *)pointer;

  case NT_uint16:
    return *(const uint16_t *)pointer;

  case NT_uint32:
    return *(const uint32_t *)pointer;

  case NT_packed_dcba:
    {
      uint32_t dword = *(const uint32_t *)pointer;
      return GeomVertexData::unpack_abcd_d(dword);
    }

  case NT_packed_dabc:
    {
      uint32_t dword = *(const uint32_t *)pointer;
      return GeomVertexData::unpack_abcd_b(dword);
    }

  case NT_float32:
    return *(const PN_float32 *)pointer;

  case NT_float64:
    return *(const PN_float64 *)pointer;

  case NT_int8:
    return *(const int8_t *)pointer;

  case NT_int16:
    return *(const int16_t *)pointer;

  case NT_int32:
    return *(const int32_t *)pointer;

  case NT_packed_ufloat:
    {
      uint32_t dword = *(const uint32_t *)pointer;
      return GeomVertexData::unpack_ufloat_a(dword);
    }

  default:
    nassertr(false, 0.0f);
  }

  return 0.0f;
}

/**
 *
 */
const LVecBase2f &GeomVertexColumn::Packer::
get_data2f(const unsigned char *pointer) {
  if (_column->get_num_values() == 1) {
    _v2.set(get_data1f(pointer), 0.0f);
    return _v2;

  } else {
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        const uint8_t *pi = (const uint8_t *)pointer;
        _v2.set(pi[0], pi[1]);
      }
      return _v2;

    case NT_uint16:
      {
        const uint16_t *pi = (const uint16_t *)pointer;
        _v2.set(pi[0], pi[1]);
      }
      return _v2;

    case NT_uint32:
      {
        const uint32_t *pi = (const uint32_t *)pointer;
        _v2.set(pi[0], pi[1]);
      }
      return _v2;

    case NT_packed_dcba:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v2.set(GeomVertexData::unpack_abcd_d(dword),
                GeomVertexData::unpack_abcd_c(dword));
      }
      return _v2;

    case NT_packed_dabc:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v2.set(GeomVertexData::unpack_abcd_b(dword),
                GeomVertexData::unpack_abcd_c(dword));
      }
      return _v2;

    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v2.set(pi[0], pi[1]);
      }
      return _v2;

    case NT_float64:
      {
        const PN_float64 *pi = (const PN_float64 *)pointer;
        _v2.set(pi[0], pi[1]);
      }
      return _v2;

    case NT_stdfloat:
      nassertr(false, _v2);
      return _v2;

    case NT_int8:
      {
        const int8_t *pi = (const int8_t *)pointer;
        _v2.set(pi[0], pi[1]);
      }
      return _v2;

    case NT_int16:
      {
        const int16_t *pi = (const int16_t *)pointer;
        _v2.set(pi[0], pi[1]);
      }
      return _v2;

    case NT_int32:
      {
        const int32_t *pi = (const int32_t *)pointer;
        _v2.set(pi[0], pi[1]);
      }
      return _v2;

    case NT_packed_ufloat:
      nassertr(false, _v2);
      return _v2;
    }
  }

  return _v2;
}

/**
 *
 */
const LVecBase3f &GeomVertexColumn::Packer::
get_data3f(const unsigned char *pointer) {
  switch (_column->get_num_values()) {
  case 1:
    _v3.set(get_data1f(pointer), 0.0f, 0.0f);
    return _v3;

  case 2:
    {
      const LVecBase2f &v2 = get_data2f(pointer);
      _v3.set(v2[0], v2[1], 0.0f);
    }
    return _v3;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        const uint8_t *pi = (const uint8_t *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
      }
      return _v3;

    case NT_uint16:
      {
        const uint16_t *pi = (const uint16_t *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
      }
      return _v3;

    case NT_uint32:
      {
        const uint32_t *pi = (const uint32_t *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
      }
      return _v3;

    case NT_packed_dcba:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v3.set(GeomVertexData::unpack_abcd_d(dword),
                GeomVertexData::unpack_abcd_c(dword),
                GeomVertexData::unpack_abcd_b(dword));
      }
      return _v3;

    case NT_packed_dabc:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v3.set(GeomVertexData::unpack_abcd_b(dword),
                GeomVertexData::unpack_abcd_c(dword),
                GeomVertexData::unpack_abcd_d(dword));
      }
      return _v3;

    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
      }
      return _v3;

    case NT_float64:
      {
        const PN_float64 *pi = (const PN_float64 *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
      }
      return _v3;

    case NT_stdfloat:
      nassertr(false, _v3);
      return _v3;

    case NT_int8:
      {
        const int8_t *pi = (const int8_t *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
      }
      return _v3;

    case NT_int16:
      {
        const int16_t *pi = (const int16_t *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
      }
      return _v3;

    case NT_int32:
      {
        const int32_t *pi = (const int32_t *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
      }
      return _v3;

    case NT_packed_ufloat:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v3.set(GeomVertexData::unpack_ufloat_a(dword),
                GeomVertexData::unpack_ufloat_b(dword),
                GeomVertexData::unpack_ufloat_c(dword));
      }
      return _v3;
    }
  }

  return _v3;
}

/**
 *
 */
const LVecBase4f &GeomVertexColumn::Packer::
get_data4f(const unsigned char *pointer) {
  switch (_column->get_num_values()) {
  case 1:
    _v4.set(get_data1f(pointer), 0.0f, 0.0f, 0.0f);
    return _v4;

  case 2:
    {
      const LVecBase2f &v2 = get_data2f(pointer);
      _v4.set(v2[0], v2[1], 0.0f, 0.0f);
    }
    return _v4;

  case 3:
    {
      const LVecBase3f &v3 = get_data3f(pointer);
      _v4.set(v3[0], v3[1], v3[2], 0.0f);
    }
    return _v4;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        const uint8_t *pi = (const uint8_t *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_uint16:
      {
        const uint16_t *pi = (const uint16_t *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_uint32:
      {
        const uint32_t *pi = (const uint32_t *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_packed_dcba:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v4.set(GeomVertexData::unpack_abcd_d(dword),
                GeomVertexData::unpack_abcd_c(dword),
                GeomVertexData::unpack_abcd_b(dword),
                GeomVertexData::unpack_abcd_a(dword));
      }
      return _v4;

    case NT_packed_dabc:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v4.set(GeomVertexData::unpack_abcd_b(dword),
                GeomVertexData::unpack_abcd_c(dword),
                GeomVertexData::unpack_abcd_d(dword),
                GeomVertexData::unpack_abcd_a(dword));
      }
      return _v4;

    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_float64:
      {
        const PN_float64 *pi = (const PN_float64 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_stdfloat:
      nassertr(false, _v4);
      break;

    case NT_int8:
      {
        const int8_t *pi = (const int8_t *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_int16:
      {
        const int16_t *pi = (const int16_t *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_int32:
      {
        const int32_t *pi = (const int32_t *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_packed_ufloat:
      nassertr(false, _v4);
      break;
    }
  }

  return _v4;
}

/**
 *
 */
double GeomVertexColumn::Packer::
get_data1d(const unsigned char *pointer) {
  switch (_column->get_numeric_type()) {
  case NT_uint8:
    return *(const uint8_t *)pointer;

  case NT_uint16:
    return *(const uint16_t *)pointer;

  case NT_uint32:
    return *(const uint32_t *)pointer;

  case NT_packed_dcba:
    {
      uint32_t dword = *(const uint32_t *)pointer;
      return GeomVertexData::unpack_abcd_d(dword);
    }

  case NT_packed_dabc:
    {
      uint32_t dword = *(const uint32_t *)pointer;
      return GeomVertexData::unpack_abcd_b(dword);
    }

  case NT_float32:
    return *(const PN_float32 *)pointer;

  case NT_float64:
    return *(const PN_float64 *)pointer;

  case NT_stdfloat:
    nassertr(false, 0.0);
    return 0.0;

  case NT_int8:
    return *(const int8_t *)pointer;

  case NT_int16:
    return *(const int16_t *)pointer;

  case NT_int32:
    return *(const int32_t *)pointer;

  case NT_packed_ufloat:
    {
      uint32_t dword = *(const uint32_t *)pointer;
      return GeomVertexData::unpack_ufloat_a(dword);
    }
  }

  return 0.0;
}

/**
 *
 */
const LVecBase2d &GeomVertexColumn::Packer::
get_data2d(const unsigned char *pointer) {
  if (_column->get_num_values() == 1) {
    _v2d.set(get_data1d(pointer), 0.0f);
    return _v2d;

  } else {
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        const uint8_t *pi = (const uint8_t *)pointer;
        _v2d.set(pi[0], pi[1]);
      }
      return _v2d;

    case NT_uint16:
      {
        const uint16_t *pi = (const uint16_t *)pointer;
        _v2d.set(pi[0], pi[1]);
      }
      return _v2d;

    case NT_uint32:
      {
        const uint32_t *pi = (const uint32_t *)pointer;
        _v2d.set(pi[0], pi[1]);
      }
      return _v2d;

    case NT_packed_dcba:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v2d.set(GeomVertexData::unpack_abcd_d(dword),
                 GeomVertexData::unpack_abcd_c(dword));
      }
      return _v2d;

    case NT_packed_dabc:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v2d.set(GeomVertexData::unpack_abcd_b(dword),
                 GeomVertexData::unpack_abcd_c(dword));
      }
      return _v2d;

    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v2d.set(pi[0], pi[1]);
      }
      return _v2d;

    case NT_float64:
      {
        const PN_float64 *pi = (const PN_float64 *)pointer;
        _v2d.set(pi[0], pi[1]);
      }
      return _v2d;

    case NT_stdfloat:
      nassertr(false, _v2d);
      return _v2d;

    case NT_int8:
      {
        const int8_t *pi = (const int8_t *)pointer;
        _v2d.set(pi[0], pi[1]);
      }
      return _v2d;

    case NT_int16:
      {
        const int16_t *pi = (const int16_t *)pointer;
        _v2d.set(pi[0], pi[1]);
      }
      return _v2d;

    case NT_int32:
      {
        const int32_t *pi = (const int32_t *)pointer;
        _v2d.set(pi[0], pi[1]);
      }
      return _v2d;

    case NT_packed_ufloat:
      nassertr(false, _v2d);
      break;
    }
  }

  return _v2d;
}

/**
 *
 */
const LVecBase3d &GeomVertexColumn::Packer::
get_data3d(const unsigned char *pointer) {
  switch (_column->get_num_values()) {
  case 1:
    _v3d.set(get_data1d(pointer), 0.0f, 0.0f);
    return _v3d;

  case 2:
    {
      const LVecBase2d &v2 = get_data2d(pointer);
      _v3d.set(v2[0], v2[1], 0.0f);
    }
    return _v3d;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        const uint8_t *pi = (const uint8_t *)pointer;
        _v3d.set(pi[0], pi[1], pi[2]);
      }
      return _v3d;

    case NT_uint16:
      {
        const uint16_t *pi = (const uint16_t *)pointer;
        _v3d.set(pi[0], pi[1], pi[2]);
      }
      return _v3d;

    case NT_uint32:
      {
        const uint32_t *pi = (const uint32_t *)pointer;
        _v3d.set(pi[0], pi[1], pi[2]);
      }
      return _v3d;

    case NT_packed_dcba:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v3d.set(GeomVertexData::unpack_abcd_d(dword),
                 GeomVertexData::unpack_abcd_c(dword),
                 GeomVertexData::unpack_abcd_b(dword));
      }
      return _v3d;

    case NT_packed_dabc:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v3d.set(GeomVertexData::unpack_abcd_b(dword),
                 GeomVertexData::unpack_abcd_c(dword),
                 GeomVertexData::unpack_abcd_d(dword));
      }
      return _v3d;

    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v3d.set(pi[0], pi[1], pi[2]);
      }
      return _v3d;

    case NT_float64:
      {
        const PN_float64 *pi = (const PN_float64 *)pointer;
        _v3d.set(pi[0], pi[1], pi[2]);
      }
      return _v3d;

    case NT_stdfloat:
      nassertr(false, _v3d);
      return _v3d;

    case NT_int8:
      {
        const int8_t *pi = (const int8_t *)pointer;
        _v3d.set(pi[0], pi[1], pi[2]);
      }
      return _v3d;

    case NT_int16:
      {
        const int16_t *pi = (const int16_t *)pointer;
        _v3d.set(pi[0], pi[1], pi[2]);
      }
      return _v3d;

    case NT_int32:
      {
        const int32_t *pi = (const int32_t *)pointer;
        _v3d.set(pi[0], pi[1], pi[2]);
      }
      return _v3d;

    case NT_packed_ufloat:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v3d.set(GeomVertexData::unpack_ufloat_a(dword),
                 GeomVertexData::unpack_ufloat_b(dword),
                 GeomVertexData::unpack_ufloat_c(dword));
      }
      return _v3d;
    }
  }

  return _v3d;
}

/**
 *
 */
const LVecBase4d &GeomVertexColumn::Packer::
get_data4d(const unsigned char *pointer) {
  switch (_column->get_num_values()) {
  case 1:
    _v4d.set(get_data1d(pointer), 0.0f, 0.0f, 0.0f);
    return _v4d;

  case 2:
    {
      const LVecBase2d &v2 = get_data2d(pointer);
      _v4d.set(v2[0], v2[1], 0.0f, 0.0f);
    }
    return _v4d;

  case 3:
    {
      const LVecBase3d &v3 = get_data3d(pointer);
      _v4d.set(v3[0], v3[1], v3[2], 0.0f);
    }
    return _v4d;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        const uint8_t *pi = (const uint8_t *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_uint16:
      {
        const uint16_t *pi = (const uint16_t *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_uint32:
      {
        const uint32_t *pi = (const uint32_t *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_packed_dcba:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v4d.set(GeomVertexData::unpack_abcd_d(dword),
                 GeomVertexData::unpack_abcd_c(dword),
                 GeomVertexData::unpack_abcd_b(dword),
                 GeomVertexData::unpack_abcd_a(dword));
      }
      return _v4d;

    case NT_packed_dabc:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v4d.set(GeomVertexData::unpack_abcd_b(dword),
                 GeomVertexData::unpack_abcd_c(dword),
                 GeomVertexData::unpack_abcd_d(dword),
                 GeomVertexData::unpack_abcd_a(dword));
      }
      return _v4d;

    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_float64:
      {
        const PN_float64 *pi = (const PN_float64 *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_stdfloat:
      nassertr(false, _v4d);
      break;

    case NT_int8:
      {
        const int8_t *pi = (const int8_t *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_int16:
      {
        const int16_t *pi = (const int16_t *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_int32:
      {
        const int32_t *pi = (const int32_t *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_packed_ufloat:
      nassertr(false, _v4d);
      break;
    }
  }

  return _v4d;
}

/**
 *
 */
int GeomVertexColumn::Packer::
get_data1i(const unsigned char *pointer) {
  switch (_column->get_numeric_type()) {
  case NT_uint8:
    return *pointer;

  case NT_uint16:
    return *(const uint16_t *)pointer;

  case NT_uint32:
    return *(const uint32_t *)pointer;

  case NT_packed_dcba:
    {
      uint32_t dword = *(const uint32_t *)pointer;
      return GeomVertexData::unpack_abcd_d(dword);
    }

  case NT_packed_dabc:
    {
      uint32_t dword = *(const uint32_t *)pointer;
      return GeomVertexData::unpack_abcd_b(dword);
    }

  case NT_float32:
    return (int)*(const PN_float32 *)pointer;

  case NT_float64:
    return (int)*(const PN_float64 *)pointer;

  case NT_stdfloat:
    nassertr(false, 0);
    break;

  case NT_int8:
    return *(const int8_t *)pointer;

  case NT_int16:
    return *(const int16_t *)pointer;

  case NT_int32:
    return *(const int32_t *)pointer;

  case NT_packed_ufloat:
    {
      uint32_t dword = *(const uint32_t *)pointer;
      return (int)GeomVertexData::unpack_ufloat_a(dword);
    }
  }

  return 0;
}

/**
 *
 */
const LVecBase2i &GeomVertexColumn::Packer::
get_data2i(const unsigned char *pointer) {
  switch (_column->get_num_values()) {
  case 1:
    _v2i.set(get_data1i(pointer), 0);
    return _v2i;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      _v2i.set(pointer[0], pointer[1]);
      return _v2i;

    case NT_uint16:
      {
        const uint16_t *pi = (const uint16_t *)pointer;
        _v2i.set(pi[0], pi[1]);
      }
      return _v2i;

    case NT_uint32:
      {
        const uint32_t *pi = (const uint32_t *)pointer;
        _v2i.set(pi[0], pi[1]);
      }
      return _v2i;

    case NT_packed_dcba:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v2i.set(GeomVertexData::unpack_abcd_d(dword),
                 GeomVertexData::unpack_abcd_c(dword));
      }
      return _v2i;

    case NT_packed_dabc:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v2i.set(GeomVertexData::unpack_abcd_b(dword),
                 GeomVertexData::unpack_abcd_c(dword));
      }
      return _v2i;

    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v2i.set((int)pi[0], (int)pi[1]);
      }
      return _v2i;

    case NT_float64:
      {
        const PN_float64 *pi = (const PN_float64 *)pointer;
        _v2i.set((int)pi[0], (int)pi[1]);
      }
      return _v2i;

    case NT_stdfloat:
      nassertr(false, _v2i);
      break;

    case NT_int8:
      {
        const int8_t *pi = (const int8_t *)pointer;
        _v2i.set(pi[0], pi[1]);
      }
      return _v2i;

    case NT_int16:
      {
        const int16_t *pi = (const int16_t *)pointer;
        _v2i.set(pi[0], pi[1]);
      }
      return _v2i;

    case NT_int32:
      {
        const int32_t *pi = (const int32_t *)pointer;
        _v2i.set(pi[0], pi[1]);
      }
      return _v2i;

    case NT_packed_ufloat:
      nassertr(false, _v2i);
      break;
    }
  }

  return _v2i;
}

/**
 *
 */
const LVecBase3i &GeomVertexColumn::Packer::
get_data3i(const unsigned char *pointer) {
  switch (_column->get_num_values()) {
  case 1:
    _v3i.set(get_data1i(pointer), 0, 0);
    return _v3i;

  case 2:
    {
      const LVecBase2i &v2 = get_data2i(pointer);
      _v3i.set(v2[0], v2[1], 0);
    }
    return _v3i;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      _v3i.set(pointer[0], pointer[1], 0);
      return _v3i;

    case NT_uint16:
      {
        const uint16_t *pi = (const uint16_t *)pointer;
        _v3i.set(pi[0], pi[1], pi[2]);
      }
      return _v3i;

    case NT_uint32:
      {
        const uint32_t *pi = (const uint32_t *)pointer;
        _v3i.set(pi[0], pi[1], pi[2]);
      }
      return _v3i;

    case NT_packed_dcba:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v3i.set(GeomVertexData::unpack_abcd_d(dword),
                 GeomVertexData::unpack_abcd_c(dword),
                 GeomVertexData::unpack_abcd_b(dword));
      }
      return _v3i;

    case NT_packed_dabc:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v3i.set(GeomVertexData::unpack_abcd_b(dword),
                 GeomVertexData::unpack_abcd_c(dword),
                 GeomVertexData::unpack_abcd_d(dword));
      }
      return _v3i;

    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v3i.set((int)pi[0], (int)pi[1], (int)pi[2]);
      }
      return _v3i;

    case NT_float64:
      {
        const PN_float64 *pi = (const PN_float64 *)pointer;
        _v3i.set((int)pi[0], (int)pi[1], (int)pi[2]);
      }
      return _v3i;

    case NT_stdfloat:
      nassertr(false, _v3i);
      break;

    case NT_int8:
      {
        const int8_t *pi = (const int8_t *)pointer;
        _v3i.set(pi[0], pi[1], pi[2]);
      }
      return _v3i;

    case NT_int16:
      {
        const int16_t *pi = (const int16_t *)pointer;
        _v3i.set(pi[0], pi[1], pi[2]);
      }
      return _v3i;

    case NT_int32:
      {
        const int32_t *pi = (const int32_t *)pointer;
        _v3i.set(pi[0], pi[1], pi[2]);
      }
      return _v3i;

    case NT_packed_ufloat:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v3i.set((int)GeomVertexData::unpack_ufloat_a(dword),
                 (int)GeomVertexData::unpack_ufloat_b(dword),
                 (int)GeomVertexData::unpack_ufloat_c(dword));
      }
      return _v3i;
    }
  }

  return _v3i;
}

/**
 *
 */
const LVecBase4i &GeomVertexColumn::Packer::
get_data4i(const unsigned char *pointer) {
  switch (_column->get_num_values()) {
  case 1:
    _v4i.set(get_data1i(pointer), 0, 0, 0);
    return _v4i;

  case 2:
    {
      const LVecBase2i &v2 = get_data2i(pointer);
      _v4i.set(v2[0], v2[1], 0, 0);
    }
    return _v4i;

  case 3:
    {
      const LVecBase3i &v3 = get_data3i(pointer);
      _v4i.set(v3[0], v3[1], v3[2], 0);
    }
    return _v4i;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      _v4i.set(pointer[0], pointer[1], pointer[2], pointer[3]);
      return _v4i;

    case NT_uint16:
      {
        const uint16_t *pi = (const uint16_t *)pointer;
        _v4i.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4i;

    case NT_uint32:
      {
        const uint32_t *pi = (const uint32_t *)pointer;
        _v4i.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4i;

    case NT_packed_dcba:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v4i.set(GeomVertexData::unpack_abcd_d(dword),
                 GeomVertexData::unpack_abcd_c(dword),
                 GeomVertexData::unpack_abcd_b(dword),
                 GeomVertexData::unpack_abcd_a(dword));
      }
      return _v4i;

    case NT_packed_dabc:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v4i.set(GeomVertexData::unpack_abcd_b(dword),
                 GeomVertexData::unpack_abcd_c(dword),
                 GeomVertexData::unpack_abcd_d(dword),
                 GeomVertexData::unpack_abcd_a(dword));
      }
      return _v4i;

    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v4i.set((int)pi[0], (int)pi[1], (int)pi[2], (int)pi[3]);
      }
      return _v4i;

    case NT_float64:
      {
        const PN_float64 *pi = (const PN_float64 *)pointer;
        _v4i.set((int)pi[0], (int)pi[1], (int)pi[2], (int)pi[3]);
      }
      return _v4i;

    case NT_stdfloat:
      nassertr(false, _v4i);
      break;

    case NT_int8:
      {
        const int8_t *pi = (const int8_t *)pointer;
        _v4i.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4i;

    case NT_int16:
      {
        const int16_t *pi = (const int16_t *)pointer;
        _v4i.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4i;

    case NT_int32:
      {
        const int32_t *pi = (const int32_t *)pointer;
        _v4i.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4i;

    case NT_packed_ufloat:
      nassertr(false, _v4i);
      break;
    }
  }

  return _v4i;
}

/**
 *
 */
void GeomVertexColumn::Packer::
set_data1f(unsigned char *pointer, float data) {
  switch (_column->get_num_values()) {
  case 1:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      *(uint8_t *)pointer = (unsigned int)data;
      break;

    case NT_uint16:
      *(uint16_t *)pointer = (unsigned int)data;
      break;

    case NT_uint32:
      *(uint32_t *)pointer = (unsigned int)data;
      break;

    case NT_packed_dcba:
    case NT_packed_dabc:
      nassertv(false);
      break;

    case NT_float32:
      *(PN_float32 *)pointer = data;
      break;

    case NT_float64:
      *(PN_float64 *)pointer = data;
      break;

    case NT_stdfloat:
      nassertv(false);
      break;

    case NT_int8:
      *(int8_t *)pointer = (int)data;
      break;

    case NT_int16:
      *(int16_t *)pointer = (int)data;
      break;

    case NT_int32:
      *(int32_t *)pointer = (int)data;
      break;

    case NT_packed_ufloat:
      nassertv(false);
      break;
    }
    break;

  case 2:
    set_data2f(pointer, LVecBase2f(data, 0.0f));
    break;

  case 3:
    set_data3f(pointer, LVecBase3f(data, 0.0f, 0.0f));
    break;

  case 4:
    set_data4f(pointer, LVecBase4f(data, 0.0f, 0.0f, 0.0f));
    break;
  }
}

/**
 *
 */
void GeomVertexColumn::Packer::
set_data2f(unsigned char *pointer, const LVecBase2f &data) {
  switch (_column->get_num_values()) {
  case 1:
    set_data1f(pointer, data[0]);
    break;

  case 2:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        uint8_t *pi = (uint8_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
      }
      break;

    case NT_uint16:
      {
        uint16_t *pi = (uint16_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
      }
      break;

    case NT_uint32:
      {
        uint32_t *pi = (uint32_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
      }
      break;

    case NT_packed_dcba:
    case NT_packed_dabc:
      nassertv(false);
      break;

    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
      }
      break;

    case NT_float64:
      {
        PN_float64 *pi = (PN_float64 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
      }
      break;

    case NT_stdfloat:
      nassertv(false);
      break;

    case NT_int8:
      {
        int8_t *pi = (int8_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
      }
      break;

    case NT_int16:
      {
        int16_t *pi = (int16_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
      }
      break;

    case NT_int32:
      {
        int32_t *pi = (int32_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
      }
      break;

    case NT_packed_ufloat:
      nassertv(false);
      break;
    }
    break;

  case 3:
    set_data3f(pointer, LVecBase3f(data[0], data[1], 0.0f));
    break;

  default:
    set_data4f(pointer, LVecBase4f(data[0], data[1], 0.0f, 0.0f));
    break;
  }
}

/**
 *
 */
void GeomVertexColumn::Packer::
set_data3f(unsigned char *pointer, const LVecBase3f &data) {
  switch (_column->get_num_values()) {
  case 1:
    set_data1f(pointer, data[0]);
    break;

  case 2:
    set_data2f(pointer, LVecBase2f(data[0], data[1]));
    break;

  case 3:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        uint8_t *pi = (uint8_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
      }
      break;

    case NT_uint16:
      {
        uint16_t *pi = (uint16_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
      }
      break;

    case NT_uint32:
      {
        uint32_t *pi = (uint32_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
      }
      break;

    case NT_packed_dcba:
    case NT_packed_dabc:
      nassertv(false);
      break;

    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
      }
      break;

    case NT_float64:
      {
        PN_float64 *pi = (PN_float64 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
      }
      break;

    case NT_stdfloat:
      nassertv(false);
      break;

    case NT_int8:
      {
        int8_t *pi = (int8_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
      }
      break;

    case NT_int16:
      {
        int16_t *pi = (int16_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
      }
      break;

    case NT_int32:
      {
        int32_t *pi = (int32_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
      }
      break;

    case NT_packed_ufloat:
      *(uint32_t *)pointer = GeomVertexData::pack_ufloat(data[0], data[1], data[2]);
      break;
    }
    break;

  default:
    set_data4f(pointer, LVecBase4f(data[0], data[1], data[2], 0.0f));
    break;
  }
}

/**
 *
 */
void GeomVertexColumn::Packer::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  switch (_column->get_num_values()) {
  case 1:
    set_data1f(pointer, data[0]);
    break;

  case 2:
    set_data2f(pointer, LVecBase2f(data[0], data[1]));
    break;

  case 3:
    set_data3f(pointer, LVecBase3f(data[0], data[1], data[2]));
    break;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        uint8_t *pi = (uint8_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;

    case NT_uint16:
      {
        uint16_t *pi = (uint16_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;

    case NT_uint32:
      {
        uint32_t *pi = (uint32_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;

    case NT_packed_dcba:
      *(uint32_t *)pointer = GeomVertexData::pack_abcd(data[3], data[2], data[1], data[0]);
      break;

    case NT_packed_dabc:
      *(uint32_t *)pointer = GeomVertexData::pack_abcd(data[3], data[0], data[1], data[2]);
      break;

    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_float64:
      {
        PN_float64 *pi = (PN_float64 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_stdfloat:
      nassertv(false);
      break;

    case NT_int8:
      {
        int8_t *pi = (int8_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
        pi[3] = (int)data[3];
      }
      break;

    case NT_int16:
      {
        int16_t *pi = (int16_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
        pi[3] = (int)data[3];
      }
      break;

    case NT_int32:
      {
        int32_t *pi = (int32_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
        pi[3] = (int)data[3];
      }
      break;

    case NT_packed_ufloat:
      nassertv(false);
      break;
    }
    break;
  }
}

/**
 *
 */
void GeomVertexColumn::Packer::
set_data1d(unsigned char *pointer, double data) {
  switch (_column->get_num_values()) {
  case 1:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      *(uint8_t *)pointer = (unsigned int)data;
      break;

    case NT_uint16:
      *(uint16_t *)pointer = (unsigned int)data;
      break;

    case NT_uint32:
      *(uint32_t *)pointer = (unsigned int)data;
      break;

    case NT_packed_dcba:
    case NT_packed_dabc:
      nassertv(false);
      break;

    case NT_float32:
      *(PN_float32 *)pointer = data;
      break;

    case NT_float64:
      *(PN_float64 *)pointer = data;
      break;

    case NT_stdfloat:
      nassertv(false);
      break;

    case NT_int8:
      *(int8_t *)pointer = (int)data;
      break;

    case NT_int16:
      *(int16_t *)pointer = (int)data;
      break;

    case NT_int32:
      *(int32_t *)pointer = (int)data;
      break;

    case NT_packed_ufloat:
      nassertv(false);
      break;
    }
    break;

  case 2:
    set_data2d(pointer, LVecBase2d(data, 0.0f));
    break;

  case 3:
    set_data3d(pointer, LVecBase3d(data, 0.0f, 0.0f));
    break;

  case 4:
    set_data4d(pointer, LVecBase4d(data, 0.0f, 0.0f, 0.0f));
    break;
  }
}

/**
 *
 */
void GeomVertexColumn::Packer::
set_data2d(unsigned char *pointer, const LVecBase2d &data) {
  switch (_column->get_num_values()) {
  case 1:
    set_data1d(pointer, data[0]);

  case 2:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        uint8_t *pi = (uint8_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
      }
      break;

    case NT_uint16:
      {
        uint16_t *pi = (uint16_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
      }
      break;

    case NT_uint32:
      {
        uint32_t *pi = (uint32_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
      }
      break;

    case NT_packed_dcba:
    case NT_packed_dabc:
      nassertv(false);
      break;

    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
      }
      break;

    case NT_float64:
      {
        PN_float64 *pi = (PN_float64 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
      }
      break;

    case NT_stdfloat:
      nassertv(false);
      break;

    case NT_int8:
      {
        int8_t *pi = (int8_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
      }
      break;

    case NT_int16:
      {
        int16_t *pi = (int16_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
      }
      break;

    case NT_int32:
      {
        int32_t *pi = (int32_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
      }
      break;

    case NT_packed_ufloat:
      nassertv(false);
      break;
    }
    break;

  case 3:
    set_data3d(pointer, LVecBase3d(data[0], data[1], 0.0f));
    break;

  default:
    set_data4d(pointer, LVecBase4d(data[0], data[1], 0.0f, 0.0f));
    break;
  }
}

/**
 *
 */
void GeomVertexColumn::Packer::
set_data3d(unsigned char *pointer, const LVecBase3d &data) {
  switch (_column->get_num_values()) {
  case 1:
    set_data1d(pointer, data[0]);
    break;

  case 2:
    set_data2d(pointer, LVecBase2d(data[0], data[1]));
    break;

  case 3:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        uint8_t *pi = (uint8_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
      }
      break;

    case NT_uint16:
      {
        uint16_t *pi = (uint16_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
      }
      break;

    case NT_uint32:
      {
        uint32_t *pi = (uint32_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
      }
      break;

    case NT_packed_dcba:
    case NT_packed_dabc:
      nassertv(false);
      break;

    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
      }
      break;

    case NT_float64:
      {
        PN_float64 *pi = (PN_float64 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
      }
      break;

    case NT_stdfloat:
      nassertv(false);
      break;

    case NT_int8:
      {
        int8_t *pi = (int8_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
      }
      break;

    case NT_int16:
      {
        int16_t *pi = (int16_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
      }
      break;

    case NT_int32:
      {
        int32_t *pi = (int32_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
      }
      break;

    case NT_packed_ufloat:
      *(uint32_t *)pointer = GeomVertexData::pack_ufloat(data[0], data[1], data[2]);
      break;
    }
    break;

  default:
    set_data4d(pointer, LVecBase4d(data[0], data[1], data[2], 0.0f));
    break;
  }
}

/**
 *
 */
void GeomVertexColumn::Packer::
set_data4d(unsigned char *pointer, const LVecBase4d &data) {
  switch (_column->get_num_values()) {
  case 1:
    set_data1d(pointer, data[0]);
    break;

  case 2:
    set_data2d(pointer, LVecBase2d(data[0], data[1]));
    break;

  case 3:
    set_data3d(pointer, LVecBase3d(data[0], data[1], data[2]));
    break;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        uint8_t *pi = (uint8_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;

    case NT_uint16:
      {
        uint16_t *pi = (uint16_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;

    case NT_uint32:
      {
        uint32_t *pi = (uint32_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;

    case NT_packed_dcba:
      *(uint32_t *)pointer = GeomVertexData::pack_abcd(data[3], data[2], data[1], data[0]);
      break;

    case NT_packed_dabc:
      *(uint32_t *)pointer = GeomVertexData::pack_abcd(data[3], data[0], data[1], data[2]);
      break;

    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_float64:
      {
        PN_float64 *pi = (PN_float64 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_stdfloat:
      nassertv(false);
      break;

    case NT_int8:
      {
        int8_t *pi = (int8_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
        pi[3] = (int)data[3];
      }
      break;

    case NT_int16:
      {
        int16_t *pi = (int16_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
        pi[3] = (int)data[3];
      }
      break;

    case NT_int32:
      {
        int32_t *pi = (int32_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
        pi[3] = (int)data[3];
      }
      break;

    case NT_packed_ufloat:
      nassertv(false);
      break;
    }
    break;
  }
}

/**
 *
 */
void GeomVertexColumn::Packer::
set_data1i(unsigned char *pointer, int data) {
  switch (_column->get_num_values()) {
  case 1:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      *pointer = data;
      nassertv((*pointer) == data);
      break;

    case NT_uint16:
      *(uint16_t *)pointer = data;
      nassertv(*(uint16_t *)pointer == data);
      break;

    case NT_uint32:
      *(uint32_t *)pointer = data;
      break;

    case NT_packed_dcba:
    case NT_packed_dabc:
      nassertv(false);
      break;

    case NT_float32:
      *(PN_float32 *)pointer = (float)data;
      break;

    case NT_float64:
      *(PN_float64 *)pointer = (double)data;
      break;

    case NT_stdfloat:
      nassertv(false);
      break;

    case NT_int8:
      *(int8_t *)pointer = data;
      nassertv(*(int8_t *)pointer == data);
      break;

    case NT_int16:
      *(int16_t *)pointer = data;
      nassertv(*(int16_t *)pointer == data);
      break;

    case NT_int32:
      *(int32_t *)pointer = data;
      break;

    case NT_packed_ufloat:
      nassertv(false);
      break;
    }
    break;

  case 2:
    set_data2i(pointer, LVecBase2i(data, 0));
    break;

  case 3:
    set_data3i(pointer, LVecBase3i(data, 0, 0));
    break;

  default:
    set_data4i(pointer, LVecBase4i(data, 0, 0, 0));
    break;
  }
}

/**
 *
 */
void GeomVertexColumn::Packer::
set_data2i(unsigned char *pointer, const LVecBase2i &data) {
  switch (_column->get_num_values()) {
  case 1:
    set_data1i(pointer, data[0]);
    break;

  case 2:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      pointer[0] = data[0];
      pointer[1] = data[1];
      break;

    case NT_uint16:
      {
        uint16_t *pi = (uint16_t *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
      }
      break;

    case NT_uint32:
      {
        uint32_t *pi = (uint32_t *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
      }
      break;

    case NT_packed_dcba:
    case NT_packed_dabc:
      nassertv(false);
      break;

    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
      }
      break;

    case NT_float64:
      {
        PN_float64 *pi = (PN_float64 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
      }
      break;

    case NT_stdfloat:
      nassertv(false);
      break;

    case NT_int8:
      {
        int8_t *pi = (int8_t *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
      }
      break;

    case NT_int16:
      {
        int16_t *pi = (int16_t *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
      }
      break;

    case NT_int32:
      {
        int32_t *pi = (int32_t *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
      }
      break;

    case NT_packed_ufloat:
      nassertv(false);
      break;
    }
    break;

  case 3:
    set_data3i(pointer, LVecBase3i(data[0], data[1], 0));
    break;

  default:
    set_data4i(pointer, LVecBase4i(data[0], data[1], 0, 0));
    break;
  }
}

/**
 *
 */
void GeomVertexColumn::Packer::
set_data3i(unsigned char *pointer, const LVecBase3i &data) {
  switch (_column->get_num_values()) {
  case 1:
    set_data1i(pointer, data[0]);
    break;

  case 2:
    set_data2i(pointer, LVecBase2i(data[0], data[1]));
    break;

  case 3:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      pointer[0] = data[0];
      pointer[1] = data[1];
      pointer[2] = data[2];
      break;

    case NT_uint16:
      {
        uint16_t *pi = (uint16_t *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
      }
      break;

    case NT_uint32:
      {
        uint32_t *pi = (uint32_t *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
      }
      break;

    case NT_packed_dcba:
    case NT_packed_dabc:
      nassertv(false);
      break;

    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
      }
      break;

    case NT_float64:
      {
        PN_float64 *pi = (PN_float64 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
      }
      break;

    case NT_stdfloat:
      nassertv(false);
      break;

    case NT_int8:
      {
        int8_t *pi = (int8_t *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
      }
      break;

    case NT_int16:
      {
        int16_t *pi = (int16_t *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
      }
      break;

    case NT_int32:
      {
        int32_t *pi = (int32_t *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
      }
      break;

    case NT_packed_ufloat:
      *(uint32_t *)pointer = GeomVertexData::pack_ufloat(data[0], data[1], data[2]);
      break;
    }
    break;

  default:
    set_data4i(pointer, LVecBase4i(data[0], data[1], data[2], 0));
    break;
  }
}

/**
 *
 */
void GeomVertexColumn::Packer::
set_data4i(unsigned char *pointer, const LVecBase4i &data) {
  switch (_column->get_num_values()) {
  case 1:
    set_data1i(pointer, data[0]);
    break;

  case 2:
    set_data2i(pointer, LVecBase2i(data[0], data[1]));
    break;

  case 3:
    set_data3i(pointer, LVecBase3i(data[0], data[1], data[2]));
    break;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      pointer[0] = data[0];
      pointer[1] = data[1];
      pointer[2] = data[2];
      pointer[3] = data[3];
      break;

    case NT_uint16:
      {
        uint16_t *pi = (uint16_t *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_uint32:
      {
        uint32_t *pi = (uint32_t *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_packed_dcba:
      *(uint32_t *)pointer = GeomVertexData::pack_abcd(data[3], data[2], data[1], data[0]);
      break;

    case NT_packed_dabc:
      *(uint32_t *)pointer = GeomVertexData::pack_abcd(data[3], data[0], data[1], data[2]);
      break;

    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_float64:
      {
        PN_float64 *pi = (PN_float64 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_stdfloat:
      nassertv(false);
      break;

    case NT_int8:
      {
        int8_t *pi = (int8_t *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_int16:
      {
        int16_t *pi = (int16_t *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_int32:
      {
        int32_t *pi = (int32_t *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_packed_ufloat:
      nassertv(false);
      break;
    }
    break;
  }
}

/**
 *
 */
float GeomVertexColumn::Packer_point::
get_data1f(const unsigned char *pointer) {
  if (_column->get_num_values() == 4) {
    const LVecBase4f &v4 = get_data4f(pointer);
    return v4[0] / v4[3];
  } else {
    return Packer::get_data1f(pointer);
  }
}

/**
 *
 */
const LVecBase2f &GeomVertexColumn::Packer_point::
get_data2f(const unsigned char *pointer) {
  if (_column->get_num_values() == 4) {
    const LVecBase4f &v4 = get_data4f(pointer);
    _v2.set(v4[0] / v4[3], v4[1] / v4[3]);
    return _v2;
  } else {
    return Packer::get_data2f(pointer);
  }
}

/**
 *
 */
const LVecBase3f &GeomVertexColumn::Packer_point::
get_data3f(const unsigned char *pointer) {
  if (_column->get_num_values() == 4) {
    const LVecBase4f &v4 = get_data4f(pointer);
    _v3.set(v4[0] / v4[3], v4[1] / v4[3], v4[2] / v4[3]);
    return _v3;
  } else {
    return Packer::get_data3f(pointer);
  }
}

/**
 *
 */
const LVecBase4f &GeomVertexColumn::Packer_point::
get_data4f(const unsigned char *pointer) {
  switch (_column->get_num_values()) {
  case 1:
    _v4.set(get_data1f(pointer), 0.0f, 0.0f, 1.0f);
    return _v4;

  case 2:
    {
      const LVecBase2f &v2 = get_data2f(pointer);
      _v4.set(v2[0], v2[1], 0.0f, 1.0f);
    }
    return _v4;

  case 3:
    {
      const LVecBase3f &v3 = get_data3f(pointer);
      _v4.set(v3[0], v3[1], v3[2], 1.0f);
    }
    return _v4;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        const uint8_t *pi = (const uint8_t *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_uint16:
      {
        const uint16_t *pi = (const uint16_t *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_uint32:
      {
        const uint32_t *pi = (const uint32_t *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_packed_dcba:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v4.set(GeomVertexData::unpack_abcd_d(dword),
                GeomVertexData::unpack_abcd_c(dword),
                GeomVertexData::unpack_abcd_b(dword),
                GeomVertexData::unpack_abcd_a(dword));
      }
      return _v4;

    case NT_packed_dabc:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v4.set(GeomVertexData::unpack_abcd_b(dword),
                GeomVertexData::unpack_abcd_c(dword),
                GeomVertexData::unpack_abcd_d(dword),
                GeomVertexData::unpack_abcd_a(dword));
      }
      return _v4;

    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_float64:
      {
        const PN_float64 *pi = (const PN_float64 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_stdfloat:
      nassertr(false, _v4);
      break;

    case NT_int8:
      {
        const int8_t *pi = (const int8_t *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_int16:
      {
        const int16_t *pi = (const int16_t *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_int32:
      {
        const int32_t *pi = (const int32_t *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_packed_ufloat:
      nassertr(false, _v4);
      break;
    }
  }

  return _v4;
}

/**
 *
 */
double GeomVertexColumn::Packer_point::
get_data1d(const unsigned char *pointer) {
  if (_column->get_num_values() == 4) {
    const LVecBase4d &v4 = get_data4d(pointer);
    return v4[0] / v4[3];
  } else {
    return Packer::get_data1d(pointer);
  }
}

/**
 *
 */
const LVecBase2d &GeomVertexColumn::Packer_point::
get_data2d(const unsigned char *pointer) {
  if (_column->get_num_values() == 4) {
    const LVecBase4d &v4 = get_data4d(pointer);
    _v2d.set(v4[0] / v4[3], v4[1] / v4[3]);
    return _v2d;
  } else {
    return Packer::get_data2d(pointer);
  }
}

/**
 *
 */
const LVecBase3d &GeomVertexColumn::Packer_point::
get_data3d(const unsigned char *pointer) {
  if (_column->get_num_values() == 4) {
    const LVecBase4d &v4 = get_data4d(pointer);
    _v3d.set(v4[0] / v4[3], v4[1] / v4[3], v4[2] / v4[3]);
    return _v3d;
  } else {
    return Packer::get_data3d(pointer);
  }
}

/**
 *
 */
const LVecBase4d &GeomVertexColumn::Packer_point::
get_data4d(const unsigned char *pointer) {
  switch (_column->get_num_values()) {
  case 1:
    _v4d.set(get_data1d(pointer), 0.0f, 0.0f, 1.0f);
    return _v4d;

  case 2:
    {
      const LVecBase2d &v2 = get_data2d(pointer);
      _v4d.set(v2[0], v2[1], 0.0f, 1.0f);
    }
    return _v4d;

  case 3:
    {
      const LVecBase3d &v3 = get_data3d(pointer);
      _v4d.set(v3[0], v3[1], v3[2], 1.0f);
    }
    return _v4d;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        const uint8_t *pi = (const uint8_t *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_uint16:
      {
        const uint16_t *pi = (const uint16_t *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_uint32:
      {
        const uint32_t *pi = (const uint32_t *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_packed_dcba:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v4d.set(GeomVertexData::unpack_abcd_d(dword),
                 GeomVertexData::unpack_abcd_c(dword),
                 GeomVertexData::unpack_abcd_b(dword),
                 GeomVertexData::unpack_abcd_a(dword));
      }
      return _v4d;

    case NT_packed_dabc:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v4d.set(GeomVertexData::unpack_abcd_b(dword),
                 GeomVertexData::unpack_abcd_c(dword),
                 GeomVertexData::unpack_abcd_d(dword),
                 GeomVertexData::unpack_abcd_a(dword));
      }
      return _v4d;

    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_float64:
      {
        const PN_float64 *pi = (const PN_float64 *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_stdfloat:
      nassertr(false, _v4d);
      break;

    case NT_int8:
      {
        const int8_t *pi = (const int8_t *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_int16:
      {
        const int16_t *pi = (const int16_t *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_int32:
      {
        const int32_t *pi = (const int32_t *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_packed_ufloat:
      nassertr(false, _v4d);
      break;
    }
  }

  return _v4d;
}

/**
 *
 */
void GeomVertexColumn::Packer_point::
set_data1f(unsigned char *pointer, float data) {
  if (_column->get_num_values() == 4) {
    set_data4f(pointer, LVecBase4f(data, 0.0f, 0.0f, 1.0f));
  } else {
    Packer::set_data1f(pointer, data);
  }
}

/**
 *
 */
void GeomVertexColumn::Packer_point::
set_data2f(unsigned char *pointer, const LVecBase2f &data) {
  if (_column->get_num_values() == 4) {
    set_data4f(pointer, LVecBase4f(data[0], data[1], 0.0f, 1.0f));
  } else {
    Packer::set_data2f(pointer, data);
  }
}

/**
 *
 */
void GeomVertexColumn::Packer_point::
set_data3f(unsigned char *pointer, const LVecBase3f &data) {
  if (_column->get_num_values() == 4) {
    set_data4f(pointer, LVecBase4f(data[0], data[1], data[2], 1.0f));
  } else {
    Packer::set_data3f(pointer, data);
  }
}

/**
 *
 */
void GeomVertexColumn::Packer_point::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  switch (_column->get_num_values()) {
  case 1:
    set_data1f(pointer, data[0] / data[3]);
    break;

  case 2:
    set_data2f(pointer, LVecBase2f(data[0] / data[3], data[1] / data[3]));
    break;

  case 3:
    set_data3f(pointer, LVecBase3f(data[0] / data[3], data[1] / data[3], data[2] / data[3]));
    break;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        uint8_t *pi = (uint8_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;

    case NT_uint16:
      {
        uint16_t *pi = (uint16_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;

    case NT_uint32:
      {
        uint32_t *pi = (uint32_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;

    case NT_packed_dcba:
      *(uint32_t *)pointer = GeomVertexData::pack_abcd(data[3], data[2], data[1], data[0]);
      break;

    case NT_packed_dabc:
      *(uint32_t *)pointer = GeomVertexData::pack_abcd(data[3], data[0], data[1], data[2]);
      break;

    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_float64:
      {
        PN_float64 *pi = (PN_float64 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_stdfloat:
      nassertv(false);
      break;

    case NT_int8:
      {
        int8_t *pi = (int8_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
        pi[3] = (int)data[3];
      }
      break;

    case NT_int16:
      {
        int16_t *pi = (int16_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
        pi[3] = (int)data[3];
      }
      break;

    case NT_int32:
      {
        int32_t *pi = (int32_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
        pi[3] = (int)data[3];
      }
      break;

    case NT_packed_ufloat:
      nassertv(false);
      break;
    }
    break;
  }
}

/**
 *
 */
void GeomVertexColumn::Packer_point::
set_data1d(unsigned char *pointer, double data) {
  if (_column->get_num_values() == 4) {
    set_data4d(pointer, LVecBase4d(data, 0.0f, 0.0f, 1.0f));
  } else {
    Packer::set_data1d(pointer, data);
  }
}

/**
 *
 */
void GeomVertexColumn::Packer_point::
set_data2d(unsigned char *pointer, const LVecBase2d &data) {
  if (_column->get_num_values() == 4) {
    set_data4d(pointer, LVecBase4d(data[0], data[1], 0.0f, 1.0f));
  } else {
    Packer::set_data2d(pointer, data);
  }
}

/**
 *
 */
void GeomVertexColumn::Packer_point::
set_data3d(unsigned char *pointer, const LVecBase3d &data) {
  if (_column->get_num_values() == 4) {
    set_data4d(pointer, LVecBase4d(data[0], data[1], data[2], 1.0f));
  } else {
    Packer::set_data3d(pointer, data);
  }
}

/**
 *
 */
void GeomVertexColumn::Packer_point::
set_data4d(unsigned char *pointer, const LVecBase4d &data) {
  switch (_column->get_num_values()) {
  case 1:
    set_data1d(pointer, data[0] / data[3]);
    break;

  case 2:
    set_data2d(pointer, LVecBase2d(data[0] / data[3], data[1] / data[3]));
    break;

  case 3:
    set_data3d(pointer, LVecBase3d(data[0] / data[3], data[1] / data[3], data[2] / data[3]));
    break;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        uint8_t *pi = (uint8_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;

    case NT_uint16:
      {
        uint16_t *pi = (uint16_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;

    case NT_uint32:
      {
        uint32_t *pi = (uint32_t *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;

    case NT_packed_dcba:
      *(uint32_t *)pointer = GeomVertexData::pack_abcd(data[3], data[2], data[1], data[0]);
      break;

    case NT_packed_dabc:
      *(uint32_t *)pointer = GeomVertexData::pack_abcd(data[3], data[0], data[1], data[2]);
      break;

    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_float64:
      {
        PN_float64 *pi = (PN_float64 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_stdfloat:
      nassertv(false);
      break;

    case NT_int8:
      {
        int8_t *pi = (int8_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
        pi[3] = (int)data[3];
      }
      break;

    case NT_int16:
      {
        int16_t *pi = (int16_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
        pi[3] = (int)data[3];
      }
      break;

    case NT_int32:
      {
        int32_t *pi = (int32_t *)pointer;
        pi[0] = (int)data[0];
        pi[1] = (int)data[1];
        pi[2] = (int)data[2];
        pi[3] = (int)data[3];
      }
      break;

    case NT_packed_ufloat:
      nassertv(false);
      break;
    }
    break;
  }
}

/**
 *
 */
float GeomVertexColumn::Packer_color::
get_data1f(const unsigned char *pointer) {
  switch (_column->get_numeric_type()) {
  case NT_uint8:
    return (*(const uint8_t *)pointer) / 255.0f;

  case NT_uint16:
    return (*(const uint16_t *)pointer) / 65535.0f;

  case NT_uint32:
    return (*(const uint32_t *)pointer) / 4294967295.0f;

  case NT_packed_dcba:
    {
      uint32_t dword = *(const uint32_t *)pointer;
      return GeomVertexData::unpack_abcd_d(dword) / 255.0f;
    }

  case NT_packed_dabc:
    {
      uint32_t dword = *(const uint32_t *)pointer;
      return GeomVertexData::unpack_abcd_b(dword) / 255.0f;
    }

  case NT_float32:
    return *(const PN_float32 *)pointer;

  case NT_float64:
    return *(const PN_float64 *)pointer;

  default:
    nassertr(false, 0.0f);
  }

  return 0.0f;
}

/**
 *
 */
const LVecBase2f &GeomVertexColumn::Packer_color::
get_data2f(const unsigned char *pointer) {
  if (_column->get_num_values() == 3) {
    const LVecBase3f &v3 = get_data3f(pointer);
    _v2.set(v3[0], v3[1]);
    return _v2;
  } else {
    const LVecBase4f &v4 = get_data4f(pointer);
    _v2.set(v4[0], v4[1]);
    return _v2;
  }
}

/**
 *
 */
const LVecBase3f &GeomVertexColumn::Packer_color::
get_data3f(const unsigned char *pointer) {
  if (_column->get_num_values() == 3) {
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        const uint8_t *pi = (const uint8_t *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
        _v3 /= 255.0f;
      }
      return _v3;

    case NT_uint16:
      {
        const uint16_t *pi = (const uint16_t *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
        _v3 /= 65535.0f;
      }
      return _v3;

    case NT_uint32:
      {
        const uint32_t *pi = (const uint32_t *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
        _v3 /= 4294967295.0f;
      }
      return _v3;

    case NT_packed_dcba:
    case NT_packed_dabc:
      nassertr(false, _v3);
      return _v3;

    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
      }
      return _v3;

    case NT_float64:
      {
        const PN_float64 *pi = (const PN_float64 *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
      }
      return _v3;

    case NT_stdfloat:
    case NT_int8:
    case NT_int16:
    case NT_int32:
      nassertr(false, _v3);
      return _v3;

    case NT_packed_ufloat:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v3.set(GeomVertexData::unpack_ufloat_a(dword),
                GeomVertexData::unpack_ufloat_b(dword),
                GeomVertexData::unpack_ufloat_c(dword));
      }
      return _v3;
    }
  } else {
    const LVecBase4f &v4 = get_data4f(pointer);
    _v3.set(v4[0], v4[1], v4[2]);
    return _v3;
  }

  return _v3;
}

/**
 *
 */
const LVecBase4f &GeomVertexColumn::Packer_color::
get_data4f(const unsigned char *pointer) {
  if (_column->get_num_values() == 3) {
    const LVecBase3f &v3 = get_data3f(pointer);
    _v4.set(v3[0], v3[1], v3[2], 1.0f);
    return _v4;
  } else {
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        const uint8_t *pi = (const uint8_t *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
        _v4 /= 255.0f;
      }
      return _v4;

    case NT_uint16:
      {
        const uint16_t *pi = (const uint16_t *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
        _v4 /= 65535.0f;
      }
      return _v4;

    case NT_uint32:
      {
        const uint32_t *pi = (const uint32_t *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
        _v4 /= 4294967295.0f;
      }
      return _v4;

    case NT_packed_dcba:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v4.set(GeomVertexData::unpack_abcd_d(dword),
                GeomVertexData::unpack_abcd_c(dword),
                GeomVertexData::unpack_abcd_b(dword),
                GeomVertexData::unpack_abcd_a(dword));
        _v4 /= 255.0f;
      }
      return _v4;

    case NT_packed_dabc:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v4.set(GeomVertexData::unpack_abcd_b(dword),
                GeomVertexData::unpack_abcd_c(dword),
                GeomVertexData::unpack_abcd_d(dword),
                GeomVertexData::unpack_abcd_a(dword));
        _v4 /= 255.0f;
      }
      return _v4;

    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_float64:
      {
        const PN_float64 *pi = (const PN_float64 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;

    case NT_stdfloat:
    case NT_int8:
    case NT_int16:
    case NT_int32:
    case NT_packed_ufloat:
      nassertr(false, _v4);
      return _v4;
    }
  }

  return _v4;
}

/**
 *
 */
double GeomVertexColumn::Packer_color::
get_data1d(const unsigned char *pointer) {
  switch (_column->get_numeric_type()) {
  case NT_uint8:
    return (*(const uint8_t *)pointer) / 255.0;

  case NT_uint16:
    return (*(const uint16_t *)pointer) / 65535.0;

  case NT_uint32:
    return (*(const uint32_t *)pointer) / 4294967295.0;

  case NT_packed_dcba:
    {
      uint32_t dword = *(const uint32_t *)pointer;
      return GeomVertexData::unpack_abcd_d(dword) / 255.0;
    }

  case NT_packed_dabc:
    {
      uint32_t dword = *(const uint32_t *)pointer;
      return GeomVertexData::unpack_abcd_b(dword) / 255.0;
    }

  case NT_float32:
    return *(const PN_float32 *)pointer;

  case NT_float64:
    return *(const PN_float64 *)pointer;

  default:
    nassertr(false, 0.0);
  }

  return 0.0;
}

/**
 *
 */
const LVecBase2d &GeomVertexColumn::Packer_color::
get_data2d(const unsigned char *pointer) {
  if (_column->get_num_values() == 3) {
    const LVecBase3d &v3 = get_data3d(pointer);
    _v2d.set(v3[0], v3[1]);
    return _v2d;
  } else {
    const LVecBase4d &v4 = get_data4d(pointer);
    _v2d.set(v4[0], v4[1]);
    return _v2d;
  }
}

/**
 *
 */
const LVecBase3d &GeomVertexColumn::Packer_color::
get_data3d(const unsigned char *pointer) {
  if (_column->get_num_values() == 3) {
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        const uint8_t *pi = (const uint8_t *)pointer;
        _v3d.set(pi[0], pi[1], pi[2]);
        _v3d /= 255.0;
      }
      return _v3d;

    case NT_uint16:
      {
        const uint16_t *pi = (const uint16_t *)pointer;
        _v3d.set(pi[0], pi[1], pi[2]);
        _v3d /= 65535.0;
      }
      return _v3d;

    case NT_uint32:
      {
        const uint32_t *pi = (const uint32_t *)pointer;
        _v3d.set(pi[0], pi[1], pi[2]);
        _v3d /= 4294967295.0;
      }
      return _v3d;

    case NT_packed_dcba:
    case NT_packed_dabc:
      nassertr(false, _v3d);
      return _v3d;

    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v3d.set(pi[0], pi[1], pi[2]);
      }
      return _v3d;

    case NT_float64:
      {
        const PN_float64 *pi = (const PN_float64 *)pointer;
        _v3d.set(pi[0], pi[1], pi[2]);
      }
      return _v3d;

    case NT_stdfloat:
    case NT_int8:
    case NT_int16:
    case NT_int32:
      nassertr(false, _v3d);
      return _v3d;

    case NT_packed_ufloat:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v3d.set(GeomVertexData::unpack_ufloat_a(dword),
                 GeomVertexData::unpack_ufloat_b(dword),
                 GeomVertexData::unpack_ufloat_c(dword));
      }
      return _v3d;
    }
  } else {
    const LVecBase4d &v4 = get_data4d(pointer);
    _v3d.set(v4[0], v4[1], v4[2]);
    return _v3d;
  }

  return _v3d;
}

/**
 *
 */
const LVecBase4d &GeomVertexColumn::Packer_color::
get_data4d(const unsigned char *pointer) {
  if (_column->get_num_values() == 3) {
    const LVecBase3d &v3 = get_data3d(pointer);
    _v4d.set(v3[0], v3[1], v3[2], 1.0);
    return _v4d;
  } else {
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        const uint8_t *pi = (const uint8_t *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
        _v4d /= 255.0;
      }
      return _v4d;

    case NT_uint16:
      {
        const uint16_t *pi = (const uint16_t *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
        _v4d /= 65535.0;
      }
      return _v4d;

    case NT_uint32:
      {
        const uint32_t *pi = (const uint32_t *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
        _v4d /= 4294967295.0;
      }
      return _v4d;

    case NT_packed_dcba:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v4d.set(GeomVertexData::unpack_abcd_d(dword),
                 GeomVertexData::unpack_abcd_c(dword),
                 GeomVertexData::unpack_abcd_b(dword),
                 GeomVertexData::unpack_abcd_a(dword));
        _v4d /= 255.0;
      }
      return _v4d;

    case NT_packed_dabc:
      {
        uint32_t dword = *(const uint32_t *)pointer;
        _v4d.set(GeomVertexData::unpack_abcd_b(dword),
                 GeomVertexData::unpack_abcd_c(dword),
                 GeomVertexData::unpack_abcd_d(dword),
                 GeomVertexData::unpack_abcd_a(dword));
        _v4d /= 255.0;
      }
      return _v4d;

    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_float64:
      {
        const PN_float64 *pi = (const PN_float64 *)pointer;
        _v4d.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4d;

    case NT_stdfloat:
    case NT_int8:
    case NT_int16:
    case NT_int32:
    case NT_packed_ufloat:
      nassertr(false, _v4d);
      return _v4d;
    }
  }

  return _v4d;
}

/**
 *
 */
void GeomVertexColumn::Packer_color::
set_data1f(unsigned char *pointer, float data) {
  if (_column->get_num_values() == 3) {
    set_data3f(pointer, LVecBase3f(data, 0.0f, 0.0f));
  } else {
    set_data4f(pointer, LVecBase4f(data, 0.0f, 0.0f, 1.0f));
  }
}

/**
 *
 */
void GeomVertexColumn::Packer_color::
set_data2f(unsigned char *pointer, const LVecBase2f &data) {
  if (_column->get_num_values() == 3) {
    set_data3f(pointer, LVecBase3f(data[0], data[1], 0.0f));
  } else {
    set_data4f(pointer, LVecBase4f(data[0], data[1], 0.0f, 1.0f));
  }
}

/**
 *
 */
void GeomVertexColumn::Packer_color::
set_data3f(unsigned char *pointer, const LVecBase3f &data) {
  if (_column->get_num_values() == 3) {
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        LVecBase3f scaled = data * 255.0f;
        uint8_t *pi = (uint8_t *)pointer;
        pi[0] = (unsigned int)scaled[0];
        pi[1] = (unsigned int)scaled[1];
        pi[2] = (unsigned int)scaled[2];
      }
      break;

    case NT_uint16:
      {
        LVecBase3f scaled = data * 65535.0f;
        uint16_t *pi = (uint16_t *)pointer;
        pi[0] = (unsigned int)scaled[0];
        pi[1] = (unsigned int)scaled[1];
        pi[2] = (unsigned int)scaled[2];
      }
      break;

    case NT_uint32:
      {
        LVecBase3f scaled = data * 4294967295.0f;
        uint32_t *pi = (uint32_t *)pointer;
        pi[0] = (unsigned int)scaled[0];
        pi[1] = (unsigned int)scaled[1];
        pi[2] = (unsigned int)scaled[2];
      }
      break;

    case NT_packed_dcba:
      {
        LVecBase3f scaled = data * 255.0f;
        *(uint32_t *)pointer = GeomVertexData::pack_abcd(
          1.0f, scaled[2], scaled[1], scaled[0]);
      }
      break;

    case NT_packed_dabc:
      {
        LVecBase3f scaled = data * 255.0f;
        *(uint32_t *)pointer = GeomVertexData::pack_abcd(
          1.0f, scaled[0], scaled[1], scaled[2]);
      }
      break;

    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
      }
      break;

    case NT_float64:
      {
        PN_float64 *pi = (PN_float64 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
      }
      break;

    case NT_stdfloat:
    case NT_int8:
    case NT_int16:
    case NT_int32:
      nassertv(false);
      break;

    case NT_packed_ufloat:
      *(uint32_t *)pointer = GeomVertexData::pack_ufloat(data[0], data[1], data[2]);
      break;
    }
  } else {
    set_data4f(pointer, LVecBase4f(data[0], data[1], data[2], 1.0f));
  }
}

/**
 *
 */
void GeomVertexColumn::Packer_color::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  if (_column->get_num_values() == 3) {
    set_data3f(pointer, LVecBase3f(data[0], data[1], data[2]));
  } else {
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        LVecBase4f scaled = data * 255.0f;
        uint8_t *pi = (uint8_t *)pointer;
        pi[0] = (unsigned int)scaled[0];
        pi[1] = (unsigned int)scaled[1];
        pi[2] = (unsigned int)scaled[2];
        pi[3] = (unsigned int)scaled[3];
      }
      break;

    case NT_uint16:
      {
        LVecBase4f scaled = data * 65535.0f;
        uint16_t *pi = (uint16_t *)pointer;
        pi[0] = (unsigned int)scaled[0];
        pi[1] = (unsigned int)scaled[1];
        pi[2] = (unsigned int)scaled[2];
        pi[3] = (unsigned int)scaled[3];
      }
      break;

    case NT_uint32:
      {
        LVecBase4f scaled = data * 4294967295.0f;
        uint32_t *pi = (uint32_t *)pointer;
        pi[0] = (unsigned int)scaled[0];
        pi[1] = (unsigned int)scaled[1];
        pi[2] = (unsigned int)scaled[2];
        pi[3] = (unsigned int)scaled[3];
      }
      break;

    case NT_packed_dcba:
      {
        LVecBase4f scaled = data * 255.0f;
        *(uint32_t *)pointer = GeomVertexData::pack_abcd(
          scaled[3], scaled[2], scaled[1], scaled[0]);
      }
      break;

    case NT_packed_dabc:
      {
        LVecBase4f scaled = data * 255.0f;
        *(uint32_t *)pointer = GeomVertexData::pack_abcd(
          scaled[3], scaled[0], scaled[1], scaled[2]);
      }
      break;

    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_float64:
      {
        PN_float64 *pi = (PN_float64 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_stdfloat:
    case NT_int8:
    case NT_int16:
    case NT_int32:
      nassertv(false);
      break;

    case NT_packed_ufloat:
      nassertv(false);
      break;
    }
  }
}

/**
 *
 */
void GeomVertexColumn::Packer_color::
set_data1d(unsigned char *pointer, double data) {
  if (_column->get_num_values() == 3) {
    set_data3d(pointer, LVecBase3d(data, 0.0, 0.0));
  } else {
    set_data4d(pointer, LVecBase4d(data, 0.0, 0.0, 1.0));
  }
}

/**
 *
 */
void GeomVertexColumn::Packer_color::
set_data2d(unsigned char *pointer, const LVecBase2d &data) {
  if (_column->get_num_values() == 3) {
    set_data3d(pointer, LVecBase3d(data[0], data[1], 0.0));
  } else {
    set_data4d(pointer, LVecBase4d(data[0], data[1], 0.0, 1.0));
  }
}

/**
 *
 */
void GeomVertexColumn::Packer_color::
set_data3d(unsigned char *pointer, const LVecBase3d &data) {
  if (_column->get_num_values() == 3) {
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        LVecBase3d scaled = data * 255.0;
        uint8_t *pi = (uint8_t *)pointer;
        pi[0] = (unsigned int)scaled[0];
        pi[1] = (unsigned int)scaled[1];
        pi[2] = (unsigned int)scaled[2];
      }
      break;

    case NT_uint16:
      {
        LVecBase3d scaled = data * 65535.0;
        uint16_t *pi = (uint16_t *)pointer;
        pi[0] = (unsigned int)scaled[0];
        pi[1] = (unsigned int)scaled[1];
        pi[2] = (unsigned int)scaled[2];
      }
      break;

    case NT_uint32:
      {
        LVecBase3d scaled = data * 4294967295.0;
        uint32_t *pi = (uint32_t *)pointer;
        pi[0] = (unsigned int)scaled[0];
        pi[1] = (unsigned int)scaled[1];
        pi[2] = (unsigned int)scaled[2];
      }
      break;

    case NT_packed_dcba:
      {
        LVecBase3d scaled = data * 255.0;
        *(uint32_t *)pointer = GeomVertexData::pack_abcd(
          1.0, scaled[2], scaled[1], scaled[0]);
      }
      break;

    case NT_packed_dabc:
      {
        LVecBase3d scaled = data * 255.0;
        *(uint32_t *)pointer = GeomVertexData::pack_abcd(
          1.0, scaled[0], scaled[1], scaled[2]);
      }
      break;

    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
      }
      break;

    case NT_float64:
      {
        PN_float64 *pi = (PN_float64 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
      }
      break;

    case NT_stdfloat:
    case NT_int8:
    case NT_int16:
    case NT_int32:
      nassertv(false);
      break;

    case NT_packed_ufloat:
      *(uint32_t *)pointer = GeomVertexData::pack_ufloat(data[0], data[1], data[2]);
      break;
    }
  } else {
    set_data4d(pointer, LVecBase4d(data[0], data[1], data[2], 1.0f));
  }
}

/**
 *
 */
void GeomVertexColumn::Packer_color::
set_data4d(unsigned char *pointer, const LVecBase4d &data) {
  if (_column->get_num_values() == 3) {
    set_data3d(pointer, LVecBase3d(data[0], data[1], data[2]));
  } else {
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      {
        LVecBase4d scaled = data * 255.0;
        uint8_t *pi = (uint8_t *)pointer;
        pi[0] = (unsigned int)scaled[0];
        pi[1] = (unsigned int)scaled[1];
        pi[2] = (unsigned int)scaled[2];
        pi[3] = (unsigned int)scaled[3];
      }
      break;

    case NT_uint16:
      {
        LVecBase4d scaled = data * 65535.0;
        uint16_t *pi = (uint16_t *)pointer;
        pi[0] = (unsigned int)scaled[0];
        pi[1] = (unsigned int)scaled[1];
        pi[2] = (unsigned int)scaled[2];
        pi[3] = (unsigned int)scaled[3];
      }
      break;

    case NT_uint32:
      {
        LVecBase4d scaled = data * 4294967295.0;
        uint32_t *pi = (uint32_t *)pointer;
        pi[0] = (unsigned int)scaled[0];
        pi[1] = (unsigned int)scaled[1];
        pi[2] = (unsigned int)scaled[2];
        pi[3] = (unsigned int)scaled[3];
      }
      break;

    case NT_packed_dcba:
      {
        LVecBase4d scaled = data * 255.0;
        *(uint32_t *)pointer = GeomVertexData::pack_abcd(
          scaled[3], scaled[2], scaled[1], scaled[0]);
      }
      break;

    case NT_packed_dabc:
      {
        LVecBase4d scaled = data * 255.0;
        *(uint32_t *)pointer = GeomVertexData::pack_abcd(
          scaled[3], scaled[0], scaled[1], scaled[2]);
      }
      break;

    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_float64:
      {
        PN_float64 *pi = (PN_float64 *)pointer;
        pi[0] = data[0];
        pi[1] = data[1];
        pi[2] = data[2];
        pi[3] = data[3];
      }
      break;

    case NT_stdfloat:
    case NT_int8:
    case NT_int16:
    case NT_int32:
      nassertv(false);
      break;

    case NT_packed_ufloat:
      nassertv(false);
      break;
    }
  }
}

/**
 *
 */
const LVecBase3f &GeomVertexColumn::Packer_float32_3::
get_data3f(const unsigned char *pointer) {
  const PN_float32 *pi = (const PN_float32 *)pointer;
  _v3.set(pi[0], pi[1], pi[2]);
  return _v3;
}

/**
 *
 */
void GeomVertexColumn::Packer_float32_3::
set_data3f(unsigned char *pointer, const LVecBase3f &data) {
  PN_float32 *pi = (PN_float32 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
  pi[2] = data[2];
}

/**
 *
 */
const LVecBase2f &GeomVertexColumn::Packer_point_float32_2::
get_data2f(const unsigned char *pointer) {
  const PN_float32 *pi = (const PN_float32 *)pointer;
  _v2.set(pi[0], pi[1]);
  return _v2;
}

/**
 *
 */
void GeomVertexColumn::Packer_point_float32_2::
set_data2f(unsigned char *pointer, const LVecBase2f &data) {
  PN_float32 *pi = (PN_float32 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
}

/**
 *
 */
const LVecBase3f &GeomVertexColumn::Packer_point_float32_3::
get_data3f(const unsigned char *pointer) {
  const PN_float32 *pi = (const PN_float32 *)pointer;
  _v3.set(pi[0], pi[1], pi[2]);
  return _v3;
}

/**
 *
 */
void GeomVertexColumn::Packer_point_float32_3::
set_data3f(unsigned char *pointer, const LVecBase3f &data) {
  PN_float32 *pi = (PN_float32 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
  pi[2] = data[2];
}

/**
 *
 */
const LVecBase4f &GeomVertexColumn::Packer_point_float32_4::
get_data4f(const unsigned char *pointer) {
  const PN_float32 *pi = (const PN_float32 *)pointer;
  _v4.set(pi[0], pi[1], pi[2], pi[3]);
  return _v4;
}

/**
 *
 */
void GeomVertexColumn::Packer_point_float32_4::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  PN_float32 *pi = (PN_float32 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
  pi[2] = data[2];
  pi[3] = data[3];
}

/**
 *
 */
const LVecBase3f &GeomVertexColumn::Packer_nativefloat_3::
get_data3f(const unsigned char *pointer) {
  return *(const LVecBase3f *)pointer;
}

/**
 *
 */
const LVecBase2f &GeomVertexColumn::Packer_point_nativefloat_2::
get_data2f(const unsigned char *pointer) {
  return *(const LVecBase2f *)pointer;
}

/**
 *
 */
const LVecBase3f &GeomVertexColumn::Packer_point_nativefloat_3::
get_data3f(const unsigned char *pointer) {
  return *(const LVecBase3f *)pointer;
}

/**
 *
 */
const LVecBase4f &GeomVertexColumn::Packer_point_nativefloat_4::
get_data4f(const unsigned char *pointer) {
  return *(const LVecBase4f *)pointer;
}

/**
 *
 */
const LVecBase3d &GeomVertexColumn::Packer_float64_3::
get_data3d(const unsigned char *pointer) {
  const PN_float64 *pi = (const PN_float64 *)pointer;
  _v3d.set(pi[0], pi[1], pi[2]);
  return _v3d;
}

/**
 *
 */
void GeomVertexColumn::Packer_float64_3::
set_data3d(unsigned char *pointer, const LVecBase3d &data) {
  PN_float64 *pi = (PN_float64 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
  pi[2] = data[2];
}

/**
 *
 */
const LVecBase2d &GeomVertexColumn::Packer_point_float64_2::
get_data2d(const unsigned char *pointer) {
  const PN_float64 *pi = (const PN_float64 *)pointer;
  _v2d.set(pi[0], pi[1]);
  return _v2d;
}

/**
 *
 */
void GeomVertexColumn::Packer_point_float64_2::
set_data2d(unsigned char *pointer, const LVecBase2d &data) {
  PN_float64 *pi = (PN_float64 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
}

/**
 *
 */
const LVecBase3d &GeomVertexColumn::Packer_point_float64_3::
get_data3d(const unsigned char *pointer) {
  const PN_float64 *pi = (const PN_float64 *)pointer;
  _v3d.set(pi[0], pi[1], pi[2]);
  return _v3d;
}

/**
 *
 */
void GeomVertexColumn::Packer_point_float64_3::
set_data3d(unsigned char *pointer, const LVecBase3d &data) {
  PN_float64 *pi = (PN_float64 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
  pi[2] = data[2];
}

/**
 *
 */
const LVecBase4d &GeomVertexColumn::Packer_point_float64_4::
get_data4d(const unsigned char *pointer) {
  const PN_float64 *pi = (const PN_float64 *)pointer;
  _v4d.set(pi[0], pi[1], pi[2], pi[3]);
  return _v4d;
}

/**
 *
 */
void GeomVertexColumn::Packer_point_float64_4::
set_data4d(unsigned char *pointer, const LVecBase4d &data) {
  PN_float64 *pi = (PN_float64 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
  pi[2] = data[2];
  pi[3] = data[3];
}

/**
 *
 */
const LVecBase3d &GeomVertexColumn::Packer_nativedouble_3::
get_data3d(const unsigned char *pointer) {
  return *(const LVecBase3d *)pointer;
}

/**
 *
 */
const LVecBase2d &GeomVertexColumn::Packer_point_nativedouble_2::
get_data2d(const unsigned char *pointer) {
  return *(const LVecBase2d *)pointer;
}

/**
 *
 */
const LVecBase3d &GeomVertexColumn::Packer_point_nativedouble_3::
get_data3d(const unsigned char *pointer) {
  return *(const LVecBase3d *)pointer;
}

/**
 *
 */
const LVecBase4d &GeomVertexColumn::Packer_point_nativedouble_4::
get_data4d(const unsigned char *pointer) {
  return *(const LVecBase4d *)pointer;
}

/**
 *
 */
const LVecBase4f &GeomVertexColumn::Packer_argb_packed::
get_data4f(const unsigned char *pointer) {
  uint32_t dword = *(const uint32_t *)pointer;
  _v4.set(GeomVertexData::unpack_abcd_b(dword),
          GeomVertexData::unpack_abcd_c(dword),
          GeomVertexData::unpack_abcd_d(dword),
          GeomVertexData::unpack_abcd_a(dword));
  _v4 /= 255.0f;
  return _v4;
}

/**
 *
 */
void GeomVertexColumn::Packer_argb_packed::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  // when packing an argb, we want to make sure we cap the input values at 1
  // since going above one will cause the value to be truncated.
  *(uint32_t *)pointer = GeomVertexData::pack_abcd
    ((unsigned int)(min(max(data[3], 0.0f), 1.0f) * 255.0f),
     (unsigned int)(min(max(data[0], 0.0f), 1.0f) * 255.0f),
     (unsigned int)(min(max(data[1], 0.0f), 1.0f) * 255.0f),
     (unsigned int)(min(max(data[2], 0.0f), 1.0f) * 255.0f));
}

/**
 *
 */
const LVecBase4f &GeomVertexColumn::Packer_rgba_uint8_4::
get_data4f(const unsigned char *pointer) {
  _v4.set((float)pointer[0], (float)pointer[1],
          (float)pointer[2], (float)pointer[3]);
  _v4 /= 255.0f;
  return _v4;
}

/**
 *
 */
void GeomVertexColumn::Packer_rgba_uint8_4::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  pointer[0] = (unsigned int)(min(max(data[0], 0.0f), 1.0f) * 255.0f);
  pointer[1] = (unsigned int)(min(max(data[1], 0.0f), 1.0f) * 255.0f);
  pointer[2] = (unsigned int)(min(max(data[2], 0.0f), 1.0f) * 255.0f);
  pointer[3] = (unsigned int)(min(max(data[3], 0.0f), 1.0f) * 255.0f);
}

/**
 *
 */
const LVecBase4f &GeomVertexColumn::Packer_rgba_float32_4::
get_data4f(const unsigned char *pointer) {
  const PN_float32 *pi = (const PN_float32 *)pointer;
  _v4.set(pi[0], pi[1], pi[2], pi[3]);
  return _v4;
}

/**
 *
 */
void GeomVertexColumn::Packer_rgba_float32_4::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  PN_float32 *pi = (PN_float32 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
  pi[2] = data[2];
  pi[3] = data[3];
}

/**
 *
 */
const LVecBase4f &GeomVertexColumn::Packer_rgba_nativefloat_4::
get_data4f(const unsigned char *pointer) {
  return *(const LVecBase4f *)pointer;
}

/**
 *
 */
int GeomVertexColumn::Packer_uint16_1::
get_data1i(const unsigned char *pointer) {
  return *(const uint16_t *)pointer;
}

/**
 *
 */
void GeomVertexColumn::Packer_uint16_1::
set_data1i(unsigned char *pointer, int data) {
  *(uint16_t *)pointer = data;
  nassertv(*(uint16_t *)pointer == data);
}
