/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderAttrib.cxx
 * @author sshodhan
 * @date 2004-07-10
 * @author fperazzi, PandaSE
 * @date 2010-04-06
 *   for set_shader_input)
 * @author weifengh, PandaSE
 * @date 2010-04-15
 *   set_shader_auto)
 */

#include "pandabase.h"
#include "shaderAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "nodePath.h"
#include "paramNodePath.h"
#include "paramTexture.h"
#include "shaderBuffer.h"

using std::ostream;
using std::ostringstream;

TypeHandle ShaderAttrib::_type_handle;
int ShaderAttrib::_attrib_slot;

/**
 * Constructs a new ShaderAttrib object that disables the use of shaders (it
 * does not clear out all shader data, however.)
 */
CPT(RenderAttrib) ShaderAttrib::
make_off() {
  static CPT(RenderAttrib) _off_attrib;
  if (_off_attrib == nullptr) {
    ShaderAttrib *attrib = new ShaderAttrib;
    attrib->_has_shader = true;
    _off_attrib = return_new(attrib);
  }
  return _off_attrib;
}

/**
 * Constructs a new ShaderAttrib object with nothing set.
 */
CPT(RenderAttrib) ShaderAttrib::
make(const Shader *shader, int priority) {
  static CPT(RenderAttrib) _null_attrib;
  if (_null_attrib == nullptr) {
    ShaderAttrib *attrib = new ShaderAttrib;
    _null_attrib = return_new(attrib);
  }

  if (shader == nullptr) {
    return _null_attrib;
  } else {
    return DCAST(ShaderAttrib, _null_attrib)->set_shader(shader, priority);
  }
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) ShaderAttrib::
make_default() {
  return return_new(new ShaderAttrib);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
set_shader(const Shader *s, int priority) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_shader = s;
  result->_shader_priority = priority;
  result->_auto_shader = false;
  result->_has_shader = true;
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
set_shader_off(int priority) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_shader = nullptr;
  result->_shader_priority = priority;
  result->_auto_shader = false;
  result->_auto_normal_on = false;
  result->_auto_glow_on = false;
  result->_auto_gloss_on = false;
  result->_auto_ramp_on = false;
  result->_auto_shadow_on = false;

  result->_has_shader = true;
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
set_shader_auto(int priority) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_shader = nullptr;
  result->_shader_priority = priority;
  result->_auto_shader = true;
  result->_has_shader = true;
  result->_auto_normal_on = true;
  result->_auto_glow_on = true;
  result->_auto_gloss_on = true;
  result->_auto_ramp_on = true;
  result->_auto_shadow_on = true;
  return return_new(result);
}

/**
 * Set auto shader with bitmask to customize use, e.g., to keep normal, glow,
 * etc., on or off
 */
CPT(RenderAttrib) ShaderAttrib::
set_shader_auto(BitMask32 shader_switch, int priority) const {

  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_shader = nullptr;
  result->_shader_priority = priority;
  result->_auto_shader = true;
  result->_has_shader = true;
  result->_auto_normal_on = shader_switch.get_bit(Shader::bit_AutoShaderNormal);
  result->_auto_glow_on = shader_switch.get_bit(Shader::bit_AutoShaderGlow);
  result->_auto_gloss_on = shader_switch.get_bit(Shader::bit_AutoShaderGloss);
  result->_auto_ramp_on = shader_switch.get_bit(Shader::bit_AutoShaderRamp);
  result->_auto_shadow_on = shader_switch.get_bit(Shader::bit_AutoShaderShadow);

  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
clear_shader() const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_shader = nullptr;
  result->_shader_priority = 0;
  result->_auto_shader = false;
  result->_has_shader = false;
  result->_auto_normal_on = false;
  result->_auto_glow_on = false;
  result->_auto_gloss_on = false;
  result->_auto_ramp_on = false;
  result->_auto_shadow_on = false;
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
set_flag(int flag, bool value) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  if (value) {
    result->_flags |= flag;
  } else {
    result->_flags &= ~flag;
  }
  result->_has_flags |= flag;
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
clear_flag(int flag) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_flags &= ~flag;
  result->_has_flags &= ~flag;
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
set_shader_input(const ShaderInput &input) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  Inputs::iterator i = result->_inputs.find(input.get_name());
  if (i == result->_inputs.end()) {
    result->_inputs.insert(Inputs::value_type(input.get_name(), input));
  } else {
    i->second = input;
  }
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
set_shader_input(ShaderInput &&input) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  Inputs::iterator i = result->_inputs.find(input.get_name());
  if (i == result->_inputs.end()) {
    result->_inputs.insert(Inputs::value_type(input.get_name(), std::move(input)));
  } else {
    i->second = std::move(input);
  }
  return return_new(result);
}

