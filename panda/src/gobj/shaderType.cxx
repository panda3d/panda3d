/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderType.cxx
 * @author rdb
 * @date 2019-03-12
 */

#include "shaderType.h"

const char *texture_type_suffixes[] = {
  "1D", "2D", "3D", "2DArray", "Cube", "Buffer", "CubeArray", "1DArray",
};

ShaderType::Registry *ShaderType::_registered_types = nullptr;
TypeHandle ShaderType::_type_handle;
TypeHandle ShaderType::Scalar::_type_handle;
TypeHandle ShaderType::Vector::_type_handle;
TypeHandle ShaderType::Matrix::_type_handle;
TypeHandle ShaderType::Struct::_type_handle;
TypeHandle ShaderType::Array::_type_handle;
TypeHandle ShaderType::Image::_type_handle;
TypeHandle ShaderType::Sampler::_type_handle;
TypeHandle ShaderType::SampledImage::_type_handle;

const ShaderType::Scalar *ShaderType::bool_type;
const ShaderType::Scalar *ShaderType::int_type;
const ShaderType::Scalar *ShaderType::uint_type;
const ShaderType::Scalar *ShaderType::float_type;
const ShaderType::Scalar *ShaderType::double_type;
const ShaderType::Sampler *ShaderType::sampler_type;

/**
 *
 */
void ShaderType::
init_type() {
  if (_registered_types == nullptr) {
    _registered_types = new Registry;
  }

  TypedReferenceCount::init_type();
  ::register_type(_type_handle, "ShaderType",
                  TypedReferenceCount::get_class_type());

  ::register_type(Scalar::_type_handle, "ShaderType::Scalar", _type_handle);
  ::register_type(Vector::_type_handle, "ShaderType::Vector", _type_handle);
  ::register_type(Matrix::_type_handle, "ShaderType::Matrix", _type_handle);
  ::register_type(Struct::_type_handle, "ShaderType::Struct", _type_handle);
  ::register_type(Array::_type_handle, "ShaderType::Array", _type_handle);
  ::register_type(Image::_type_handle, "ShaderType::Image", _type_handle);
  ::register_type(Sampler::_type_handle, "ShaderType::Sampler", _type_handle);
  ::register_type(SampledImage::_type_handle, "ShaderType::SampledImage", _type_handle);

  bool_type = ShaderType::register_type(ShaderType::Scalar(ST_bool));
  int_type = ShaderType::register_type(ShaderType::Scalar(ST_int));
  uint_type = ShaderType::register_type(ShaderType::Scalar(ST_uint));
  float_type = ShaderType::register_type(ShaderType::Scalar(ST_float));
  double_type = ShaderType::register_type(ShaderType::Scalar(ST_double));

  sampler_type = ShaderType::register_type(ShaderType::Sampler());
}

/**
 * Outputs a string description of the ScalarType to the stream.
 */
std::ostream &operator << (std::ostream &out, ShaderType::ScalarType scalar_type) {
  static const char *names[] = {"unknown", "float", "double", "int", "uint", "bool"};
  const char *name;
  if ((size_t)scalar_type < sizeof(names) / sizeof(names[0])) {
    name = names[(size_t)scalar_type];
  } else {
    name = "**invalid**";
  }
  return out << name;
}

#ifndef CPPPARSER
/**
 * If this is an array, vector or matrix of a scalar type, extracts the
 * dimensions.
 */
bool ShaderType::Scalar::
as_scalar_type(ScalarType &type, uint32_t &num_elements,
               uint32_t &num_rows, uint32_t &num_columns) const {
  type = _scalar_type;
  num_elements = 1;
  num_rows = 1;
  num_columns = 1;
  return true;
}

/**
 *
 */
void ShaderType::Scalar::
output(std::ostream &out) const {
  out << _scalar_type;
}

/**
 * Private implementation of compare_to, only called for types with the same
 * TypeHandle.
 */
int ShaderType::Scalar::
compare_to_impl(const ShaderType &other) const {
  const Scalar &other_scalar = (const Scalar &)other;
  return (_scalar_type > other_scalar._scalar_type)
       - (_scalar_type < other_scalar._scalar_type);
}

/**
 * If this is an array, vector or matrix of a scalar type, extracts the
 * dimensions.
 */
bool ShaderType::Vector::
as_scalar_type(ScalarType &type, uint32_t &num_elements,
               uint32_t &num_rows, uint32_t &num_columns) const {
  type = _scalar_type;
  num_elements = 1;
  num_rows = 1;
  num_columns = _num_components;
  return true;
}

/**
 *
 */
void ShaderType::Vector::
output(std::ostream &out) const {
  out << _scalar_type << _num_components;
}

/**
 * Private implementation of compare_to, only called for types with the same
 * TypeHandle.
 */
int ShaderType::Vector::
compare_to_impl(const ShaderType &other) const {
  const Vector &other_vector = (const Vector &)other;
  if (_scalar_type != other_vector._scalar_type) {
    return _scalar_type < other_vector._scalar_type ? -1 : 1;
  }
  return (_num_components > other_vector._num_components)
       - (_num_components < other_vector._num_components);
}

