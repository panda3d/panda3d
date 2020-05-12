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
TypeHandle ShaderType::SampledImage::_type_handle;

const ShaderType::Scalar *ShaderType::bool_type;
const ShaderType::Scalar *ShaderType::int_type;
const ShaderType::Scalar *ShaderType::uint_type;
const ShaderType::Scalar *ShaderType::float_type;

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
  ::register_type(SampledImage::_type_handle, "ShaderType::SampledImage", _type_handle);
  //::register_type(Sampler::_type_handle, "ShaderType::Sampler", _type_handle);

  bool_type = nullptr;
  int_type = ShaderType::register_type(ShaderType::Scalar(GeomEnums::NT_int32));
  uint_type = ShaderType::register_type(ShaderType::Scalar(GeomEnums::NT_uint32));
  float_type = ShaderType::register_type(ShaderType::Scalar(GeomEnums::NT_float32));
}

#ifndef CPPPARSER
/**
 *
 */
void ShaderType::Scalar::
output(std::ostream &out) const {
  if (this == bool_type) {
    out << "bool";
  } else if (this == int_type) {
    out << "int";
  } else if (this == uint_type) {
    out << "uint";
  } else if (this == float_type) {
    out << "float";
  } else {
    out << "unknown";
  }
}

/**
 * Private implementation of compare_to, only called for types with the same
 * TypeHandle.
 */
int ShaderType::Scalar::
compare_to_impl(const ShaderType &other) const {
  const Scalar &other_scalar = (const Scalar &)other;
  return (_numeric_type > other_scalar._numeric_type)
       - (_numeric_type < other_scalar._numeric_type);
}

/**
 *
 */
void ShaderType::Vector::
output(std::ostream &out) const {
  out << *_base_type << _num_elements;
}

/**
 * Private implementation of compare_to, only called for types with the same
 * TypeHandle.
 */
int ShaderType::Vector::
compare_to_impl(const ShaderType &other) const {
  const Vector &other_vector = (const Vector &)other;
  if (_base_type != other_vector._base_type) {
    return _base_type < other_vector._base_type ? -1 : 1;
  }
  return (_num_elements > other_vector._num_elements)
       - (_num_elements < other_vector._num_elements);
}

/**
 *
 */
void ShaderType::Matrix::
output(std::ostream &out) const {
  out << *_base_type << _num_rows << "x" << _num_columns;
}

/**
 * Private implementation of compare_to, only called for types with the same
 * TypeHandle.
 */
int ShaderType::Matrix::
compare_to_impl(const ShaderType &other) const {
  const Matrix &other_matrix = (const Matrix &)other;
  if (_base_type != other_matrix._base_type) {
    return _base_type < other_matrix._base_type ? -1 : 1;
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