/**
 * Returns a new ShaderAttrib with the given shader inputs set.  This is a
 * more efficient way to set multiple shader inputs than calling
 * set_shader_input multiple times.
 */
CPT(RenderAttrib) ShaderAttrib::
set_shader_inputs(const pvector<ShaderInput> &inputs) const {
  ShaderAttrib *result = new ShaderAttrib(*this);

  size_t num_inputs = inputs.size();
  for (size_t i = 0; i < num_inputs; i++) {
    const ShaderInput &input = inputs[i];
    Inputs::iterator itr = result->_inputs.find(input.get_name());
    if (itr == result->_inputs.end()) {
      result->_inputs.insert(Inputs::value_type(input.get_name(), input));
    } else {
      itr->second = input;
    }
  }

  return return_new(result);
}

/**
 * Sets the geometry instance count.  Do not confuse this with instanceTo,
 * which is used for animation instancing, and has nothing to do with this.  A
 * value of 0 means not to use instancing at all.
 *
 * This value should not be set if F_hardware_instancing is also set.
 */
CPT(RenderAttrib) ShaderAttrib::
set_instance_count(int instance_count) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_instance_count = instance_count;
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
clear_shader_input(const InternalName *id) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_inputs.erase(id);
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
clear_shader_input(const std::string &id) const {
  return clear_shader_input(InternalName::make(id));
}

/**
 * Clears all the shader inputs on the attrib.
 */
CPT(RenderAttrib) ShaderAttrib::
clear_all_shader_inputs() const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_inputs.clear();
  return return_new(result);
}

/**
 * Returns the ShaderInput of the given name.  If no such name is found, this
 * function does not return NULL --- it returns the "blank" ShaderInput.
 */
const ShaderInput &ShaderAttrib::
get_shader_input(const InternalName *id) const {
  Inputs::const_iterator i = _inputs.find(id);
  if (i != _inputs.end()) {
    return (*i).second;
  } else {
    return ShaderInput::get_blank();
  }
}

/**
 * Returns the ShaderInput of the given name.  If no such name is found, this
 * function does not return NULL --- it returns the "blank" ShaderInput.
 */
const ShaderInput &ShaderAttrib::
get_shader_input(const std::string &id) const {
  return get_shader_input(InternalName::make(id));
}

/**
 * Returns the ShaderInput as a nodepath.  Assertion fails if there is none,
 * or if it is not a nodepath.
 */
NodePath ShaderAttrib::
get_shader_input_nodepath(const InternalName *id) const {
  static NodePath resfail;
  Inputs::const_iterator i = _inputs.find(id);
  if (i != _inputs.end()) {
    const ShaderInput &p = (*i).second;
    if (p.get_value_type() == ShaderInput::M_nodepath) {
      return ((const ParamNodePath *)p.get_value())->get_value();
    } else {
      ostringstream strm;
      strm << "Shader input " << id->get_name() << " is not a nodepath.\n";
      nassert_raise(strm.str());
      return resfail;
    }
  } else {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return resfail;
  }

  // Satisfy compiler.
  return resfail;
}

/**
 * Returns the ShaderInput as a vector.  Assertion fails if there is none, or
 * if it is not a vector.
 */
LVecBase4 ShaderAttrib::
get_shader_input_vector(const InternalName *id) const {
  static LVecBase4 resfail(0,0,0,0);
  Inputs::const_iterator i = _inputs.find(id);
  if (i != _inputs.end()) {
    const ShaderInput &p = (*i).second;

    if (p.get_value_type() == ShaderInput::M_vector) {
      return p.get_vector();

    } else if (p.get_value_type() == ShaderInput::M_numeric && p.get_ptr()._size <= 4) {
      const Shader::ShaderPtrData &ptr = p.get_ptr();

      switch (ptr._type) {
      case ShaderType::ST_float:
        {
          LVector4f vectorf;
          memcpy(&vectorf[0], ptr._ptr, sizeof(float) * ptr._size);
          return LCAST(PN_stdfloat, vectorf);
        }
      case ShaderType::ST_double:
        {
          LVector4d vectord;
          memcpy(&vectord[0], ptr._ptr, sizeof(double) * ptr._size);
          return LCAST(PN_stdfloat, vectord);
        }
      case ShaderType::ST_int:
        {
          LVector4i vectori;
          memcpy(&vectori[0], ptr._ptr, sizeof(int) * ptr._size);
          return LCAST(PN_stdfloat, vectori);
        }
      default:
       {
          ostringstream strm;
          strm << "Shader input " << id->get_name() << " does not contain numeric data.\n";
          nassert_raise(strm.str());
          return resfail;
        }
      }

    } else if (p.get_value_type() == ShaderInput::M_param) {
      // Temporary solution until the new param system
      TypedWritableReferenceCount *param = p.get_value();
      if (param != nullptr && param->is_of_type(ParamVecBase4::get_class_type())) {
        return ((const ParamVecBase4 *)param)->get_value();
      }
    }

    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not a vector.\n";
    nassert_raise(strm.str());
  } else {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
  }
  return resfail;
}