/**
 * If this is an array, vector or matrix of a scalar type, extracts the
 * dimensions.
 */
bool ShaderType::Matrix::
as_scalar_type(ScalarType &type, uint32_t &num_elements,
               uint32_t &num_rows, uint32_t &num_columns) const {
  type = _scalar_type;
  num_elements = 1;
  num_rows = _num_rows;
  num_columns = _num_columns;
  return true;
}

/**
 *
 */
void ShaderType::Matrix::
output(std::ostream &out) const {
  out << _scalar_type << _num_rows << "x" << _num_columns;
}

/**
 * Private implementation of compare_to, only called for types with the same
 * TypeHandle.
 */
int ShaderType::Matrix::
compare_to_impl(const ShaderType &other) const {
  const Matrix &other_matrix = (const Matrix &)other;
  if (_scalar_type != other_matrix._scalar_type) {
    return _scalar_type < other_matrix._scalar_type ? -1 : 1;
  }
  if (_num_rows != other_matrix._num_rows) {
    return _num_rows < other_matrix._num_rows ? -1 : 1;
  }
  return (_num_columns > other_matrix._num_columns)
       - (_num_columns < other_matrix._num_columns);
}

/**
 *
 */
void ShaderType::Struct::
output(std::ostream &out) const {
  out << "struct { ";
  for (const Member &member : _members) {
    if (member.type != nullptr) {
      out << *member.type << ' ';
    }
    out << member.name << "; ";
  }
  out << '}';
}

/**
 * Private implementation of compare_to, only called for types with the same
 * TypeHandle.
 */
int ShaderType::Struct::
compare_to_impl(const ShaderType &other) const {
  const Struct &other_struct = (const Struct &)other;
  if (_members.size() != other_struct._members.size()) {
    return (_members.size() > other_struct._members.size())
         - (_members.size() < other_struct._members.size());
  }

  for (size_t i = 0; i < _members.size(); ++i) {
    if (_members[i].type != other_struct._members[i].type) {
      return (_members[i].type > other_struct._members[i].type)
           - (_members[i].type < other_struct._members[i].type);
    }
    if (_members[i].name != other_struct._members[i].name) {
      return (_members[i].name > other_struct._members[i].name)
           - (_members[i].name < other_struct._members[i].name);
    }
  }

  return 0;
}

/**
 * Returns the number of uniform locations taken up by uniform variables having
 * this type.
 */
int ShaderType::Struct::
get_num_parameter_locations() const {
  int total = 0;
  for (const Member &member : _members) {
    total += member.type->get_num_parameter_locations();
  }
  return total;
}

/**
 * If this is an array, vector or matrix of a scalar type, extracts the
 * dimensions.
 */
bool ShaderType::Array::
as_scalar_type(ScalarType &type, uint32_t &num_elements,
               uint32_t &num_rows, uint32_t &num_columns) const {
  if (_element_type != nullptr &&
      _element_type->as_scalar_type(type, num_elements, num_rows, num_columns) &&
      num_elements == 1) {
    num_elements = _num_elements;
    return true;
  }
  return false;
}

/**
 *
 */
void ShaderType::Array::
output(std::ostream &out) const {
  out << *_element_type << "[" << _num_elements << "]";
}

/**
 * Private implementation of compare_to, only called for types with the same
 * TypeHandle.
 */
int ShaderType::Array::
compare_to_impl(const ShaderType &other) const {
  const Array &other_array = (const Array &)other;
  if (_element_type != other_array._element_type) {
    return _element_type < other_array._element_type ? -1 : 1;
  }
  return (_num_elements > other_array._num_elements)
       - (_num_elements < other_array._num_elements);
}

/**
 * Returns the number of uniform locations taken up by uniform variables having
 * this type.
 */
int ShaderType::Array::
get_num_parameter_locations() const {
  return _element_type->get_num_parameter_locations() * _num_elements;
}

/**
 *
 */
void ShaderType::Image::
output(std::ostream &out) const {
  out << "image" << texture_type_suffixes[_texture_type];
}

/**
 * Private implementation of compare_to, only called for types with the same
 * TypeHandle.
 */
int ShaderType::Image::
compare_to_impl(const ShaderType &other) const {
  const Image &other_image = (const Image &)other;
  return (_texture_type > other_image._texture_type)
       - (_texture_type < other_image._texture_type);
}

/**
 *
 */
void ShaderType::Sampler::
output(std::ostream &out) const {
  out << "sampler";
}

/**
 * Private implementation of compare_to, only called for types with the same
 * TypeHandle.
 */
int ShaderType::Sampler::
compare_to_impl(const ShaderType &other) const {
  // All samplers are the same type.
  return true;
}

/**
 *
 */
void ShaderType::SampledImage::
output(std::ostream &out) const {
  out << "sampler" << texture_type_suffixes[_texture_type];
}

/**
 * Private implementation of compare_to, only called for types with the same
 * TypeHandle.
 */
int ShaderType::SampledImage::
compare_to_impl(const ShaderType &other) const {
  const SampledImage &other_sampled_image = (const SampledImage &)other;
  return (_texture_type > other_sampled_image._texture_type)
       - (_texture_type < other_sampled_image._texture_type);
}
#endif  // CPPPARSER
