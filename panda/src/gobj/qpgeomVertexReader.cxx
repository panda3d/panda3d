// Filename: qpgeomVertexReader.cxx
// Created by:  drose (25Mar05)
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

#include "qpgeomVertexReader.h"

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::set_data_type
//       Access: Published
//  Description: Sets up the reader to use the indicated data_type
//               description on the given array.
//
//               This also resets the current read vertex numbers to
//               the start vertex (the same value passed to a previous
//               call to set_vertex(), or 0 if set_vertex() was never
//               called.)
////////////////////////////////////////////////////////////////////
void qpGeomVertexReader::
set_data_type(int array, const qpGeomVertexDataType *data_type) {
  nassertv(array >= 0 && array < _vertex_data->get_num_arrays());
  nassertv(data_type != (qpGeomVertexDataType *)NULL);
  _array = array;
  _data_type = data_type;
  _stride = _vertex_data->get_format()->get_array(_array)->get_stride();

  set_pointer();

  // Delete the old reader, if we've got one.
  if (_reader != (Reader *)NULL) {
    delete _reader;
    _reader = NULL;
  }

  // Now set up a new reader.
  _reader = make_reader();
  _reader->_data_type = _data_type;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::make_reader
//       Access: Private
//  Description: Returns a newly-allocated Reader object suitable for
//               reading the current data type.
////////////////////////////////////////////////////////////////////
qpGeomVertexReader::Reader *qpGeomVertexReader::
make_reader() const {
  if (_data_type->get_contents() == qpGeomVertexDataType::C_point &&
      _data_type->get_num_values() < 4) {
    return new Reader_point;

  } else {
    switch (_data_type->get_numeric_type()) {
    case qpGeomVertexDataType::NT_float32:
      if (sizeof(float) == 4) {
        switch (_data_type->get_num_components()) {
        case 3:
          return new Reader_float_3;
          
        default:
          break;
        }
      }
      break;

    case qpGeomVertexDataType::NT_uint16:
      switch (_data_type->get_num_components()) {
      case 1:
        return new Reader_uint16_1;
        
      default:
        break;
      }

    default:
      break;
    }
  }

  // If we got here, we have to create a generic reader.
  return new Reader;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Reader::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexReader::Reader::
~Reader() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Reader::get_data1f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
float qpGeomVertexReader::Reader::
get_data1f(const unsigned char *pointer) {
  switch (_data_type->get_numeric_type()) {
  case qpGeomVertexDataType::NT_uint8:
    return maybe_scale_color(*pointer);

  case qpGeomVertexDataType::NT_uint16:
    return *(PN_uint16 *)pointer;

  case qpGeomVertexDataType::NT_packed_8888:
    {
      packed_8888 dword;
      dword._i = *(PN_uint32 *)pointer;
      return maybe_scale_color(dword._b[0]);
    }

  case qpGeomVertexDataType::NT_float32:
    return *(PN_float32 *)pointer;
  }

  return 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Reader::get_data2f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase2f &qpGeomVertexReader::Reader::
get_data2f(const unsigned char *pointer) {
  if (_data_type->get_num_values() == 1) {
    _v2.set(get_data1i(pointer), 0.0f);
    return _v2;

  } else {
    switch (_data_type->get_numeric_type()) {
    case qpGeomVertexDataType::NT_uint8:
      maybe_scale_color(pointer[0], pointer[1]);
      return _v2;
      
    case qpGeomVertexDataType::NT_uint16:
      {
        PN_uint16 *pi = (PN_uint16 *)pointer;
        _v2.set(pi[0], pi[1]);
      }
      return _v2;
      
    case qpGeomVertexDataType::NT_packed_8888:
      {
        packed_8888 dword;
        dword._i = *(PN_uint32 *)pointer;
        maybe_scale_color(dword._b[0], dword._b[1]);
      }
      return _v2;
      
    case qpGeomVertexDataType::NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        _v2.set(pi[0], pi[1]);
      }
      return _v2;
    }
  }

  return _v2;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Reader::get_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase3f &qpGeomVertexReader::Reader::
get_data3f(const unsigned char *pointer) {
  switch (_data_type->get_num_values()) {
  case 1:
    _v3.set(get_data1i(pointer), 0.0f, 0.0f);
    return _v3;

  case 2:
    {
      const LVecBase2f &v2 = get_data2f(pointer);
      _v3.set(v2[0], v2[1], 0.0f);
    }
    return _v3;

  default:
    switch (_data_type->get_numeric_type()) {
    case qpGeomVertexDataType::NT_uint8:
      maybe_scale_color(pointer[0], pointer[1], pointer[2]);
      return _v3;
      
    case qpGeomVertexDataType::NT_uint16:
      {
        PN_uint16 *pi = (PN_uint16 *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
      }
      return _v3;
      
    case qpGeomVertexDataType::NT_packed_8888:
      {
        packed_8888 dword;
        dword._i = *(PN_uint32 *)pointer;
        maybe_scale_color(dword._b[0], dword._b[1], dword._b[2]);
      }
      return _v3;
      
    case qpGeomVertexDataType::NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        _v3.set(pi[0], pi[1], pi[2]);
      }
      return _v3;
    }
  }

  return _v3;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Reader::get_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase4f &qpGeomVertexReader::Reader::
get_data4f(const unsigned char *pointer) {
  switch (_data_type->get_num_values()) {
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
    switch (_data_type->get_numeric_type()) {
    case qpGeomVertexDataType::NT_uint8:
      maybe_scale_color(pointer[0], pointer[1], pointer[2], pointer[3]);
      return _v4;
      
    case qpGeomVertexDataType::NT_uint16:
      {
        PN_uint16 *pi = (PN_uint16 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;
      
    case qpGeomVertexDataType::NT_packed_8888:
      {
        packed_8888 dword;
        dword._i = *(PN_uint32 *)pointer;
        maybe_scale_color(dword._b[0], dword._b[1], dword._b[2], dword._b[3]);
      }
      return _v4;
      
    case qpGeomVertexDataType::NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;
    }
  }

  return _v4;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Reader::get_data1i
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int qpGeomVertexReader::Reader::
get_data1i(const unsigned char *pointer) {
  switch (_data_type->get_numeric_type()) {
  case qpGeomVertexDataType::NT_uint8:
    return *pointer;

  case qpGeomVertexDataType::NT_uint16:
    return *(PN_uint16 *)pointer;

  case qpGeomVertexDataType::NT_packed_8888:
    {
      packed_8888 dword;
      dword._i = *(PN_uint32 *)pointer;
      return dword._b[0];
    }
    break;

  case qpGeomVertexDataType::NT_float32:
    return (int)*(PN_float32 *)pointer;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Reader_point::get_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase4f &qpGeomVertexReader::Reader_point::
get_data4f(const unsigned char *pointer) {
  switch (_data_type->get_num_values()) {
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
    switch (_data_type->get_numeric_type()) {
    case qpGeomVertexDataType::NT_uint8:
      maybe_scale_color(pointer[0], pointer[1], pointer[2], pointer[3]);
      return _v4;
      
    case qpGeomVertexDataType::NT_uint16:
      {
        PN_uint16 *pi = (PN_uint16 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;
      
    case qpGeomVertexDataType::NT_packed_8888:
      {
        packed_8888 dword;
        dword._i = *(PN_uint32 *)pointer;
        maybe_scale_color(dword._b[0], dword._b[1], dword._b[2], dword._b[3]);
      }
      return _v4;
      
    case qpGeomVertexDataType::NT_float32:
      {
        PN_float32 *pi = (PN_float32 *)pointer;
        _v4.set(pi[0], pi[1], pi[2], pi[3]);
      }
      return _v4;
    }
  }

  return _v4;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Reader_float_3::get_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const LVecBase3f &qpGeomVertexReader::Reader_float_3::
get_data3f(const unsigned char *pointer) {
  // We sneakily cast a float[3] array to an LVecBase3f reference,
  // making all kinds of assumptions about how an LVecBase3f is
  // stored.
  return (LVecBase3f &)(float *)pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexReader::Reader_uint16_1::get_data1i
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int qpGeomVertexReader::Reader_uint16_1::
get_data1i(const unsigned char *pointer) {
  return *(PN_uint16 *)pointer;
}