/**
 * Returns the ShaderInput as a ShaderPtrData struct.  Assertion fails if
 * there is none.  or if it is not a PTA(double/float)
 */
const Shader::ShaderPtrData *ShaderAttrib::
get_shader_input_ptr(const InternalName *id) const {
  Inputs::const_iterator i = _inputs.find(id);
  if (i != _inputs.end()) {
    const ShaderInput &p = (*i).second;
    if (p.get_value_type() != ShaderInput::M_numeric &&
        p.get_value_type() != ShaderInput::M_vector) {
      ostringstream strm;
      strm << "Shader input " << id->get_name() << " is not a PTA(float/double) type.\n";
      nassert_raise(strm.str());
      return nullptr;
    }
    return &(p.get_ptr());
  } else {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return nullptr;
  }
}

/**
 * Returns the ShaderInput as a ShaderPtrData struct.  Assertion fails if
 * there is none.  or if it is not a PTA(double/float)
 */
bool ShaderAttrib::
get_shader_input_ptr(const InternalName *id, Shader::ShaderPtrData &data) const {
  Inputs::const_iterator i = _inputs.find(id);
  if (i != _inputs.end()) {
    const ShaderInput &p = (*i).second;
    if (p.get_value_type() == ShaderInput::M_numeric ||
        p.get_value_type() == ShaderInput::M_vector) {

      data = p.get_ptr();
      return (data._ptr != nullptr);
    }
    if (p.get_value_type() == ShaderInput::M_param) {
      // Temporary solution until the new param system
      TypedWritableReferenceCount *param = p.get_value();
      if (param != nullptr) {
        if (param->is_of_type(ParamVecBase4f::get_class_type())) {
          data._ptr = (void *)((const ParamVecBase4f *)param)->get_value().get_data();
          data._size = 4;
          data._type = ShaderType::ST_float;
          return true;
        }
        else if (param->is_of_type(ParamVecBase4i::get_class_type())) {
          data._ptr = (void *)((const ParamVecBase4i *)param)->get_value().get_data();
          data._size = 4;
          data._type = ShaderType::ST_int;
          return true;
        }
        else if (param->is_of_type(ParamVecBase4d::get_class_type())) {
          data._ptr = (void *)((const ParamVecBase4d *)param)->get_value().get_data();
          data._size = 4;
          data._type = ShaderType::ST_double;
          return true;
        }
      }
    }
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " was given an incompatible parameter type ("
         << p.get_value_type() << ").\n";
    nassert_raise(strm.str());
    return false;
  } else {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return false;
  }
}

/**
 * Extracts the shader input data according to the given type expected by the
 * shader.  Returns the number of bytes written to "into".
 */
size_t ShaderAttrib::
get_shader_input_data(const InternalName *id, void *into,
                      const ShaderType *type, bool pad_rows) const {
  ShaderType::ScalarType scalar_type;
  uint32_t num_elements;
  uint32_t num_rows;
  uint32_t num_columns;
  if (type->as_scalar_type(scalar_type, num_elements, num_rows, num_columns)) {
    Shader::ShaderPtrData data;
    get_shader_input_data(id, into, scalar_type, num_elements, num_rows, num_columns, pad_rows, true);
    return num_elements * num_rows * (pad_rows ? 16 : num_columns * 4);
  }
  else if (const ShaderType::Array *array_type = type->as_array()) {
    size_t basename_size = id->get_basename().size();
    char *buffer = (char *)alloca(basename_size + 14);
    memcpy(buffer, id->get_basename().c_str(), basename_size);

    size_t total_size = 0;
    for (size_t i = 0; i < array_type->get_num_elements(); ++i) {
      sprintf(buffer + basename_size, "[%d]", (int)i);

      size_t size = get_shader_input_data(id->get_parent()->append(buffer), into, array_type->get_element_type(), pad_rows);
      into = (char *)into + size;
      total_size += size;
    }
    return total_size;
  }
  else if (const ShaderType::Struct *struct_type = type->as_struct()) {
    size_t total_size = 0;
    for (size_t i = 0; i < struct_type->get_num_members(); ++i) {
      const ShaderType::Struct::Member &member = struct_type->get_member(i);

      size_t size = get_shader_input_data(((InternalName *)id)->append(member.name), (char *)into + member.offset, member.type, pad_rows);
      total_size += size;
    }
    return total_size;
  }
  else {
    return 0;
  }
}

