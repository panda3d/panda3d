/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderType.I
 * @author rdb
 * @date 2019-03-12
 */

/**
 * Takes an rvalue ShaderType and returns a uniquified pointer to an equivalent
 * ShaderType object.
 */
template<class Type>
const Type *ShaderType::
register_type(Type &&type) {
  nassertr(_registered_types != nullptr, nullptr);

  Registry::iterator it = _registered_types->find(&type);
  if (it != _registered_types->end()) {
    return (const Type *)*it;
  }

  Type *new_type = new Type(std::move(type));
  _registered_types->insert(new_type);
  return new_type;
}

/**
 * Provides an arbitrary ordering among all unique shader types, so we can
 * store the essentially different ones in a big set and throw away the rest.
 */
INLINE int ShaderType::
compare_to(const ShaderType &other) const {
  TypeHandle this_type = get_type();
  TypeHandle other_type = other.get_type();
  if (this_type == other_type) {
    return compare_to_impl(other);
  }
  return (this_type > other_type) - (this_type < other_type);
}

/**
 * Constructs a scalar type.
 */
INLINE ShaderType::Scalar::
Scalar(ScalarType scalar_type) : _scalar_type(scalar_type) {
}

/**
 * Returns the base numeric type.
 */
INLINE ShaderType::ScalarType ShaderType::Scalar::
get_scalar_type() const {
  return _scalar_type;
}

/**
 * Constructs a vector from a scalar type and number of elements.
 */
INLINE ShaderType::Vector::
Vector(ScalarType scalar_type, uint32_t num_components) :
  _scalar_type(scalar_type),
  _num_components(num_components) {
}

/**
 * Returns the scalar type that this vector consists of.
 */
INLINE ShaderType::ScalarType ShaderType::Vector::
get_scalar_type() const {
  return _scalar_type;
}

/**
 * Returns the number of components in this vector.
 */
INLINE uint32_t ShaderType::Vector::
get_num_components() const {
  return _num_components;
}

/**
 * Constructs a matrix type from a scalar type, a number of rows and columns.
 */
INLINE ShaderType::Matrix::
Matrix(ScalarType scalar_type, uint32_t num_rows, uint32_t num_columns) :
  _scalar_type(scalar_type),
  _num_rows(num_rows),
  _num_columns(num_columns) {
}

/**
 * Returns the scalar type that this matrix consists of.
 */
INLINE ShaderType::ScalarType ShaderType::Matrix::
get_scalar_type() const {
  return _scalar_type;
}

/**
 * Returns the number of rows in this matrix type.
 */
INLINE uint32_t ShaderType::Matrix::
get_num_rows() const {
  return _num_rows;
}

/**
 * Returns the number of columns in this matrix type.
 */
INLINE uint32_t ShaderType::Matrix::
get_num_columns() const {
  return _num_columns;
}

/**
 * Returns the number of members in this struct.
 */
INLINE size_t ShaderType::Struct::
get_num_members() const {
  return _members.size();
}

/**
 * Returns the nth member of this struct.
 */
INLINE const ShaderType::Struct::Member &ShaderType::Struct::
get_member(size_t i) const {
  return _members[i];
}

/**
 * Constructs an array type from a base type and number of elements.
 */
INLINE ShaderType::Array::
Array(const ShaderType *element_type, uint32_t num_elements) :
  _element_type(element_type),
  _num_elements(num_elements) {
}

/**
 * Returns the type of elements in this array.
 */
INLINE const ShaderType *ShaderType::Array::
get_element_type() const {
  return _element_type;
}

/**
 * Returns the number of elements in this array.
 */
INLINE uint32_t ShaderType::Array::
get_num_elements() const {
  return _num_elements;
}

/**
 * Constructs an image type.
 */
INLINE ShaderType::Image::
Image(Texture::TextureType type, ScalarType sampled_type, Access access) :
  _texture_type(type),
  _sampled_type(sampled_type),
  _access(access) {
}

/**
 * Returns the type of texture this image samples.
 */
INLINE Texture::TextureType ShaderType::Image::
get_texture_type() const {
  return _texture_type;
}

/**
 * Returns the type of scalar that sampling this image produces.
 */
INLINE ShaderType::ScalarType ShaderType::Image::
get_sampled_type() const {
  return _sampled_type;
}

/**
 * Returns the way this image is accessed.
 */
INLINE ShaderType::Image::Access ShaderType::Image::
get_access() const {
  return _access;
}

/**
 * Returns true if this image input is writable.
 */
INLINE bool ShaderType::Image::
is_writable() const {
  return ((int)_access & (int)Access::write_only) != 0;
}

/**
 * Constructs a sampled image type.
 */
INLINE ShaderType::SampledImage::
SampledImage(Texture::TextureType type, ScalarType sampled_type) :
  _texture_type(type),
  _sampled_type(sampled_type) {
}

/**
 * Returns the type of texture this image samples.
 */
INLINE Texture::TextureType ShaderType::SampledImage::
get_texture_type() const {
  return _texture_type;
}

/**
 * Returns the type of scalar that sampling this image produces.
 */
INLINE ShaderType::ScalarType ShaderType::SampledImage::
get_sampled_type() const {
  return _sampled_type;
}
