// Filename: qpgeomVertexColumn.cxx
// Created by:  drose (06Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "qpgeomVertexColumn.h"
#include "bamReader.h"
#include "bamWriter.h"

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::
operator = (const qpGeomVertexColumn &copy) {
  _name = copy._name;
  _num_components = copy._num_components;
  _numeric_type = copy._numeric_type;
  _contents = copy._contents;
  _start = copy._start;

  delete _packer;
  _packer = NULL;

  setup();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::
output(ostream &out) const {
  out << *get_name() << "(" << get_num_components() << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::setup
//       Access: Private
//  Description: Called once at construction time (or at bam-reading
//               time) to initialize the internal dependent values.
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::
setup() {
  nassertv(_num_components > 0 && _start >= 0);

  _num_values = _num_components;

  switch (_numeric_type) {
  case NT_uint16:
    _component_bytes = 2;  // sizeof(PN_uint16)
    break;

  case NT_uint32:
    _component_bytes = 4;  // sizeof(PN_uint32)
    break;

  case NT_uint8:
    _component_bytes = 1;
    break;

  case NT_packed_dcba:
  case NT_packed_dabc:
    _component_bytes = 4;  // sizeof(PN_uint32)
    _num_values *= 4;
    break;

  case NT_float32:
    _component_bytes = 4;  // sizeof(PN_float32)
    break;
  }

  _total_bytes = _component_bytes * _num_components;

  _packer = make_packer();
  _packer->_column = this;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::make_packer
//       Access: Private
//  Description: Returns a newly-allocated Packer object suitable for
//               packing and unpacking this column.  The _column
//               member of the packer is not filled in.
////////////////////////////////////////////////////////////////////
qpGeomVertexColumn::Packer *qpGeomVertexColumn::
make_packer() const {
  switch (get_contents()) {
  case C_point:
  case C_clip_point:
  case C_texcoord:
    // These types are read as a 4-d homogeneous point.
    switch (get_numeric_type()) {
    case NT_float32:
      if (sizeof(float) == sizeof(PN_float32)) {
        // Use the native float type implementation for a tiny bit
        // more optimization.
        switch (get_num_components()) {
        case 2:
          return new Packer_point_nativefloat_2;
        case 3:
          return new Packer_point_nativefloat_3;
        case 4:
          return new Packer_point_nativefloat_4;
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
    default:
      break;
    }
    return new Packer_point;

  case C_color:
    switch (get_numeric_type()) {
    case NT_uint8:
      switch (get_num_components()) {
      case 4:
        return new Packer_rgba_uint8_4;
        
      default:
        break;
      }
      break;
    case NT_packed_dabc:
      switch (get_num_components()) {
      case 1:
        return new Packer_argb_packed;
        
      default:
        break;
      }
      break;
    case NT_float32:
      switch (get_num_components()) {
      case 4:
        if (sizeof(float) == sizeof(PN_float32)) {
          // Use the native float type implementation for a tiny bit
          // more optimization.
          return new Packer_rgba_nativefloat_4;
        } else {
          return new Packer_rgba_float32_4;
        }
        
      default:
        break;
      }
      break;
    default:
      break;
    }
    return new Packer_color;

  default:
    // Otherwise, we just read it as a generic value.
    switch (get_numeric_type()) {
    case NT_float32:
      switch (get_num_components()) {
      case 3:
        if (sizeof(float) == sizeof(PN_float32)) {
          // Use the native float type implementation for a tiny bit
          // more optimization.
          return new Packer_nativefloat_3;
        } else {
          return new Packer_float32_3;
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

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::write_datagram
//       Access: Public
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::
write_datagram(BamWriter *manager, Datagram &dg) {
  manager->write_pointer(dg, _name);
  dg.add_uint8(_num_components);
  dg.add_uint8(_numeric_type);
  dg.add_uint8(_contents);
  dg.add_uint16(_start);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::complete_pointers
//       Access: Public
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int qpGeomVertexColumn::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = 0;

  _name = DCAST(InternalName, p_list[pi++]);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpGeomVertexColumn.
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::
fillin(DatagramIterator &scan, BamReader *manager) {
  manager->read_pointer(scan);

  _num_components = scan.get_uint8();
  _numeric_type = (NumericType)scan.get_uint8();
  _contents = (Contents)scan.get_uint8();
  _start = scan.get_uint16();

  setup();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexColumn::Packer::
~Packer() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::get_data1f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
float qpGeomVertexColumn::Packer::
get_data1f(const unsigned char *pointer) {
  switch (_column->get_numeric_type()) {
  case NT_uint8:
    return maybe_scale_color(*pointer);

  case NT_uint16:
    return *(const PN_uint16 *)pointer;

  case NT_uint32:
    return *(const PN_uint32 *)pointer;

  case NT_packed_dcba:
    {
      PN_uint32 dword = *(const PN_uint32 *)pointer;
      return maybe_scale_color(qpGeomVertexData::unpack_abcd_d(dword));
    }

  case NT_packed_dabc:
    {
      PN_uint32 dword = *(const PN_uint32 *)pointer;
      return maybe_scale_color(qpGeomVertexData::unpack_abcd_b(dword));
    }

  case NT_float32:
    return *(const PN_float32 *)pointer;
  }

  return 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::get_data2f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase2f &qpGeomVertexColumn::Packer::
get_data2f(const unsigned char *pointer) {
  if (_column->get_num_values() == 1) {
    _v2.set(get_data1i(pointer), 0.0f);
    return _v2;

  } else {
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      maybe_scale_color(pointer[0], pointer[1]);
      return _v2;
      
    case NT_uint16:
      {
        const PN_uint16 *pi = (const PN_uint16 *)pointer;
        _v2.set(pi[0], pi[1]);
      }
      return _v2;
      
    case NT_uint32:
      {
        const PN_uint32 *pi = (const PN_uint32 *)pointer;
        _v2.set(pi[0], pi[1]);
      }
      return _v2;
      
    case NT_packed_dcba:
      {
        PN_uint32 dword = *(const PN_uint32 *)pointer;
        maybe_scale_color(qpGeomVertexData::unpack_abcd_d(dword),
                          qpGeomVertexData::unpack_abcd_c(dword));
      }
      return _v2;
      
    case NT_packed_dabc:
      {
        PN_uint32 dword = *(const PN_uint32 *)pointer;
        maybe_scale_color(qpGeomVertexData::unpack_abcd_b(dword),
                          qpGeomVertexData::unpack_abcd_c(dword));
      }
      return _v2;
      
    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v2.set(pi[0], pi[1]);
      }
      return _v2;
    }
  }

  return _v2;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::get_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase3f &qpGeomVertexColumn::Packer::
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
      maybe_scale_color(pointer[0], pointer[1], pointer[2]);
      return _v3;
      
    case NT_uint16:
      {
        const PN_uint16 *pi = (const PN_uint16 *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
      }
      return _v3;
      
    case NT_uint32:
      {
        const PN_uint32 *pi = (const PN_uint32 *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
      }
      return _v3;
      
    case NT_packed_dcba:
      {
        PN_uint32 dword = *(const PN_uint32 *)pointer;
        maybe_scale_color(qpGeomVertexData::unpack_abcd_d(dword),
                          qpGeomVertexData::unpack_abcd_c(dword),
                          qpGeomVertexData::unpack_abcd_b(dword));
      }
      return _v3;
      
    case NT_packed_dabc:
      {
        PN_uint32 dword = *(const PN_uint32 *)pointer;
        maybe_scale_color(qpGeomVertexData::unpack_abcd_b(dword),
                          qpGeomVertexData::unpack_abcd_c(dword),
                          qpGeomVertexData::unpack_abcd_d(dword));
      }
      return _v3;
      
    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
      }
      return _v3;
    }
  }

  return _v3;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::get_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase4f &qpGeomVertexColumn::Packer::
get_data4f(const unsigned char *pointer) {
  switch (_column->get_num_values()) {
  case 1:
    _v4.set(get_data1i(pointer), 0.0f, 0.0f, 0.0f);
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
      maybe_scale_color(pointer[0], pointer[1], pointer[2], pointer[3]);
      return _v4;
      
    case NT_uint16:
      {
        const PN_uint16 *pi = (const PN_uint16 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;
      
    case NT_uint32:
      {
        const PN_uint32 *pi = (const PN_uint32 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;
      
    case NT_packed_dcba:
      {
        PN_uint32 dword = *(const PN_uint32 *)pointer;
        maybe_scale_color(qpGeomVertexData::unpack_abcd_d(dword),
                          qpGeomVertexData::unpack_abcd_c(dword),
                          qpGeomVertexData::unpack_abcd_b(dword),
                          qpGeomVertexData::unpack_abcd_a(dword));
      }
      return _v4;
      
    case NT_packed_dabc:
      {
        PN_uint32 dword = *(const PN_uint32 *)pointer;
        maybe_scale_color(qpGeomVertexData::unpack_abcd_b(dword),
                          qpGeomVertexData::unpack_abcd_c(dword),
                          qpGeomVertexData::unpack_abcd_d(dword),
                          qpGeomVertexData::unpack_abcd_a(dword));
      }
      return _v4;
      
    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;
    }
  }

  return _v4;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::get_data1i
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int qpGeomVertexColumn::Packer::
get_data1i(const unsigned char *pointer) {
  switch (_column->get_numeric_type()) {
  case NT_uint8:
    return *pointer;

  case NT_uint16:
    return *(const PN_uint16 *)pointer;

  case NT_uint32:
    return *(const PN_uint32 *)pointer;

  case NT_packed_dcba:
    {
      PN_uint32 dword = *(const PN_uint32 *)pointer;
      return qpGeomVertexData::unpack_abcd_d(dword);
    }
    break;

  case NT_packed_dabc:
    {
      PN_uint32 dword = *(const PN_uint32 *)pointer;
      return qpGeomVertexData::unpack_abcd_b(dword);
    }
    break;

  case NT_float32:
    return (int)*(const PN_float32 *)pointer;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::get_data2i
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const int *qpGeomVertexColumn::Packer::
get_data2i(const unsigned char *pointer) {
  switch (_column->get_num_values()) {
  case 1:
    _i[0] = get_data1i(pointer);
    _i[1] = 0;
    return _i;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      _i[0] = pointer[0];
      _i[1] = pointer[1];
      return _i;
      
    case NT_uint16:
      {
        const PN_uint16 *pi = (const PN_uint16 *)pointer;
        _i[0] = pi[0];
        _i[1] = pi[1];
      }
      return _i;
      
    case NT_uint32:
      {
        const PN_uint32 *pi = (const PN_uint32 *)pointer;
        _i[0] = pi[0];
        _i[1] = pi[1];
      }
      return _i;
      
    case NT_packed_dcba:
      {
        PN_uint32 dword = *(const PN_uint32 *)pointer;
        _i[0] = qpGeomVertexData::unpack_abcd_d(dword);
        _i[1] = qpGeomVertexData::unpack_abcd_c(dword);
      }
      return _i;
      
    case NT_packed_dabc:
      {
        PN_uint32 dword = *(const PN_uint32 *)pointer;
        _i[0] = qpGeomVertexData::unpack_abcd_b(dword);
        _i[1] = qpGeomVertexData::unpack_abcd_c(dword);
      }
      return _i;
      
    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _i[0] = (int)pi[0];
        _i[1] = (int)pi[1];
      }
      return _i;
    }
  }

  return _i;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::get_data3i
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const int *qpGeomVertexColumn::Packer::
get_data3i(const unsigned char *pointer) {
  switch (_column->get_num_values()) {
  case 1:
    _i[0] = get_data1i(pointer);
    _i[1] = 0;
    _i[2] = 0;
    return _i;

  case 2:
    {
      const int *i = get_data2i(pointer);
      _i[0] = i[0];
      _i[1] = i[1];
      _i[2] = 0;
    }
    return _i;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      _i[0] = pointer[0];
      _i[1] = pointer[1];
      _i[2] = pointer[2];
      return _i;
      
    case NT_uint16:
      {
        const PN_uint16 *pi = (const PN_uint16 *)pointer;
        _i[0] = pi[0];
        _i[1] = pi[1];
        _i[2] = pi[2];
      }
      return _i;
      
    case NT_uint32:
      {
        const PN_uint32 *pi = (const PN_uint32 *)pointer;
        _i[0] = pi[0];
        _i[1] = pi[1];
        _i[2] = pi[2];
      }
      return _i;
      
    case NT_packed_dcba:
      {
        PN_uint32 dword = *(const PN_uint32 *)pointer;
        _i[0] = qpGeomVertexData::unpack_abcd_d(dword);
        _i[1] = qpGeomVertexData::unpack_abcd_c(dword);
        _i[2] = qpGeomVertexData::unpack_abcd_b(dword);
      }
      return _i;
      
    case NT_packed_dabc:
      {
        PN_uint32 dword = *(const PN_uint32 *)pointer;
        _i[0] = qpGeomVertexData::unpack_abcd_b(dword);
        _i[1] = qpGeomVertexData::unpack_abcd_c(dword);
        _i[2] = qpGeomVertexData::unpack_abcd_d(dword);
      }
      return _i;
      
    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _i[0] = (int)pi[0];
        _i[1] = (int)pi[1];
        _i[2] = (int)pi[2];
      }
      return _i;
    }
  }

  return _i;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::get_data4i
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const int *qpGeomVertexColumn::Packer::
get_data4i(const unsigned char *pointer) {
  switch (_column->get_num_values()) {
  case 1:
    _i[0] = get_data1i(pointer);
    _i[1] = 0;
    _i[2] = 0;
    _i[3] = 0;
    return _i;

  case 2:
    {
      const int *i = get_data2i(pointer);
      _i[0] = i[0];
      _i[1] = i[1];
      _i[2] = 0;
      _i[3] = 0;
    }
    return _i;

  case 3:
    {
      const int *i = get_data3i(pointer);
      _i[0] = i[0];
      _i[1] = i[1];
      _i[2] = i[2];
      _i[3] = 0;
    }
    return _i;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      _i[0] = pointer[0];
      _i[1] = pointer[1];
      _i[2] = pointer[2];
      _i[3] = pointer[3];
      return _i;
      
    case NT_uint16:
      {
        const PN_uint16 *pi = (const PN_uint16 *)pointer;
        _i[0] = pi[0];
        _i[1] = pi[1];
        _i[2] = pi[2];
        _i[3] = pi[3];
      }
      return _i;
      
    case NT_uint32:
      {
        const PN_uint32 *pi = (const PN_uint32 *)pointer;
        _i[0] = pi[0];
        _i[1] = pi[1];
        _i[2] = pi[2];
        _i[3] = pi[3];
      }
      return _i;
      
    case NT_packed_dcba:
      {
        PN_uint32 dword = *(const PN_uint32 *)pointer;
        _i[0] = qpGeomVertexData::unpack_abcd_d(dword);
        _i[1] = qpGeomVertexData::unpack_abcd_c(dword);
        _i[2] = qpGeomVertexData::unpack_abcd_b(dword);
        _i[3] = qpGeomVertexData::unpack_abcd_a(dword);
      }
      return _i;
      
    case NT_packed_dabc:
      {
        PN_uint32 dword = *(const PN_uint32 *)pointer;
        _i[0] = qpGeomVertexData::unpack_abcd_b(dword);
        _i[1] = qpGeomVertexData::unpack_abcd_c(dword);
        _i[2] = qpGeomVertexData::unpack_abcd_d(dword);
        _i[3] = qpGeomVertexData::unpack_abcd_a(dword);
      }
      return _i;
      
    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _i[0] = (int)pi[0];
        _i[1] = (int)pi[1];
        _i[2] = (int)pi[2];
        _i[3] = (int)pi[3];
      }
      return _i;
    }
  }

  return _i;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::set_data1f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer::
set_data1f(unsigned char *pointer, float data) {
  switch (_column->get_num_values()) {
  case 1:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      *pointer = maybe_unscale_color(data);
      break;
      
    case NT_uint16:
      *(PN_uint16 *)pointer = (unsigned int)data;
      break;
      
    case NT_uint32:
      *(PN_uint32 *)pointer = (unsigned int)data;
      break;
      
    case NT_packed_dcba:
    case NT_packed_dabc:
      nassertv(false);
      break;
      
    case NT_float32:
      *(PN_float32 *)pointer = data;
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

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::set_data2f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer::
set_data2f(unsigned char *pointer, const LVecBase2f &data) {
  switch (_column->get_num_values()) {
  case 1:
    set_data1f(pointer, data[0]);

  case 2:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      maybe_unscale_color(data);
      pointer[0] = _a;
      pointer[1] = _b;
      break;
      
    case NT_uint16:
      {
        PN_uint16 *pi = (PN_uint16 *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
      }
      break;
      
    case NT_uint32:
      {
        PN_uint32 *pi = (PN_uint32 *)pointer;
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

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::set_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer::
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
      maybe_unscale_color(data);
      pointer[0] = _a;
      pointer[1] = _b;
      pointer[2] = _c;
      break;
      
    case NT_uint16:
      {
        PN_uint16 *pi = (PN_uint16 *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
      }
      break;
      
    case NT_uint32:
      {
        PN_uint32 *pi = (PN_uint32 *)pointer;
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
    }
    break;

  default:
    set_data4f(pointer, LVecBase4f(data[0], data[1], data[2], 0.0f));
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::set_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer::
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
      maybe_unscale_color(data);
      pointer[0] = _a;
      pointer[1] = _b;
      pointer[2] = _c;
      pointer[3] = _d;
      break;

    case NT_uint16:
      {
        PN_uint16 *pi = (PN_uint16 *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;

    case NT_uint32:
      {
        PN_uint32 *pi = (PN_uint32 *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;
      
    case NT_packed_dcba:
      maybe_unscale_color(data);
      *(PN_uint32 *)pointer = qpGeomVertexData::pack_abcd(_d, _c, _b, _a);
      break;
      
    case NT_packed_dabc:
      maybe_unscale_color(data);
      *(PN_uint32 *)pointer = qpGeomVertexData::pack_abcd(_d, _a, _b, _c);
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
    }
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::set_data1i
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer::
set_data1i(unsigned char *pointer, int a) {
  switch (_column->get_num_values()) {
  case 1:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      *pointer = a;
      nassertv((*pointer) == a);
      break;
      
    case NT_uint16:
      *(PN_uint16 *)pointer = a;
      nassertv(*(PN_uint16 *)pointer == a);
      break;
      
    case NT_uint32:
      *(PN_uint32 *)pointer = a;
      break;
      
    case NT_packed_dcba:
    case NT_packed_dabc:
      nassertv(false);
      break;
      
    case NT_float32:
      *(PN_float32 *)pointer = (float)a;
      break;
    }
    break;

  case 2:
    set_data2i(pointer, a, 0);
    break;

  case 3:
    set_data3i(pointer, a, 0, 0);
    break;

  default:
    set_data4i(pointer, a, 0, 0, 0);
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::set_data2i
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer::
set_data2i(unsigned char *pointer, int a, int b) {
  switch (_column->get_num_values()) {
  case 1:
    set_data1i(pointer, a);
    break;

  case 2:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      pointer[0] = a;
      pointer[1] = b;
      break;

    case NT_uint16:
      {
        PN_uint16 *pi = (PN_uint16 *)pointer;
        pi[0] = a;
        pi[1] = b;
      }
      break;

    case NT_uint32:
      {
        PN_uint32 *pi = (PN_uint32 *)pointer;
        pi[0] = a;
        pi[1] = b;
      }
      break;
      
    case NT_packed_dcba:
    case NT_packed_dabc:
      nassertv(false);
      break;
      
    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = a;
        pi[1] = b;
      }
      break;
    }
    break;

  case 3:
    set_data3i(pointer, a, b, 0);
    break;

  default:
    set_data4i(pointer, a, b, 0, 0);
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::set_data3i
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer::
set_data3i(unsigned char *pointer, int a, int b, int c) {
  switch (_column->get_num_values()) {
  case 1:
    set_data1i(pointer, a);
    break;

  case 2:
    set_data2i(pointer, a, b);
    break;

  case 3:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      pointer[0] = a;
      pointer[1] = b;
      pointer[2] = c;
      break;

    case NT_uint16:
      {
        PN_uint16 *pi = (PN_uint16 *)pointer;
        pi[0] = a;
        pi[1] = b;
        pi[2] = c;
      }
      break;

    case NT_uint32:
      {
        PN_uint32 *pi = (PN_uint32 *)pointer;
        pi[0] = a;
        pi[1] = b;
        pi[2] = c;
      }
      break;
      
    case NT_packed_dcba:
    case NT_packed_dabc:
      nassertv(false);
      break;
      
    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = a;
        pi[1] = b;
        pi[2] = c;
      }
      break;
    }
    break;

  default:
    set_data4i(pointer, a, b, c, 0);
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer::set_data4i
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer::
set_data4i(unsigned char *pointer, int a, int b, int c, int d) {
  switch (_column->get_num_values()) {
  case 1:
    set_data1i(pointer, a);
    break;

  case 2:
    set_data2i(pointer, a, b);
    break;

  case 3:
    set_data3i(pointer, a, b, c);
    break;

  default:
    switch (_column->get_numeric_type()) {
    case NT_uint8:
      pointer[0] = a;
      pointer[1] = b;
      pointer[2] = c;
      pointer[3] = d;
      break;

    case NT_uint16:
      {
        PN_uint16 *pi = (PN_uint16 *)pointer;
        pi[0] = a;
        pi[1] = b;
        pi[2] = c;
        pi[3] = d;
      }
      break;

    case NT_uint32:
      {
        PN_uint32 *pi = (PN_uint32 *)pointer;
        pi[0] = a;
        pi[1] = b;
        pi[2] = c;
        pi[3] = d;
      }
      break;
      
    case NT_packed_dcba:
      *(PN_uint32 *)pointer = qpGeomVertexData::pack_abcd(d, c, b, a);
      break;
      
    case NT_packed_dabc:
      *(PN_uint32 *)pointer = qpGeomVertexData::pack_abcd(d, a, b, c);
      break;
      
    case NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        pi[0] = a;
        pi[1] = b;
        pi[2] = c;
        pi[3] = d;
      }
      break;
    }
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point::get_data1f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
float qpGeomVertexColumn::Packer_point::
get_data1f(const unsigned char *pointer) {
  if (_column->get_num_values() == 4) {
    const LVecBase4f &v4 = get_data4f(pointer);
    return v4[0] / v4[3];
  } else {
    return Packer::get_data1f(pointer);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point::get_data2f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase2f &qpGeomVertexColumn::Packer_point::
get_data2f(const unsigned char *pointer) {
  if (_column->get_num_values() == 4) {
    const LVecBase4f &v4 = get_data4f(pointer);
    _v2.set(v4[0] / v4[3], v4[1] / v4[3]);
    return _v2;
  } else {
    return Packer::get_data2f(pointer);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point::get_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase3f &qpGeomVertexColumn::Packer_point::
get_data3f(const unsigned char *pointer) {
  if (_column->get_num_values() == 4) {
    const LVecBase4f &v4 = get_data4f(pointer);
    _v3.set(v4[0] / v4[3], v4[1] / v4[3], v4[2] / v4[3]);
    return _v3;
  } else {
    return Packer::get_data3f(pointer);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point::get_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase4f &qpGeomVertexColumn::Packer_point::
get_data4f(const unsigned char *pointer) {
  switch (_column->get_num_values()) {
  case 1:
    _v4.set(get_data1i(pointer), 0.0f, 0.0f, 1.0f);
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
      maybe_scale_color(pointer[0], pointer[1], pointer[2], pointer[3]);
      return _v4;
      
    case NT_uint16:
      {
        const PN_uint16 *pi = (const PN_uint16 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;
      
    case NT_uint32:
      {
        const PN_uint32 *pi = (const PN_uint32 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;
      
    case NT_packed_dcba:
      {
        PN_uint32 dword = *(const PN_uint32 *)pointer;
        maybe_scale_color(qpGeomVertexData::unpack_abcd_d(dword),
                          qpGeomVertexData::unpack_abcd_c(dword),
                          qpGeomVertexData::unpack_abcd_b(dword),
                          qpGeomVertexData::unpack_abcd_a(dword));
      }
      return _v4;
      
    case NT_packed_dabc:
      {
        PN_uint32 dword = *(const PN_uint32 *)pointer;
        maybe_scale_color(qpGeomVertexData::unpack_abcd_b(dword),
                          qpGeomVertexData::unpack_abcd_c(dword),
                          qpGeomVertexData::unpack_abcd_d(dword),
                          qpGeomVertexData::unpack_abcd_a(dword));
      }
      return _v4;
      
    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;
    }
  }

  return _v4;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point::set_data1f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer_point::
set_data1f(unsigned char *pointer, float data) {
  if (_column->get_num_values() == 4) {
    set_data4f(pointer, LVecBase4f(data, 0.0f, 0.0f, 1.0f));
  } else {
    Packer::set_data1f(pointer, data);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point::set_data2f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer_point::
set_data2f(unsigned char *pointer, const LVecBase2f &data) {
  if (_column->get_num_values() == 4) {
    set_data4f(pointer, LVecBase4f(data[0], data[1], 0.0f, 1.0f));
  } else {
    Packer::set_data2f(pointer, data);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point::set_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer_point::
set_data3f(unsigned char *pointer, const LVecBase3f &data) {
  if (_column->get_num_values() == 4) {
    set_data4f(pointer, LVecBase4f(data[0], data[1], data[2], 1.0f));
  } else {
    Packer::set_data3f(pointer, data);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point::set_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer_point::
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
      maybe_unscale_color(data);
      pointer[0] = _a;
      pointer[1] = _b;
      pointer[2] = _c;
      pointer[3] = _d;
      break;

    case NT_uint16:
      {
        PN_uint16 *pi = (PN_uint16 *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;

    case NT_uint32:
      {
        PN_uint32 *pi = (PN_uint32 *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;
      
    case NT_packed_dcba:
      maybe_unscale_color(data);
      *(PN_uint32 *)pointer = qpGeomVertexData::pack_abcd(_d, _c, _b, _a);
      break;
      
    case NT_packed_dabc:
      maybe_unscale_color(data);
      *(PN_uint32 *)pointer = qpGeomVertexData::pack_abcd(_d, _a, _b, _c);
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
    }
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_color::get_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase4f &qpGeomVertexColumn::Packer_color::
get_data4f(const unsigned char *pointer) {
  switch (_column->get_num_values()) {
  case 1:
    _v4.set(get_data1i(pointer), 0.0f, 0.0f, 1.0f);
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
      maybe_scale_color(pointer[0], pointer[1], pointer[2], pointer[3]);
      return _v4;
      
    case NT_uint16:
      {
        const PN_uint16 *pi = (const PN_uint16 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;
      
    case NT_uint32:
      {
        const PN_uint32 *pi = (const PN_uint32 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;
      
    case NT_packed_dcba:
      {
        PN_uint32 dword = *(const PN_uint32 *)pointer;
        maybe_scale_color(qpGeomVertexData::unpack_abcd_d(dword),
                          qpGeomVertexData::unpack_abcd_c(dword),
                          qpGeomVertexData::unpack_abcd_b(dword),
                          qpGeomVertexData::unpack_abcd_a(dword));
      }
      return _v4;
      
    case NT_packed_dabc:
      {
        PN_uint32 dword = *(const PN_uint32 *)pointer;
        maybe_scale_color(qpGeomVertexData::unpack_abcd_b(dword),
                          qpGeomVertexData::unpack_abcd_c(dword),
                          qpGeomVertexData::unpack_abcd_d(dword),
                          qpGeomVertexData::unpack_abcd_a(dword));
      }
      return _v4;
      
    case NT_float32:
      {
        const PN_float32 *pi = (const PN_float32 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;
    }
  }

  return _v4;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_color::set_data1f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer_color::
set_data1f(unsigned char *pointer, float data) {
  if (_column->get_num_values() == 4) {
    set_data4f(pointer, LVecBase4f(data, 0.0f, 0.0f, 1.0f));
  } else {
    Packer::set_data1f(pointer, data);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_color::set_data2f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer_color::
set_data2f(unsigned char *pointer, const LVecBase2f &data) {
  if (_column->get_num_values() == 4) {
    set_data4f(pointer, LVecBase4f(data[0], data[1], 0.0f, 1.0f));
  } else {
    Packer::set_data2f(pointer, data);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_color::set_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer_color::
set_data3f(unsigned char *pointer, const LVecBase3f &data) {
  if (_column->get_num_values() == 4) {
    set_data4f(pointer, LVecBase4f(data[0], data[1], data[2], 1.0f));
  } else {
    Packer::set_data3f(pointer, data);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_float32_3::get_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase3f &qpGeomVertexColumn::Packer_float32_3::
get_data3f(const unsigned char *pointer) {
  const PN_float32 *pi = (const PN_float32 *)pointer;
  _v3.set(pi[0], pi[1], pi[2]);
  return _v3;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_float32_3::set_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer_float32_3::
set_data3f(unsigned char *pointer, const LVecBase3f &data) {
  PN_float32 *pi = (PN_float32 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
  pi[2] = data[2];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point_float32_2::get_data2f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase2f &qpGeomVertexColumn::Packer_point_float32_2::
get_data2f(const unsigned char *pointer) {
  const PN_float32 *pi = (const PN_float32 *)pointer;
  _v2.set(pi[0], pi[1]);
  return _v2;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point_float32_2::set_data2f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer_point_float32_2::
set_data2f(unsigned char *pointer, const LVecBase2f &data) {
  PN_float32 *pi = (PN_float32 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point_float32_3::get_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase3f &qpGeomVertexColumn::Packer_point_float32_3::
get_data3f(const unsigned char *pointer) {
  const PN_float32 *pi = (const PN_float32 *)pointer;
  _v3.set(pi[0], pi[1], pi[2]);
  return _v3;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point_float32_3::set_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer_point_float32_3::
set_data3f(unsigned char *pointer, const LVecBase3f &data) {
  PN_float32 *pi = (PN_float32 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
  pi[2] = data[2];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point_float32_4::get_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase4f &qpGeomVertexColumn::Packer_point_float32_4::
get_data4f(const unsigned char *pointer) {
  const PN_float32 *pi = (const PN_float32 *)pointer;
  _v4.set(pi[0], pi[1], pi[2], pi[3]);
  return _v4;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point_float32_4::set_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer_point_float32_4::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  PN_float32 *pi = (PN_float32 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
  pi[2] = data[2];
  pi[3] = data[3];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_nativefloat_3::get_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase3f &qpGeomVertexColumn::Packer_nativefloat_3::
get_data3f(const unsigned char *pointer) {
  return *(const LVecBase3f *)pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point_nativefloat_2::get_data2f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase2f &qpGeomVertexColumn::Packer_point_nativefloat_2::
get_data2f(const unsigned char *pointer) {
  return *(const LVecBase2f *)pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point_nativefloat_3::get_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase3f &qpGeomVertexColumn::Packer_point_nativefloat_3::
get_data3f(const unsigned char *pointer) {
  return *(const LVecBase3f *)pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_point_nativefloat_4::get_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase4f &qpGeomVertexColumn::Packer_point_nativefloat_4::
get_data4f(const unsigned char *pointer) {
  return *(const LVecBase4f *)pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_argb_packed::get_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase4f &qpGeomVertexColumn::Packer_argb_packed::
get_data4f(const unsigned char *pointer) {
  PN_uint32 dword = *(const PN_uint32 *)pointer;
  _v4.set((float)qpGeomVertexData::unpack_abcd_b(dword) / 255.0f,
          (float)qpGeomVertexData::unpack_abcd_c(dword) / 255.0f,
          (float)qpGeomVertexData::unpack_abcd_d(dword) / 255.0f,
          (float)qpGeomVertexData::unpack_abcd_a(dword) / 255.0f);
  return _v4;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_argb_packed::set_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer_argb_packed::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  *(PN_uint32 *)pointer = qpGeomVertexData::pack_abcd
    ((unsigned int)(data[3] * 255.0f),
     (unsigned int)(data[0] * 255.0f),
     (unsigned int)(data[1] * 255.0f),
     (unsigned int)(data[2] * 255.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_rgba_uint8_4::get_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase4f &qpGeomVertexColumn::Packer_rgba_uint8_4::
get_data4f(const unsigned char *pointer) {
  _v4.set((float)pointer[0] / 255.0f,
          (float)pointer[1] / 255.0f,
          (float)pointer[2] / 255.0f,
          (float)pointer[3] / 255.0f);
  return _v4;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_rgba_uint8_4::set_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer_rgba_uint8_4::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  pointer[0] = (unsigned int)(data[0] * 255.0f);
  pointer[1] = (unsigned int)(data[1] * 255.0f);
  pointer[2] = (unsigned int)(data[2] * 255.0f);
  pointer[3] = (unsigned int)(data[3] * 255.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_rgba_float32_4::get_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase4f &qpGeomVertexColumn::Packer_rgba_float32_4::
get_data4f(const unsigned char *pointer) {
  const PN_float32 *pi = (const PN_float32 *)pointer;
  _v4.set(pi[0], pi[1], pi[2], pi[3]);
  return _v4;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_rgba_float32_4::set_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer_rgba_float32_4::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  PN_float32 *pi = (PN_float32 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
  pi[2] = data[2];
  pi[3] = data[3];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_rgba_nativefloat_4::get_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase4f &qpGeomVertexColumn::Packer_rgba_nativefloat_4::
get_data4f(const unsigned char *pointer) {
  return *(const LVecBase4f *)pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_uint16_1::get_data1i
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int qpGeomVertexColumn::Packer_uint16_1::
get_data1i(const unsigned char *pointer) {
  return *(const PN_uint16 *)pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Packer_uint16_1::set_data1i
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::Packer_uint16_1::
set_data1i(unsigned char *pointer, int data) {
  *(PN_uint16 *)pointer = data;
  nassertv(*(PN_uint16 *)pointer == data);
}