/**
 * Extracts the shader input data, converting it as necessary.  The scratch
 * pointer must be large enough to contain the data, but may or may not be
 * filled by this function (depending on whether conversion is needed), unless
 * always_copy is true.
 */
void *ShaderAttrib::
get_shader_input_data(const InternalName *id, void *scratch,
                      ShaderType::ScalarType scalar_type, int num_elements,
                      int num_rows, int num_columns, bool pad_rows,
                      bool always_copy) const {
  Shader::ShaderPtrData ptr_data;
  if (!get_shader_input_ptr(id, ptr_data)) {
    return nullptr;
  }

  int total_rows = std::min(num_elements * num_rows, (int)ptr_data._size / num_columns);
  if (total_rows == 1) {
    pad_rows = false;
  }
  switch (scalar_type) {
  case ShaderType::ST_float:
    {
      float *data = (float *)scratch;

      switch (ptr_data._type) {
      case ShaderType::ST_int:
        // Convert int data to float data.
        if (!pad_rows || num_columns == 4) {
          for (int i = 0; i < total_rows * num_columns; ++i) {
            data[i] = (float)(((int *)ptr_data._ptr)[i]);
          }
        } else {
          const int *from_data = (const int *)ptr_data._ptr;
          for (int i = 0; i < total_rows; ++i) {
            for (int c = 0; c < num_columns; ++c) {
              data[i * 4 + c] = (float)*from_data++;
            }
          }
        }
        return data;

      case ShaderType::ST_uint:
        // Convert unsigned int data to float data.
        if (!pad_rows || num_columns == 4) {
          for (int i = 0; i < total_rows * num_columns; ++i) {
            data[i] = (float)(((unsigned int *)ptr_data._ptr)[i]);
          }
        } else {
          const unsigned int *from_data = (const unsigned int *)ptr_data._ptr;
          for (int i = 0; i < total_rows; ++i) {
            for (int c = 0; c < num_columns; ++c) {
              data[i * 4 + c] = (float)*from_data++;
            }
          }
        }
        return data;

      case ShaderType::ST_double:
        // Downgrade double data to float data.
        if (!pad_rows || num_columns == 4) {
          for (int i = 0; i < total_rows * num_columns; ++i) {
            data[i] = (float)(((double *)ptr_data._ptr)[i]);
          }
        } else {
          const double *from_data = (const double *)ptr_data._ptr;
          for (int i = 0; i < total_rows; ++i) {
            for (int c = 0; c < num_columns; ++c) {
              data[i * 4 + c] = (float)*from_data++;
            }
          }
        }
        return data;

      case ShaderType::ST_float:
        if (!pad_rows || num_columns == 4) {
          // No conversion needed.
          if (always_copy) {
            memcpy(data, ptr_data._ptr, total_rows * num_columns * sizeof(float));
            return data;
          } else {
            return (float *)ptr_data._ptr;
          }
        } else {
          const float *from_data = (const float *)ptr_data._ptr;
          for (int i = 0; i < total_rows; ++i) {
            for (int c = 0; c < num_columns; ++c) {
              data[i * 4 + c] = (float)*from_data++;
            }
          }
        }
        return data;

      default:
#ifndef NDEBUG
        pgraph_cat.error()
          << "Invalid ShaderPtrData type " << (int)ptr_data._type
          << " for shader input '" << *id << "'\n";
#endif
        return nullptr;
      }

      return data;
    }
    break;

  case ShaderType::ST_int:
    if (ptr_data._type != ShaderType::ST_int &&
        ptr_data._type != ShaderType::ST_uint &&
        ptr_data._type != ShaderType::ST_bool) {
      pgraph_cat.error()
        << "Cannot pass floating-point data to integer shader input '" << *id << "'\n";
      return nullptr;
    }
    else if (always_copy) {
      memcpy(scratch, ptr_data._ptr, total_rows * num_columns * sizeof(int));
      return scratch;
    }
    else {
      return ptr_data._ptr;
    }
    break;

  case ShaderType::ST_uint:
    if (ptr_data._type != ShaderType::ST_uint &&
        ptr_data._type != ShaderType::ST_int &&
        ptr_data._type != ShaderType::ST_bool) {
      pgraph_cat.error()
        << "Cannot pass floating-point data to integer shader input '" << *id << "'\n";
      return nullptr;
    }
    else if (always_copy) {
      memcpy(scratch, ptr_data._ptr, total_rows * num_columns * sizeof(unsigned int));
      return scratch;
    }
    else {
      return ptr_data._ptr;
    }
    break;

  case ShaderType::ST_double:
    {
      double *data = (double *)scratch;

      switch (ptr_data._type) {
      case ShaderType::ST_int:
        // Convert int data to double data.
        if (!pad_rows || num_columns == 4) {
          for (int i = 0; i < total_rows * num_columns; ++i) {
            data[i] = (double)(((int *)ptr_data._ptr)[i]);
          }
        } else {
          const int *from_data = (const int *)ptr_data._ptr;
          for (int i = 0; i < total_rows; ++i) {
            for (int c = 0; c < num_columns; ++c) {
              data[i * 4 + c] = (double)*from_data++;
            }
          }
        }
        return data;

      case ShaderType::ST_uint:
        // Convert int data to double data.
        if (!pad_rows || num_columns == 4) {
          for (int i = 0; i < total_rows * num_columns; ++i) {
            data[i] = (double)(((unsigned int *)ptr_data._ptr)[i]);
          }
        } else {
          const int *from_data = (const int *)ptr_data._ptr;
          for (int i = 0; i < total_rows; ++i) {
            for (int c = 0; c < num_columns; ++c) {
              data[i * 4 + c] = (double)*from_data++;
            }
          }
        }
        return data;

      case ShaderType::ST_double:
        if (!pad_rows || num_columns == 4) {
          // No conversion needed.
          if (always_copy) {
            memcpy(data, ptr_data._ptr, total_rows * num_columns * sizeof(double));
            return data;
          } else {
            return (double *)ptr_data._ptr;
          }
        } else {
          const double *from_data = (const double *)ptr_data._ptr;
          for (int i = 0; i < total_rows; ++i) {
            for (int c = 0; c < num_columns; ++c) {
              data[i * 4 + c] = (double)*from_data++;
            }
          }
        }
        return data;

      case ShaderType::ST_float:
        // Upgrade float data to double data.
        if (!pad_rows || num_columns == 4) {
          for (int i = 0; i < total_rows * num_columns; ++i) {
            data[i] = (double)(((float *)ptr_data._ptr)[i]);
          }
        } else {
          const float *from_data = (const float *)ptr_data._ptr;
          for (int i = 0; i < total_rows; ++i) {
            for (int c = 0; c < num_columns; ++c) {
              data[i * 4 + c] = (double)*from_data++;
            }
          }
        }
        return data;

      default:
  #ifndef NDEBUG
        pgraph_cat.error()
          << "Invalid ShaderPtrData type " << (int)ptr_data._type
          << " for shader input '" << *id << "'\n";
  #endif
        return nullptr;
      }

      return data;
    }
    break;

  case ShaderType::ST_bool:
    {
      unsigned int *data = (unsigned int *)scratch;

      switch (ptr_data._type) {
      case ShaderType::ST_int:
      case ShaderType::ST_uint:
      case ShaderType::ST_bool:
        if (!pad_rows || num_columns == 4) {
          // No conversion needed.
          if (always_copy) {
            memcpy(data, ptr_data._ptr, total_rows * num_columns * sizeof(unsigned int));
            return data;
          } else {
            return (unsigned int *)ptr_data._ptr;
          }
        } else {
          // Pad out rows.
          const unsigned int *from_data = (const unsigned int *)ptr_data._ptr;
          for (int i = 0; i < total_rows; ++i) {
            for (int c = 0; c < num_columns; ++c) {
              data[i * 4 + c] = (*from_data++) != 0;
            }
          }
        }
        return data;

      case ShaderType::ST_double:
        if (!pad_rows || num_columns == 4) {
          for (int i = 0; i < total_rows * num_columns; ++i) {
            data[i] = ((double *)ptr_data._ptr)[i] != 0.0;
          }
        } else {
          const double *from_data = (const double *)ptr_data._ptr;
          for (int i = 0; i < total_rows; ++i) {
            for (int c = 0; c < num_columns; ++c) {
              data[i * 4 + c] = (*from_data++) != 0.0;
            }
          }
        }
        return data;

      case ShaderType::ST_float:
        if (!pad_rows || num_columns == 4) {
          for (int i = 0; i < total_rows * num_columns; ++i) {
            data[i] = ((float *)ptr_data._ptr)[i] != 0.0f;
          }
        } else {
          const float *from_data = (const float *)ptr_data._ptr;
          for (int i = 0; i < total_rows; ++i) {
            for (int c = 0; c < num_columns; ++c) {
              data[i * 4 + c] = (*from_data++) != 0.0f;
            }
          }
        }
        return data;

      default:
        break;
      }
    }
    break;

  case ShaderType::ST_unknown:
    break;
  }

  return nullptr;
}

