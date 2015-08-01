// Filename: shaderInput.cxx
// Created by: jyelon (01Sep05)
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

#include "shaderInput.h"
#include "paramNodePath.h"
#include "paramTexture.h"

TypeHandle ShaderInput::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_blank
//       Access: Public, Static
//  Description: Returns a static ShaderInput object with
//               name NULL, priority zero, type INVALID, and
//               all value-fields cleared.
////////////////////////////////////////////////////////////////////
const ShaderInput *ShaderInput::
get_blank() {
  static CPT(ShaderInput) blank;
  if (blank == 0) {
    blank = new ShaderInput(NULL, 0);
  }
  return blank;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
ShaderInput::
ShaderInput(CPT_InternalName name, const NodePath &np, int priority) :
  _name(MOVE(name)),
  _type(M_nodepath),
  _priority(priority),
  _value(new ParamNodePath(np))
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
ShaderInput::
ShaderInput(CPT_InternalName name, Texture *tex, bool read, bool write, int z, int n, int priority) :
  _name(MOVE(name)),
  _type(M_texture_image),
  _priority(priority),
  _value(new ParamTextureImage(tex, read, write, z, n))
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
ShaderInput::
ShaderInput(CPT_InternalName name, Texture *tex, const SamplerState &sampler, int priority) :
  _name(MOVE(name)),
  _type(M_texture_sampler),
  _priority(priority),
  _value(new ParamTextureSampler(tex, sampler))
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_nodepath
//       Access: Published
//  Description: Warning: no error checking is done.  This *will*
//               crash if get_value_type() is not M_nodepath.
////////////////////////////////////////////////////////////////////
const NodePath &ShaderInput::
get_nodepath() const {
  return DCAST(ParamNodePath, _value)->get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_texture
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Texture *ShaderInput::
get_texture() const {
  switch (_type) {
  case M_texture_sampler:
    return DCAST(ParamTextureSampler, _value)->get_texture();

  case M_texture_image:
    return DCAST(ParamTextureImage, _value)->get_texture();

  case M_texture:
    return DCAST(Texture, _value);

  default:
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_sampler
//       Access: Published
//  Description: Warning: no error checking is done.
////////////////////////////////////////////////////////////////////
const SamplerState &ShaderInput::
get_sampler() const {
  return (_type == M_texture_sampler)
    ? DCAST(ParamTextureSampler, _value)->get_sampler()
    : get_texture()->get_default_sampler();
}

////////////////////////////////////////////////////////////////////
//     Function: ParamValue::extract_data
//       Access: Public, Virtual
//  Description: This is a convenience function for the graphics
//               backend.  It can be used for numeric types to
//               convert this value to an array of ints matching
//               the given description.  If this is not possible,
//               returns false (after which the array may or may not
//               contain meaningful data).
////////////////////////////////////////////////////////////////////
bool ShaderInput::
extract_data(PN_int32 *data, int width, size_t count) const {
  switch (_type) {
  case M_numeric:
    if (_stored_ptr._type == Shader::SPT_int) {
      int size = min(count * width, _stored_ptr._size);

      for (int i = 0; i < size; ++i) {
        data[i] = ((int *)_stored_ptr._ptr)[i];
      }
      return true;
    }
    return false;

  default:
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ParamValue::extract_data
//       Access: Public, Virtual
//  Description: This is a convenience function for the graphics
//               backend.  It can be used for numeric types to
//               convert this value to an array of floats matching
//               the given description.  If this is not possible,
//               returns false (after which the array may or may not
//               contain meaningful data).
////////////////////////////////////////////////////////////////////
bool ShaderInput::
extract_data(PN_float32 *data, int width, size_t count) const {
  int size;

  switch (_type) {
  case M_vector:
    nassertr(width > 0 && width < 4, false);
    for (int i = 0; i < width; ++i) {
      data[i] = (float)_stored_vector[i];
    }
    return (count == 1);

  case M_numeric:
    size = min(count * width, _stored_ptr._size);

    switch (_stored_ptr._type) {
    case Shader::SPT_int:
      for (int i = 0; i < size; ++i) {
        int value = ((int *)_stored_ptr._ptr)[i];
        data[i] = (float)value;
      }
      return true;

    case Shader::SPT_float:
      for (int i = 0; i < size; ++i) {
        float value = ((float *)_stored_ptr._ptr)[i];
        data[i] = (float)value;
      }
      return true;

    case Shader::SPT_double:
      for (int i = 0; i < size; ++i) {
        double value = ((double *)_stored_ptr._ptr)[i];
        data[i] = (float)value;
      }
      return true;

    default:
      return false;
    }

  default:
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ParamValue::extract_data
//       Access: Public, Virtual
//  Description: This is a convenience function for the graphics
//               backend.  It can be used for numeric types to
//               convert this value to an array of doubles matching
//               the given description.  If this is not possible,
//               returns false (after which the array may or may not
//               contain meaningful data).
////////////////////////////////////////////////////////////////////
bool ShaderInput::
extract_data(PN_float64 *data, int width, size_t count) const {
  int size;

  switch (_type) {
  case M_vector:
    nassertr(width > 0 && width < 4, false);
    for (int i = 0; i < width; ++i) {
      data[i] = (double)_stored_vector[i];
    }
    return (count == 1);

  case M_numeric:
    size = min(count * width, _stored_ptr._size);

    switch (_stored_ptr._type) {
    case Shader::SPT_int:
      for (int i = 0; i < size; ++i) {
        data[i] = (double)(((int *)_stored_ptr._ptr)[i]);
      }
      return true;

    case Shader::SPT_float:
      for (int i = 0; i < size; ++i) {
        data[i] = (double)(((float *)_stored_ptr._ptr)[i]);
      }
      return true;

    case Shader::SPT_double:
      for (int i = 0; i < size; ++i) {
        data[i] = (double)(((double *)_stored_ptr._ptr)[i]);
      }
      return true;

    default:
      return false;
    }

  default:
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::register_with_read_factory
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void ShaderInput::
register_with_read_factory() {
  // IMPLEMENT ME
}
