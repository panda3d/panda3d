// Filename: qpgeomVertexWriter.cxx
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

#include "qpgeomVertexWriter.h"

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::set_data_type
//       Access: Published
//  Description: Sets up the writer to use the indicated data_type
//               description on the given array.
//
//               This also resets the current write vertex numbers to
//               the start vertex (the same value passed to a previous
//               call to set_vertex(), or 0 if set_vertex() was never
//               called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool qpGeomVertexWriter::
set_data_type(int array, const qpGeomVertexDataType *data_type) {
  // Delete the old writer, if we've got one.
  if (_writer != (Writer *)NULL) {
    delete _writer;
    _writer = NULL;
  }

  if (array < 0 || array >= _vertex_data->get_num_arrays() || 
      data_type == (qpGeomVertexDataType *)NULL) {
    // Clear the data type.
    _array = -1;
    _data_type = NULL;
    _stride = 0;
    _write_vertex = _start_vertex;
    _num_vertices = 0;

    return false;

  } else {
    _array = array;
    _data_type = data_type;
    _stride = _vertex_data->get_format()->get_array(_array)->get_stride();

    set_pointer(_start_vertex);

    // Now set up a new writer.
    _writer = make_writer();
    _writer->_data_type = _data_type;

    return true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::make_writer
//       Access: Private
//  Description: Returns a newly-allocated Writer object suitable for
//               writing the current data type.
////////////////////////////////////////////////////////////////////
qpGeomVertexWriter::Writer *qpGeomVertexWriter::
make_writer() const {
  switch (_data_type->get_contents()) {
  case qpGeomVertexDataType::C_point:
  case qpGeomVertexDataType::C_texcoord:
    // These types are written as a 4-d homogeneous point.
    switch (_data_type->get_numeric_type()) {
    case qpGeomVertexDataType::NT_float32:
      switch (_data_type->get_num_components()) {
      case 2:
        return new Writer_point_float32_2;
      case 3:
        return new Writer_point_float32_3;
      case 4:
        return new Writer_point_float32_4;
        
      default:
        break;
      }
      break;
    default:
      break;
    }
    return new Writer_point;

  case qpGeomVertexDataType::C_rgba:
    switch (_data_type->get_numeric_type()) {
    case qpGeomVertexDataType::NT_uint8:
      switch (_data_type->get_num_components()) {
      case 4:
        return new Writer_rgba_uint8_4;
        
      default:
        break;
      }
      break;
    case qpGeomVertexDataType::NT_float32:
      switch (_data_type->get_num_components()) {
      case 4:
        return new Writer_rgba_float32_4;
        
      default:
        break;
      }
      break;
    default:
      break;
    }
    return new Writer_color;

  case qpGeomVertexDataType::C_argb:
    switch (_data_type->get_numeric_type()) {
    case qpGeomVertexDataType::NT_packed_8888:
      switch (_data_type->get_num_components()) {
      case 1:
        return new Writer_argb_packed_8888;
        
      default:
        break;
      }
      break;
    default:
      break;
    }
    return new Writer_color;

  default:
    // Otherwise, we just write it as a generic value.
    switch (_data_type->get_numeric_type()) {
    case qpGeomVertexDataType::NT_float32:
      switch (_data_type->get_num_components()) {
      case 3:
        return new Writer_float32_3;
        
      default:
        break;
      }
      break;
    default:
      break;
    }
    return new Writer;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexWriter::Writer::
~Writer() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer::set_data1f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer::
set_data1f(unsigned char *pointer, float data) {
  switch (_data_type->get_num_values()) {
  case 1:
    switch (_data_type->get_numeric_type()) {
    case qpGeomVertexDataType::NT_uint8:
      *pointer = maybe_scale_color(data);
      break;
      
    case qpGeomVertexDataType::NT_uint16:
      *(PN_uint16 *)pointer = (unsigned int)data;
      break;
      
    case qpGeomVertexDataType::NT_packed_8888:
      {
        packed_8888 dword;
        dword._i = 0;
        if (_data_type->get_contents() == qpGeomVertexDataType::C_argb) {
          dword._b[1] = maybe_scale_color(data);
        } else {
          dword._b[0] = maybe_scale_color(data);
        }
        *(PN_uint32 *)pointer = dword._i;
      }
      break;
      
    case qpGeomVertexDataType::NT_float32:
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
//     Function: qpGeomVertexWriter::Writer::set_data2f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer::
set_data2f(unsigned char *pointer, const LVecBase2f &data) {
  switch (_data_type->get_num_values()) {
  case 1:
    set_data1f(pointer, data[0]);

  case 2:
    switch (_data_type->get_numeric_type()) {
    case qpGeomVertexDataType::NT_uint8:
      maybe_scale_color(data);
      pointer[0] = _a;
      pointer[1] = _b;
      break;
      
    case qpGeomVertexDataType::NT_uint16:
      {
        PN_uint16 *pi = (PN_uint16 *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
      }
      break;
      
    case qpGeomVertexDataType::NT_packed_8888:
      {
        packed_8888 dword;
        maybe_scale_color(data);
        dword._i = 0;
        if (_data_type->get_contents() == qpGeomVertexDataType::C_argb) {
          dword._b[1] = _a;
          dword._b[2] = _b;
        } else {
          dword._b[0] = _a;
          dword._b[1] = _b;
        }
        *(PN_uint32 *)pointer = dword._i;
      }
      break;
      
    case qpGeomVertexDataType::NT_float32:
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
//     Function: qpGeomVertexWriter::Writer::set_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer::
set_data3f(unsigned char *pointer, const LVecBase3f &data) {
  switch (_data_type->get_num_values()) {
  case 1:
    set_data1f(pointer, data[0]);
    break;

  case 2:
    set_data2f(pointer, LVecBase2f(data[0], data[1]));
    break;
    
  case 3:
    switch (_data_type->get_numeric_type()) {
    case qpGeomVertexDataType::NT_uint8:
      maybe_scale_color(data);
      pointer[0] = _a;
      pointer[1] = _b;
      pointer[2] = _c;
      break;
      
    case qpGeomVertexDataType::NT_uint16:
      {
        PN_uint16 *pi = (PN_uint16 *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
      }
      break;
      
    case qpGeomVertexDataType::NT_packed_8888:
      {
        packed_8888 dword;
        maybe_scale_color(data);
        if (_data_type->get_contents() == qpGeomVertexDataType::C_argb) {
          dword._b[0] = 0;
          dword._b[1] = _a;
          dword._b[2] = _b;
          dword._b[3] = _c;
        } else {
          dword._b[0] = _a;
          dword._b[1] = _b;
          dword._b[2] = _c;
          dword._b[3] = 0;
        }
        *(PN_uint32 *)pointer = dword._i;
      }
      break;
      
    case qpGeomVertexDataType::NT_float32:
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
//     Function: qpGeomVertexWriter::Writer::set_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  switch (_data_type->get_num_values()) {
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
    switch (_data_type->get_numeric_type()) {
    case qpGeomVertexDataType::NT_uint8:
      maybe_scale_color(data);
      pointer[0] = _a;
      pointer[1] = _b;
      pointer[2] = _c;
      pointer[3] = _d;
      break;

    case qpGeomVertexDataType::NT_uint16:
      {
        PN_uint16 *pi = (PN_uint16 *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;
      
    case qpGeomVertexDataType::NT_packed_8888:
      {
        packed_8888 dword;
        maybe_scale_color(data);
        if (_data_type->get_contents() == qpGeomVertexDataType::C_argb) {
          dword._b[0] = _d;
          dword._b[1] = _a;
          dword._b[2] = _b;
          dword._b[3] = _c;
        } else {
          dword._b[0] = _a;
          dword._b[1] = _b;
          dword._b[2] = _c;
          dword._b[3] = _d;
        }
        *(PN_uint32 *)pointer = dword._i;
      }
      break;
      
    case qpGeomVertexDataType::NT_float32:
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
//     Function: qpGeomVertexWriter::Writer::set_data1i
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer::
set_data1i(unsigned char *pointer, int data) {
  switch (_data_type->get_numeric_type()) {
  case qpGeomVertexDataType::NT_uint8:
    *pointer = data;
    break;

  case qpGeomVertexDataType::NT_uint16:
    *(PN_uint16 *)pointer = data;
    break;

  case qpGeomVertexDataType::NT_packed_8888:
    {
      packed_8888 dword;
      dword._i = 0;
      if (_data_type->get_contents() == qpGeomVertexDataType::C_argb) {
        dword._b[1] = data;
      } else {
        dword._b[0] = data;
      }
      *(PN_uint32 *)pointer = dword._i;
    }
    break;

  case qpGeomVertexDataType::NT_float32:
    *(PN_float32 *)pointer = (float)data;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer_point::set_data1f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer_point::
set_data1f(unsigned char *pointer, float data) {
  if (_data_type->get_num_values() == 4) {
    set_data4f(pointer, LVecBase4f(data, 0.0f, 0.0f, 1.0f));
  } else {
    Writer::set_data1f(pointer, data);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer_point::set_data2f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer_point::
set_data2f(unsigned char *pointer, const LVecBase2f &data) {
  if (_data_type->get_num_values() == 4) {
    set_data4f(pointer, LVecBase4f(data[0], data[1], 0.0f, 1.0f));
  } else {
    Writer::set_data2f(pointer, data);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer_point::set_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer_point::
set_data3f(unsigned char *pointer, const LVecBase3f &data) {
  if (_data_type->get_num_values() == 4) {
    set_data4f(pointer, LVecBase4f(data[0], data[1], data[2], 1.0f));
  } else {
    Writer::set_data3f(pointer, data);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer_point::set_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer_point::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  switch (_data_type->get_num_values()) {
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
    switch (_data_type->get_numeric_type()) {
    case qpGeomVertexDataType::NT_uint8:
      maybe_scale_color(data);
      pointer[0] = _a;
      pointer[1] = _b;
      pointer[2] = _c;
      pointer[3] = _d;
      break;

    case qpGeomVertexDataType::NT_uint16:
      {
        PN_uint16 *pi = (PN_uint16 *)pointer;
        pi[0] = (unsigned int)data[0];
        pi[1] = (unsigned int)data[1];
        pi[2] = (unsigned int)data[2];
        pi[3] = (unsigned int)data[3];
      }
      break;
      
    case qpGeomVertexDataType::NT_packed_8888:
      {
        packed_8888 dword;
        maybe_scale_color(data);
        if (_data_type->get_contents() == qpGeomVertexDataType::C_argb) {
          dword._b[0] = _d;
          dword._b[1] = _a;
          dword._b[2] = _b;
          dword._b[3] = _c;
        } else {
          dword._b[0] = _a;
          dword._b[1] = _b;
          dword._b[2] = _c;
          dword._b[3] = _d;
        }
        *(PN_uint32 *)pointer = dword._i;
      }
      break;
      
    case qpGeomVertexDataType::NT_float32:
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
//     Function: qpGeomVertexWriter::Writer_color::set_data1f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer_color::
set_data1f(unsigned char *pointer, float data) {
  if (_data_type->get_num_values() == 4) {
    set_data4f(pointer, LVecBase4f(data, 0.0f, 0.0f, 1.0f));
  } else {
    Writer::set_data1f(pointer, data);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer_color::set_data2f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer_color::
set_data2f(unsigned char *pointer, const LVecBase2f &data) {
  if (_data_type->get_num_values() == 4) {
    set_data4f(pointer, LVecBase4f(data[0], data[1], 0.0f, 1.0f));
  } else {
    Writer::set_data2f(pointer, data);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer_color::set_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer_color::
set_data3f(unsigned char *pointer, const LVecBase3f &data) {
  if (_data_type->get_num_values() == 4) {
    set_data4f(pointer, LVecBase4f(data[0], data[1], data[2], 1.0f));
  } else {
    Writer::set_data3f(pointer, data);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer_float32_3::set_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer_float32_3::
set_data3f(unsigned char *pointer, const LVecBase3f &data) {
  PN_float32 *pi = (PN_float32 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
  pi[2] = data[2];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer_point_float32_2::set_data2f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer_point_float32_2::
set_data2f(unsigned char *pointer, const LVecBase2f &data) {
  PN_float32 *pi = (PN_float32 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer_point_float32_3::set_data3f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer_point_float32_3::
set_data3f(unsigned char *pointer, const LVecBase3f &data) {
  PN_float32 *pi = (PN_float32 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
  pi[2] = data[2];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer_point_float32_4::set_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer_point_float32_4::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  PN_float32 *pi = (PN_float32 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
  pi[2] = data[2];
  pi[3] = data[3];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer_argb_packed_8888::set_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer_argb_packed_8888::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  packed_8888 dword;
  dword._b[0] = (unsigned int)(data[3] * 255.0f);
  dword._b[1] = (unsigned int)(data[0] * 255.0f);
  dword._b[2] = (unsigned int)(data[1] * 255.0f);
  dword._b[3] = (unsigned int)(data[2] * 255.0f);

  *(PN_uint32 *)pointer = dword._i;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer_rgba_uint8_4::set_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer_rgba_uint8_4::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  pointer[0] = (unsigned int)(data[0] * 255.0f);
  pointer[1] = (unsigned int)(data[1] * 255.0f);
  pointer[2] = (unsigned int)(data[2] * 255.0f);
  pointer[3] = (unsigned int)(data[3] * 255.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer_rgba_float32_4::set_data4f
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer_rgba_float32_4::
set_data4f(unsigned char *pointer, const LVecBase4f &data) {
  PN_float32 *pi = (PN_float32 *)pointer;
  pi[0] = data[0];
  pi[1] = data[1];
  pi[2] = data[2];
  pi[3] = data[3];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexWriter::Writer_uint16_1::set_data1i
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexWriter::Writer_uint16_1::
set_data1i(unsigned char *pointer, int data) {
  *(PN_uint16 *)pointer = data;
}