/**
 * Returns the ShaderInput as a texture.  Assertion fails if there is none, or
 * if it is not a texture.
 *
 * If sampler is not NULL, the sampler state to use for this texture is
 * assigned to it.
 */
Texture *ShaderAttrib::
get_shader_input_texture(const InternalName *id, SamplerState *sampler) const {
  Inputs::const_iterator i = _inputs.find(id);
  if (i != _inputs.end()) {
    const ShaderInput &p = (*i).second;
    switch (p.get_value_type()) {
    case ShaderInput::M_texture:
      {
        Texture *tex = (Texture *)p.get_value();
        if (sampler) {
          *sampler = tex->get_default_sampler();
        }
        return tex;
      }

    case ShaderInput::M_texture_sampler:
      {
        const ParamTextureSampler *param = (const ParamTextureSampler *)p.get_value();
        if (sampler) {
          *sampler = param->get_sampler();
        }
        return param->get_texture();
      }

    default:
      ostringstream strm;
      strm <<  "Shader input " << id->get_name() << " is not a texture.\n";
      nassert_raise(strm.str());
      return nullptr;
    }

  } else {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return nullptr;
  }
}

/**
 *
 */
Texture *ShaderAttrib::
get_shader_input_texture_image(const InternalName *id, ShaderType::Access &access, int &z, int &n) const {
  PT(Texture) tex;

  Inputs::const_iterator i = _inputs.find(id);
  if (i != _inputs.end()) {
    const ShaderInput &p = (*i).second;
    const ParamTextureImage *param = nullptr;

    switch (p.get_value_type()) {
    case ShaderInput::M_texture_image:
      param = (const ParamTextureImage *)p.get_param();
      tex = param->get_texture();
      z = param->get_bind_layered() ? -1 : param->get_bind_layer();
      n = param->get_bind_level();
      break;

    case ShaderInput::M_texture:
      // People find it convenient to be able to pass a texture without
      // further ado.
      tex = p.get_texture();
      access = ShaderType::Access::read_write;
      z = -1;
      n = 0;
      break;

    default:
      ostringstream strm;
      strm << "Shader input " << id->get_name() << " is not a texture.\n";
      nassert_raise(strm.str());
    }
  } else {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
  }

  return tex;
}

/**
 * Returns the ShaderInput as a matrix.  Assertion fails if there is none, or
 * if it is not a matrix or NodePath.
 */
const LMatrix4f &ShaderAttrib::
get_shader_input_matrix(const InternalName *id, LMatrix4f &matrix) const {
  Inputs::const_iterator i = _inputs.find(id);
  if (i != _inputs.end()) {
    const ShaderInput &p = (*i).second;

    if (p.get_value_type() == ShaderInput::M_nodepath) {
      const NodePath &np = p.get_nodepath();
      nassertr(!np.is_empty(), LMatrix4f::ident_mat());
      matrix = LCAST(float, np.get_transform()->get_mat());
      return matrix;

    } else if (p.get_value_type() == ShaderInput::M_numeric &&
               p.get_ptr()._size >= 16 && (p.get_ptr()._size & 15) == 0) {
      const Shader::ShaderPtrData &ptr = p.get_ptr();

      switch (ptr._type) {
        case ShaderType::ST_float: {
          memcpy(&matrix(0, 0), ptr._ptr, sizeof(float) * 16);
          return matrix;
        }
        case ShaderType::ST_double: {
          LMatrix4d matrixd;
          memcpy(&matrixd(0, 0), ptr._ptr, sizeof(double) * 16);
          matrix = LCAST(float, matrixd);
          return matrix;
        }
        default: {
          ostringstream strm;
          strm << "Shader input " << id->get_name() << " does not contain floating-point data.\n";
          nassert_raise(strm.str());
          return LMatrix4f::ident_mat();
        }
      }
    }

    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not a NodePath, LMatrix4 or PTA_LMatrix4.\n";
    nassert_raise(strm.str());
    return LMatrix4f::ident_mat();
  } else {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return LMatrix4f::ident_mat();
  }
}

/**
 * Returns the ShaderInput as a matrix.  Assertion fails if there is none, or
 * if it is not a matrix or NodePath.
 */
const LMatrix4d &ShaderAttrib::
get_shader_input_matrix(const InternalName *id, LMatrix4d &matrix) const {
  Inputs::const_iterator i = _inputs.find(id);
  if (i != _inputs.end()) {
    const ShaderInput &p = (*i).second;

    if (p.get_value_type() == ShaderInput::M_nodepath) {
      const NodePath &np = p.get_nodepath();
      nassertr(!np.is_empty(), LMatrix4d::ident_mat());
      matrix = LCAST(double, np.get_transform()->get_mat());
      return matrix;

    } else if (p.get_value_type() == ShaderInput::M_numeric &&
               p.get_ptr()._size >= 16 && (p.get_ptr()._size & 15) == 0) {
      const Shader::ShaderPtrData &ptr = p.get_ptr();

      switch (ptr._type) {
        case ShaderType::ST_float: {
          LMatrix4f matrixf;
          memcpy(&matrixf(0, 0), ptr._ptr, sizeof(float) * 16);
          matrix = LCAST(double, matrixf);
          return matrix;
        }
        case ShaderType::ST_double: {
          memcpy(&matrix(0, 0), ptr._ptr, sizeof(double) * 16);
          return matrix;
        }
        default: {
          ostringstream strm;
          strm << "Shader input " << id->get_name() << " does not contain floating-point data.\n";
          nassert_raise(strm.str());
          return LMatrix4d::ident_mat();
        }
      }
    }

    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not a NodePath, LMatrix4 or PTA_LMatrix4.\n";
    nassert_raise(strm.str());
    return LMatrix4d::ident_mat();
  } else {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return LMatrix4d::ident_mat();
  }
}

/**
 * Returns the ShaderInput as a ShaderBuffer.  Assertion fails if there is
 * none, or if it is not a ShaderBuffer.
 */
ShaderBuffer *ShaderAttrib::
get_shader_input_buffer(const InternalName *id) const {
  Inputs::const_iterator i = _inputs.find(id);
  if (i == _inputs.end()) {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return nullptr;
  } else {
    const ShaderInput &p = (*i).second;

    if (p.get_value_type() == ShaderInput::M_buffer) {
      ShaderBuffer *value;
      DCAST_INTO_R(value, p._value, nullptr);
      return value;
    }

    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not a ShaderBuffer.\n";
    nassert_raise(strm.str());
    return nullptr;
  }
}

/**
 * Returns the shader object associated with the node.  If get_override
 * returns true, but get_shader returns NULL, that means that this attribute
 * should disable the shader.
 */
const Shader *ShaderAttrib::
get_shader() const {
  return _shader;
}

/**
 *
 */
void ShaderAttrib::
output(ostream &out) const {
  out << "ShaderAttrib:";

  if (_auto_shader) {
    out << "auto";
    return;
  } else if (_has_shader) {
    if (_shader == nullptr) {
      out << "off";
    } else {
      out << _shader->get_filename().get_basename();
    }
  }

  out << "," << _inputs.size() << " inputs";
}

/**
 * Intended to be overridden by derived ShaderAttrib types to return a unique
 * number indicating whether this ShaderAttrib is equivalent to the other one.
 *
 * This should return 0 if the two ShaderAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two ShaderAttrib objects whose get_type()
 * functions return the same.
 */
int ShaderAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ShaderAttrib *that = (const ShaderAttrib *)other;

  if (this->_shader != that->_shader) {
    return (this->_shader < that->_shader) ? -1 : 1;
  }
  if (this->_shader_priority != that->_shader_priority) {
    return (this->_shader_priority < that->_shader_priority) ? -1 : 1;
  }
  if (this->_auto_shader != that->_auto_shader) {
    return (this->_auto_shader < that->_auto_shader) ? -1 : 1;
  }
  if (this->_has_shader != that->_has_shader) {
    return (this->_has_shader < that->_has_shader) ? -1 : 1;
  }
  if (this->_flags != that->_flags) {
    return (this->_flags < that->_flags) ? -1 : 1;
  }
  if (this->_has_flags != that->_has_flags) {
    return (this->_has_flags < that->_has_flags) ? -1 : 1;
  }
  if (this->_instance_count != that->_instance_count) {
    return (this->_instance_count < that->_instance_count) ? -1 : 1;
  }
  if (this->_auto_normal_on != that->_auto_normal_on) {
    return (this->_auto_normal_on < that->_auto_normal_on) ? -1 : 1;
  }
  if (this->_auto_glow_on != that->_auto_glow_on) {
    return (this->_auto_glow_on < that->_auto_glow_on) ? -1 : 1;
  }
  if (this->_auto_gloss_on != that->_auto_gloss_on) {
    return (this->_auto_gloss_on < that->_auto_gloss_on) ? -1 : 1;
  }
  if (this->_auto_ramp_on != that->_auto_ramp_on) {
    return (this->_auto_ramp_on < that->_auto_ramp_on) ? -1 : 1;
  }
  if (this->_auto_shadow_on != that->_auto_shadow_on) {
    return (this->_auto_shadow_on < that->_auto_shadow_on) ? -1 : 1;
  }

  Inputs::const_iterator i1 = this->_inputs.begin();
  Inputs::const_iterator i2 = that->_inputs.begin();
  while ((i1 != this->_inputs.end()) && (i2 != that->_inputs.end())) {
    if (i1->second != i2->second) {
      return (i1->second < i2->second) ? -1 : 1;
    }
    ++i1;
    ++i2;
  }
  if (i1 != this->_inputs.end()) {
    return 1;
  }
  if (i2 != that->_inputs.end()) {
    return -1;
  }

  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t ShaderAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = pointer_hash::add_hash(hash, _shader);
  hash = int_hash::add_hash(hash, _shader_priority);
  hash = int_hash::add_hash(hash, (int)_auto_shader);
  hash = int_hash::add_hash(hash, (int)_has_shader);
  hash = int_hash::add_hash(hash, _flags);
  hash = int_hash::add_hash(hash, _has_flags);
  hash = int_hash::add_hash(hash, _instance_count);
  hash = int_hash::add_hash(hash, (int)_auto_normal_on);
  hash = int_hash::add_hash(hash, (int)_auto_glow_on);
  hash = int_hash::add_hash(hash, (int)_auto_gloss_on);
  hash = int_hash::add_hash(hash, (int)_auto_shadow_on);

  Inputs::const_iterator ii;
  for (ii = _inputs.begin(); ii != _inputs.end(); ++ii) {
    hash = (*ii).second.add_hash(hash);
  }

  return hash;
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
compose_impl(const RenderAttrib *other) const {
  ShaderAttrib *attr = new ShaderAttrib(*this);
  const ShaderAttrib *over = (const ShaderAttrib *)other;

  // Update the shader portion.
  if (over->_has_shader) {
    if ((attr->_has_shader == false) ||
        (over->_shader_priority >= attr->_shader_priority)) {
      attr->_shader = over->_shader;
      attr->_shader_priority = over->_shader_priority;
      attr->_auto_shader = over->_auto_shader;
      attr->_has_shader = over->_has_shader;
      attr->_auto_normal_on = over->_auto_normal_on;
      attr->_auto_glow_on = over->_auto_glow_on;
      attr->_auto_gloss_on = over->_auto_gloss_on;
      attr->_auto_ramp_on = over->_auto_ramp_on;
      attr->_auto_shadow_on = over->_auto_shadow_on;
    }
  }
  // Update the shader-data portion.
  Inputs::const_iterator iover;
  for (iover=over->_inputs.begin(); iover!=over->_inputs.end(); ++iover) {
    const InternalName *id = (*iover).first;
    const ShaderInput &dover = (*iover).second;
    Inputs::iterator iattr = attr->_inputs.find(id);
    if (iattr == attr->_inputs.end()) {
      attr->_inputs.insert(Inputs::value_type(id,dover));
    } else {
      const ShaderInput &dattr = (*iattr).second;
      if (dattr.get_priority() <= dover.get_priority()) {
        iattr->second = iover->second;
      }
    }
  }

  // In case no instance count is set, just copy it.
  if (attr->_instance_count == 0) {
    attr->_instance_count = over->_instance_count;
  } else {
    // If an instance count is set, check if the other attrib has an instance
    // count set, if so, override it, otherwise just keep the current instance
    // count
    if (over->_instance_count > 0) {
      attr->_instance_count = over->_instance_count;
    }
  }

  // Update the flags.
  attr->_flags &= ~(over->_has_flags);
  attr->_flags |= over->_flags;
  attr->_has_flags |= (over->_has_flags);
  return return_new(attr);
}

/**
 * Tells the BamReader how to create objects of type ShaderAttrib.
 */
void ShaderAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  manager->write_pointer(dg, _shader);

  dg.add_int32(_shader_priority);
  dg.add_bool(_auto_shader);
  dg.add_bool(_has_shader);
  dg.add_int32(_flags);
  dg.add_int32(_has_flags);
  dg.add_int32(_instance_count);
  dg.add_uint32(0);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int ShaderAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);
  _shader = DCAST(Shader, p_list[pi++]);
  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ShaderAttrib is encountered in the Bam file.  It should create the
 * ShaderAttrib and extract its information from the file.
 */
TypedWritable *ShaderAttrib::
make_from_bam(const FactoryParams &params) {
  ShaderAttrib *attrib = new ShaderAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ShaderAttrib.
 */
void ShaderAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  manager->read_pointer(scan);
  _shader_priority = scan.get_int32();
  _auto_shader = scan.get_bool();
  _has_shader = scan.get_bool();
  _flags = scan.get_int32();
  _has_flags = scan.get_int32();
  _instance_count = scan.get_int32();
  scan.get_uint32();
}
