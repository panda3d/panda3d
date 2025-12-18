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

static const char *texture_type_suffixes[] = {
  "1D", "2D", "3D", "2DArray", "Cube", "Buffer", "CubeArray", "1DArray",
};

static const char *texture_type_signatures[] = {
  "1", "2", "3", "A2", "C", "B", "AC", "A1",
};

static const char scalar_signatures[] = "?fdiub";
static const char access_signatures[] = "nrwx";

ShaderType::Registry *ShaderType::_registered_types = nullptr;
TypeHandle ShaderType::_type_handle;
TypeHandle ShaderType::Void::_type_handle;
TypeHandle ShaderType::Scalar::_type_handle;
TypeHandle ShaderType::Vector::_type_handle;
TypeHandle ShaderType::Matrix::_type_handle;
TypeHandle ShaderType::Struct::_type_handle;
TypeHandle ShaderType::Array::_type_handle;
TypeHandle ShaderType::Image::_type_handle;
TypeHandle ShaderType::Sampler::_type_handle;
TypeHandle ShaderType::SampledImage::_type_handle;
TypeHandle ShaderType::StorageBuffer::_type_handle;

const ShaderType::Void *ShaderType::void_type;
const ShaderType::Scalar *ShaderType::bool_type;
const ShaderType::Scalar *ShaderType::int_type;
const ShaderType::Scalar *ShaderType::uint_type;
const ShaderType::Scalar *ShaderType::float_type;
const ShaderType::Scalar *ShaderType::double_type;
const ShaderType::Sampler *ShaderType::sampler_type;

/**
 * If this type is an array, puts the element type in the first argument and the
 * number of elements in the second argument, and returns true.  If not, puts
 * the current type in the first argument, and 1 in the second argument, and
 * returns false.
 */
bool ShaderType::
unwrap_array(const ShaderType *&element_type, uint32_t &num_elements) const {
  element_type = this;
  num_elements = 1;
  return false;
}

/**
 * Returns a new type with all occurrences of the given type recursively
 * replaced with the second type.
 */
const ShaderType *ShaderType::
replace_type(const ShaderType *a, const ShaderType *b) const {
  return (this == a) ? b : this;
}

/**
 * Returns a new type that can contain both this type and the other type.
 */
const ShaderType *ShaderType::
merge(const ShaderType *other) const {
  return (this == other) ? this : nullptr;
}

/**
 *
 */
void ShaderType::
init_type() {
  if (_registered_types == nullptr) {
    _registered_types = new Registry;
  }

  TypedWritable::init_type();
  ::register_type(_type_handle, "ShaderType", TypedWritable::get_class_type());

  ::register_type(Void::_type_handle, "ShaderType::Void", _type_handle);
  ::register_type(Scalar::_type_handle, "ShaderType::Scalar", _type_handle);
  ::register_type(Vector::_type_handle, "ShaderType::Vector", _type_handle);
  ::register_type(Matrix::_type_handle, "ShaderType::Matrix", _type_handle);
  ::register_type(Struct::_type_handle, "ShaderType::Struct", _type_handle);
  ::register_type(Array::_type_handle, "ShaderType::Array", _type_handle);
  ::register_type(Image::_type_handle, "ShaderType::Image", _type_handle);
  ::register_type(Sampler::_type_handle, "ShaderType::Sampler", _type_handle);
  ::register_type(SampledImage::_type_handle, "ShaderType::SampledImage", _type_handle);
  ::register_type(StorageBuffer::_type_handle, "ShaderType::StorageBuffer", _type_handle);

  void_type = ShaderType::register_type(ShaderType::Void());
  bool_type = ShaderType::register_type(ShaderType::Scalar(ST_bool));
  int_type = ShaderType::register_type(ShaderType::Scalar(ST_int));
  uint_type = ShaderType::register_type(ShaderType::Scalar(ST_uint));
  float_type = ShaderType::register_type(ShaderType::Scalar(ST_float));
  double_type = ShaderType::register_type(ShaderType::Scalar(ST_double));

  sampler_type = ShaderType::register_type(ShaderType::Sampler());
}

/**
 * Tells the BamReader how to create objects of type ShaderType.
 */
void ShaderType::
register_with_read_factory() {
  WritableFactory *factory = BamReader::get_factory();
  factory->register_factory(Void::_type_handle, Void::make_from_bam);
  factory->register_factory(Scalar::_type_handle, Scalar::make_from_bam);
  factory->register_factory(Vector::_type_handle, Vector::make_from_bam);
  factory->register_factory(Matrix::_type_handle, Matrix::make_from_bam);
  factory->register_factory(Struct::_type_handle, Struct::make_from_bam);
  factory->register_factory(Array::_type_handle, Array::make_from_bam);
  factory->register_factory(Image::_type_handle, Image::make_from_bam);
  factory->register_factory(Sampler::_type_handle, Sampler::make_from_bam);
  factory->register_factory(SampledImage::_type_handle, SampledImage::make_from_bam);
  factory->register_factory(StorageBuffer::_type_handle, StorageBuffer::make_from_bam);
}

/**
 * Some objects require all of their nested pointers to have been completed
 * before the objects themselves can be completed.  If this is the case,
 * override this method to return true, and be careful with circular
 * references (which would make the object unreadable from a bam file).
 */
bool ShaderType::
require_fully_complete() const {
  return true;
}

/**
 * Called immediately after complete_pointers(), this gives the object a
 * chance to adjust its own pointer if desired.  Most objects don't change
 * pointers after completion, but some need to.
 *
 * Once this function has been called, the old pointer will no longer be
 * accessed.
 */
TypedWritable *ShaderType::
change_this(TypedWritable *old_ptr, BamReader *manager) {
  nassertr(_registered_types != nullptr, old_ptr);

  ShaderType *old_type = (ShaderType *)old_ptr;
  Registry::iterator it = _registered_types->find(old_type);
  if (it != _registered_types->end()) {
    delete old_type;
    return (ShaderType *)*it;
  }

  _registered_types->insert(old_type);
  return old_type;
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
 *
 */
void ShaderType::Void::
output(std::ostream &out) const {
  out << "void";
}

/**
 * Outputs a signature that compactly but uniquely identifies this type.
 */
void ShaderType::Void::
output_signature(std::ostream &out) const {
  out << 'V';
}

/**
 * Private implementation of compare_to, only called for types with the same
 * TypeHandle.
 */
int ShaderType::Void::
compare_to_impl(const ShaderType &other) const {
  return true;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ShaderType is encountered in the Bam file.  It should create the
 * ShaderType and extract its information from the file.
 */
TypedWritable *ShaderType::Void::
make_from_bam(const FactoryParams &params) {
  return (ShaderType *)ShaderType::void_type;
}

/**
 * Returns true if this type contains the given scalar type.
 */
bool ShaderType::Scalar::
contains_scalar_type(ScalarType type) const {
  return _scalar_type == type;
}

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
 * Replaces any occurrence of the given scalar type with the given other one.
 */
const ShaderType *ShaderType::Scalar::
replace_scalar_type(ScalarType a, ScalarType b) const {
  if (_scalar_type == a) {
    return ShaderType::register_type(ShaderType::Scalar(b));
  } else {
    return this;
  }
}

/**
 *
 */
void ShaderType::Scalar::
output(std::ostream &out) const {
  out << _scalar_type;
}

/**
 * Outputs a signature that compactly but uniquely identifies this type.
 */
void ShaderType::Scalar::
output_signature(std::ostream &out) const {
  out << scalar_signatures[_scalar_type];
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
 * Returns the alignment in bytes of this type in memory, if applicable.
 */
uint32_t ShaderType::Scalar::
get_align_bytes() const {
  return get_scalar_size_bytes(_scalar_type);
}

/**
 * Returns the size in bytes of this type in memory, if applicable.  Opaque
 * types will return 0.
 */
uint32_t ShaderType::Scalar::
get_size_bytes() const {
  return get_scalar_size_bytes(_scalar_type);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderType::Scalar::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_uint8(_scalar_type);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ShaderType is encountered in the Bam file.  It should create the
 * ShaderType and extract its information from the file.
 */
TypedWritable *ShaderType::Scalar::
make_from_bam(const FactoryParams &params) {
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  ScalarType scalar_type = (ScalarType)scan.get_uint8();
  return (ShaderType *)ShaderType::register_type(ShaderType::Scalar(scalar_type));
}

/**
 * Returns true if this type contains the given scalar type.
 */
bool ShaderType::Vector::
contains_scalar_type(ScalarType type) const {
  return _scalar_type == type;
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
 * Replaces any occurrence of the given scalar type with the given other one.
 */
const ShaderType *ShaderType::Vector::
replace_scalar_type(ScalarType a, ScalarType b) const {
  if (_scalar_type == a) {
    return ShaderType::register_type(ShaderType::Vector(b, _num_components));
  } else {
    return this;
  }
}

/**
 * Returns a new type that can contain both this type and the other type.
 */
const ShaderType *ShaderType::Vector::
merge(const ShaderType *other) const {
  if (this == other) {
    return this;
  }
  const ShaderType::Vector *other_vec = other->as_vector();
  if (other_vec == nullptr) {
    return nullptr;
  }

  if (_scalar_type != other_vec->_scalar_type) {
    return nullptr;
  }

  if (other_vec->_num_components > _num_components) {
    return other;
  } else {
    return this;
  }
}

/**
 * Returns the number of in/out locations taken up by in/out variables having
 * this type.
 */
int ShaderType::Vector::
get_num_interface_locations() const {
  return (get_scalar_size_bytes(_scalar_type) * _num_components + 15) / 16;
}

/**
 *
 */
void ShaderType::Vector::
output(std::ostream &out) const {
  out << _scalar_type << _num_components;
}

/**
 * Outputs a signature that compactly but uniquely identifies this type.
 */
void ShaderType::Vector::
output_signature(std::ostream &out) const {
  out << scalar_signatures[_scalar_type] << _num_components;
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
 * Returns the alignment in bytes of this type in memory, if applicable.
 */
uint32_t ShaderType::Vector::
get_align_bytes() const {
  return get_scalar_size_bytes(_scalar_type) * ((_num_components == 3) ? 4 : _num_components);
}

/**
 * Returns the size in bytes of this type in memory, if applicable.  Opaque
 * types will return 0.
 */
uint32_t ShaderType::Vector::
get_size_bytes() const {
  // Notably, a vec3 is vec4-aligned but not padded!  It is permissible for a
  // scalar to directly follow a vec3 in a struct.
  return get_scalar_size_bytes(_scalar_type) * _num_components;
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderType::Vector::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_uint8(_scalar_type);
  dg.add_uint32(_num_components);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ShaderType is encountered in the Bam file.  It should create the
 * ShaderType and extract its information from the file.
 */
TypedWritable *ShaderType::Vector::
make_from_bam(const FactoryParams &params) {
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  ScalarType scalar_type = (ScalarType)scan.get_uint8();
  uint32_t num_components = scan.get_uint32();
  return (ShaderType *)ShaderType::register_type(ShaderType::Vector(scalar_type, num_components));
}

/**
 * Returns true if this type contains the given scalar type.
 */
bool ShaderType::Matrix::
contains_scalar_type(ScalarType type) const {
  return _scalar_type == type;
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
 * Replaces any occurrence of the given scalar type with the given other one.
 */
const ShaderType *ShaderType::Matrix::
replace_scalar_type(ScalarType a, ScalarType b) const {
  if (_scalar_type == a) {
    return ShaderType::register_type(ShaderType::Matrix(b, _num_rows, _num_columns));
  } else {
    return this;
  }
}

/**
 *
 */
void ShaderType::Matrix::
output(std::ostream &out) const {
  out << _scalar_type << _num_rows << "x" << _num_columns;
}

/**
 * Outputs a signature that compactly but uniquely identifies this type.
 */
void ShaderType::Matrix::
output_signature(std::ostream &out) const {
  out << scalar_signatures[_scalar_type] << _num_rows << _num_columns;
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
 * Returns the alignment in bytes of this type in memory, if applicable.
 */
uint32_t ShaderType::Matrix::
get_align_bytes() const {
  return get_scalar_size_bytes(_scalar_type) * 4;
}

/**
 * Returns the size in bytes of this type in memory, if applicable.  Opaque
 * types will return 0.
 */
uint32_t ShaderType::Matrix::
get_size_bytes() const {
  // Pad rows to 16 bytes (std140 rules, but DX9 also expects that)
  uint32_t row_size = _num_columns * get_scalar_size_bytes(_scalar_type);
  row_size = (row_size + 15) & ~15;
  return _num_rows * row_size;
}

/**
 * Returns the number of in/out locations taken up by in/out variables having
 * this type.
 */
int ShaderType::Matrix::
get_num_interface_locations() const {
  return _num_rows;
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderType::Matrix::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_uint8(_scalar_type);
  dg.add_uint32(_num_rows);
  dg.add_uint32(_num_columns);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ShaderType is encountered in the Bam file.  It should create the
 * ShaderType and extract its information from the file.
 */
TypedWritable *ShaderType::Matrix::
make_from_bam(const FactoryParams &params) {
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  ScalarType scalar_type = (ScalarType)scan.get_uint8();
  uint32_t num_rows = scan.get_uint32();
  uint32_t num_columns = scan.get_uint32();
  return (ShaderType *)ShaderType::register_type(ShaderType::Matrix(scalar_type, num_rows, num_columns));
}

/**
 * Adds a member to this struct.
 */
void ShaderType::Struct::
add_member(const ShaderType *type, std::string name) {
  Member member;
  member.type = type;
  member.name = std::move(name);
  member.offset = _members.empty() ? 0 : _members.back().offset + _members.back().type->get_size_bytes();
  uint32_t alignment = type->get_align_bytes();
  if (alignment > 0) {
    member.offset += alignment - ((member.offset + (alignment - 1)) % alignment) - 1;
  }
  _members.push_back(std::move(member));
}

/**
 * Adds a member to this struct with a given offset.
 */
void ShaderType::Struct::
add_member(const ShaderType *type, std::string name, uint32_t offset) {
  pvector<Member>::iterator it = _members.begin();
  while (it != _members.end() && it->offset < offset) {
    ++it;
  }
  Member member;
  member.type = type;
  member.name = std::move(name);
  member.offset = offset;
  _members.insert(it, std::move(member));
}

/**
 * If a member with the given name already exists, merges the types.
 * Otherwise, simply adds it.
 */
void ShaderType::Struct::
merge_member_by_name(std::string name, const ShaderType *type) {
  bool found = false;
  uint32_t min_offset = 0;

  for (Member &member : _members) {
    if (found) {
      if (member.offset > min_offset) {
        member.offset = min_offset;
        uint32_t alignment = type->get_align_bytes();
        if (alignment > 0) {
          member.offset += alignment - ((member.offset + (alignment - 1)) % alignment) - 1;
        }
      } else {
        return;
      }
    }
    else if (member.name == name) {
      if (member.type == type) {
        return;
      }
      const ShaderType *merged_type = member.type->merge(type);
      if (merged_type == type) {
        return;
      }
      uint32_t new_size = merged_type->get_size_bytes();
      uint32_t alignment = merged_type->get_align_bytes();
      if (alignment > 0) {
        member.offset += alignment - ((member.offset + (alignment - 1)) % alignment) - 1;
      }
      member.type = merged_type;
      // Shift the rest down.
      min_offset = member.offset + new_size;
      found = true;
    }
  }

  if (!found) {
    add_member(type, name);
  }
}

/**
 * Returns true if this type is or contains any opaque type.
 */
bool ShaderType::Struct::
contains_opaque_type() const {
  for (const Member &member : _members) {
    if (member.type != nullptr && member.type->contains_opaque_type()) {
      return true;
    }
  }
  return false;
}

/**
 * Returns true if this type contains the given scalar type.
 */
bool ShaderType::Struct::
contains_scalar_type(ScalarType type) const {
  for (const Member &member : _members) {
    if (member.type != nullptr && member.type->contains_scalar_type(type)) {
      return true;
    }
  }
  return false;
}

/**
 * Replaces any occurrence of the given scalar type with the given other one.
 */
const ShaderType *ShaderType::Struct::
replace_scalar_type(ScalarType a, ScalarType b) const {
  if (contains_scalar_type(a)) {
    bool recompute_offsets = get_scalar_size_bytes(a) != get_scalar_size_bytes(b);

    ShaderType::Struct copy;
    for (const Member &member : _members) {
      const ShaderType *type = member.type->replace_scalar_type(a, b);
      if (recompute_offsets) {
        copy.add_member(type, member.name);
      } else {
        copy.add_member(type, member.name, member.offset);
      }
    }
    return ShaderType::register_type(std::move(copy));
  } else {
    return this;
  }
}

/**
 * Returns a new type with all occurrences of the given type recursively
 * replaced with the second type.
 */
const ShaderType *ShaderType::Struct::
replace_type(const ShaderType *a, const ShaderType *b) const {
  if (this == a) {
    return b;
  }
  bool any_changed = false;
  bool recompute_offsets = a->get_size_bytes() != b->get_size_bytes();
  ShaderType::Struct copy;
  for (const Member &member : _members) {
    const ShaderType *type = member.type->replace_type(a, b);
    if (type != member.type) {
      any_changed = true;
    }
    if (recompute_offsets) {
      copy.add_member(type, member.name);
    } else {
      copy.add_member(type, member.name, member.offset);
    }
  }
  if (any_changed) {
    return ShaderType::register_type(std::move(copy));
  } else {
    return this;
  }
}

/**
 * Returns a new type that can contain both this type and the other type.
 */
const ShaderType *ShaderType::Struct::
merge(const ShaderType *other) const {
  if (this == other) {
    return this;
  }
  const ShaderType::Struct *other_struct = other->as_struct();
  if (other_struct == nullptr) {
    return nullptr;
  }
  const auto &other_members = other_struct->_members;

  ShaderType::Struct new_type;
  size_t ti = 0;
  size_t oi = 0;
  while (ti < _members.size() && oi < other_members.size()) {
    const Member &this_member = _members[ti];
    const Member &other_member = other_members[oi];

    if (this_member.name == other_member.name) {
      const ShaderType *merged = this_member.type->merge(other_member.type);
      if (merged == nullptr) {
        return nullptr;
      }
      new_type.add_member(merged, this_member.name);
      ++ti;
      ++oi;
      continue;
    }

    if (!has_member(other_member.name)) {
      new_type.add_member(other_member.type, other_member.name);
      ++oi;
    } else {
      new_type.add_member(this_member.type, this_member.name);
      ++ti;
    }
  }

  while (ti < _members.size()) {
    const Member &this_member = _members[ti];
    new_type.merge_member_by_name(this_member.name, this_member.type);
    ++ti;
  }
  while (oi < other_members.size()) {
    const Member &other_member = other_members[oi];
    new_type.merge_member_by_name(other_member.name, other_member.type);
    ++oi;
  }

  return ShaderType::register_type(std::move(new_type));
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
 * Outputs a signature that compactly but uniquely identifies this type.
 */
void ShaderType::Struct::
output_signature(std::ostream &out) const {
  out << 'S';
  for (const Member &member : _members) {
    member.type->output_signature(out);
  }
  out << '_';
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
    if (_members[i].offset != other_struct._members[i].offset) {
      return (_members[i].offset > other_struct._members[i].offset)
           - (_members[i].offset < other_struct._members[i].offset);
    }
  }

  return 0;
}

/**
 * Returns the alignment in bytes of this type in memory, if applicable.
 */
uint32_t ShaderType::Struct::
get_align_bytes() const {
  uint32_t align = 16;
  for (const Member &member : _members) {
    align = std::max(align, member.type->get_align_bytes());
  }
  return (align + 15) & ~15;
}

/**
 * Returns the size in bytes of this type in memory, if applicable.  Opaque
 * types will return 0.
 */
uint32_t ShaderType::Struct::
get_size_bytes() const {
  // Structs are padded to the base alignment of a vec4.
  uint32_t size = _members.empty() ? 0 : _members.back().offset + _members.back().type->get_size_bytes();
  return (size + 15) & ~15;
}

/**
 * Returns the number of in/out locations taken up by in/out variables having
 * this type.
 */
int ShaderType::Struct::
get_num_interface_locations() const {
  int total = 0;
  for (const Member &member : _members) {
    total += member.type->get_num_interface_locations();
  }
  return total;
}

/**
 * Returns the number of resources (samplers, etc.) in this type.
 */
int ShaderType::Struct::
get_num_resources() const {
  int total = 0;
  for (const Member &member : _members) {
    total += member.type->get_num_resources();
  }
  return total;
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderType::Struct::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_uint32(_members.size());
  for (const Member &member : _members) {
    manager->write_pointer(dg, member.type);
    dg.add_string(member.name);
    dg.add_uint32(member.offset);
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int ShaderType::Struct::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = ShaderType::complete_pointers(p_list, manager);

  for (Member &member : _members) {
    member.type = (ShaderType *)p_list[pi++];
    nassertd(member.type->is_registered()) continue;
  }

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ShaderType is encountered in the Bam file.  It should create the
 * ShaderType and extract its information from the file.
 */
TypedWritable *ShaderType::Struct::
make_from_bam(const FactoryParams &params) {
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  ShaderType::Struct *struct_type = new ShaderType::Struct;

  size_t num_members = scan.get_uint32();
  struct_type->_members.resize(num_members);
  for (size_t i = 0; i < num_members; ++i) {
    manager->read_pointer(scan);

    Member &member = struct_type->_members[i];
    member.type = nullptr;
    member.name = scan.get_string();
    member.offset = scan.get_uint32();
  }

  manager->register_change_this(change_this, struct_type);
  return struct_type;
}

/**
 * If this type is an array, puts the element type in the first argument and the
 * number of elements in the second argument, and returns true.  If not, puts
 * the current type in the first argument, and 1 in the second argument, and
 * returns false.
 */
bool ShaderType::Array::
unwrap_array(const ShaderType *&element_type, uint32_t &num_elements) const {
  element_type = _element_type;
  num_elements = _num_elements;
  return true;
}

/**
 * Returns true if this type is or contains any opaque type.
 */
bool ShaderType::Array::
contains_opaque_type() const {
  nassertr_always(_element_type != nullptr, false);
  return _element_type->contains_opaque_type();
}

/**
 * Returns true if this type contains the given scalar type.
 */
bool ShaderType::Array::
contains_scalar_type(ScalarType type) const {
  return _element_type != nullptr && _element_type->contains_scalar_type(type);
}

/**
 * If this is an array, vector or matrix of a scalar type, extracts the
 * dimensions.
 */
bool ShaderType::Array::
as_scalar_type(ScalarType &type, uint32_t &num_elements,
               uint32_t &num_rows, uint32_t &num_columns) const {
  if (_element_type != nullptr &&
      _element_type->as_array() == nullptr &&
      _element_type->as_scalar_type(type, num_elements, num_rows, num_columns) &&
      num_elements == 1) {
    num_elements = _num_elements;
    return true;
  }
  return false;
}

/**
 * Replaces any occurrence of the given scalar type with the given other one.
 */
const ShaderType *ShaderType::Array::
replace_scalar_type(ScalarType a, ScalarType b) const {
  const ShaderType *element_type = _element_type->replace_scalar_type(a, b);
  if (_element_type != element_type) {
    return ShaderType::register_type(ShaderType::Array(element_type, _num_elements));
  } else {
    return this;
  }
}

/**
 * Returns a new type with all occurrences of the given type recursively
 * replaced with the second type.
 */
const ShaderType *ShaderType::Array::
replace_type(const ShaderType *a, const ShaderType *b) const {
  if (this == a) {
    return b;
  }
  const ShaderType *element_type = _element_type->replace_type(a, b);
  if (_element_type != element_type) {
    return ShaderType::register_type(ShaderType::Array(element_type, _num_elements));
  } else {
    return this;
  }
}

/**
 * Returns a new type that can contain both this type and the other type.
 */
const ShaderType *ShaderType::Array::
merge(const ShaderType *other) const {
  if (this == other) {
    return this;
  }
  const ShaderType::Array *other_array = other->as_array();
  if (other_array == nullptr) {
    return nullptr;
  }

  if (other_array->_element_type == _element_type) {
    return (other_array->_num_elements > _num_elements) ? other_array : this;
  }

  const ShaderType *merged = _element_type->merge(other_array->_element_type);
  if (merged == nullptr) {
    return nullptr;
  }
  return ShaderType::register_type(ShaderType::Array(merged, std::max(_num_elements, other_array->_num_elements)));
}

/**
 *
 */
void ShaderType::Array::
output(std::ostream &out) const {
  out << *_element_type << '[';
  if (_num_elements > 0) {
    out << _num_elements;
  }
  out << ']';
}

/**
 * Outputs a signature that compactly but uniquely identifies this type.
 */
void ShaderType::Array::
output_signature(std::ostream &out) const {
  _element_type->output_signature(out);
  out << 'A';
  out << _num_elements;
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
  if (_stride_bytes != other_array._stride_bytes) {
    return _stride_bytes < other_array._stride_bytes ? -1 : 1;
  }
  return (_num_elements > other_array._num_elements)
       - (_num_elements < other_array._num_elements);
}

/**
 * Returns the alignment in bytes of this type in memory, if applicable.
 */
uint32_t ShaderType::Array::
get_align_bytes() const {
  uint32_t align = _element_type->get_align_bytes();
  return (align + 15) & ~15;
}

/**
 * Returns the size in bytes of this type in memory, if applicable.  Opaque
 * types will return 0.
 */
uint32_t ShaderType::Array::
get_size_bytes() const {
  // Arrays have padding at the end so that the next member is aligned to a
  // 16-byte boundary.  This implies that a float may directly follow a vec3,
  // but not a vec3[1]!  I didn't make up these rules.
  uint32_t stride_bytes = _stride_bytes;
  if (stride_bytes == 0) {
    // Array stride is always (at least) 16 bytes in std140 / DX9, even though
    // this is (indeed) incredibly wasteful for arrays of scalars.
    uint32_t size = _element_type->get_size_bytes();
    stride_bytes = (size + 15) & ~15;
  }
  return stride_bytes * _num_elements;
}

/**
 * Returns the number of in/out locations taken up by in/out variables having
 * this type.
 */
int ShaderType::Array::
get_num_interface_locations() const {
  return _element_type->get_num_interface_locations() * _num_elements;
}

/**
 * Returns the number of resources (samplers, etc.) in this type.
 */
int ShaderType::Array::
get_num_resources() const {
  return _element_type->get_num_resources() * _num_elements;
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderType::Array::
write_datagram(BamWriter *manager, Datagram &dg) {
  manager->write_pointer(dg, _element_type);
  dg.add_uint32(_num_elements);
  dg.add_uint32(_stride_bytes);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int ShaderType::Array::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = ShaderType::complete_pointers(p_list, manager);
  _element_type = (ShaderType *)p_list[pi++];
  nassertr(_element_type->is_registered(), pi);
  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ShaderType is encountered in the Bam file.  It should create the
 * ShaderType and extract its information from the file.
 */
TypedWritable *ShaderType::Array::
make_from_bam(const FactoryParams &params) {
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  manager->read_pointer(scan);
  uint32_t num_elements = scan.get_uint32();
  uint32_t stride_bytes = 0;
  if (scan.get_remaining_size() >= 4) {
    stride_bytes = scan.get_uint32();
  }
  ShaderType *type = new ShaderType::Array(nullptr, num_elements, stride_bytes);

  manager->register_change_this(change_this, type);
  return type;
}

/**
 *
 */
void ShaderType::Image::
output(std::ostream &out) const {
  if ((_access & Access::WRITE_ONLY) == Access::NONE) {
    out << "readonly ";
  }
  if ((_access & Access::READ_ONLY) == Access::NONE) {
    out << "writeonly ";
  }
  if (_sampled_type == ST_int) {
    out << 'i';
  } else if (_sampled_type == ST_uint) {
    out << 'u';
  }
  out << "image" << texture_type_suffixes[_texture_type];
}

/**
 * Outputs a signature that compactly but uniquely identifies this type.
 */
void ShaderType::Image::
output_signature(std::ostream &out) const {
  out << 'I' << scalar_signatures[_sampled_type] << texture_type_signatures[_texture_type] << access_signatures[(size_t)_access];
}

/**
 * Private implementation of compare_to, only called for types with the same
 * TypeHandle.
 */
int ShaderType::Image::
compare_to_impl(const ShaderType &other) const {
  const Image &other_image = (const Image &)other;
  if (_texture_type != other_image._texture_type) {
    return (_texture_type > other_image._texture_type)
         - (_texture_type < other_image._texture_type);
  }
  if (_sampled_type != other_image._sampled_type) {
    return (_sampled_type > other_image._sampled_type)
         - (_sampled_type < other_image._sampled_type);
  }
  return (_access > other_image._access)
       - (_access < other_image._access);
}

/**
 * Returns true if this type contains the given scalar type.
 */
bool ShaderType::Image::
contains_scalar_type(ScalarType type) const {
  return _sampled_type == type;
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderType::Image::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_uint8(_texture_type);
  dg.add_uint8(_sampled_type);
  dg.add_uint8((uint8_t)_access);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ShaderType is encountered in the Bam file.  It should create the
 * ShaderType and extract its information from the file.
 */
TypedWritable *ShaderType::Image::
make_from_bam(const FactoryParams &params) {
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  Texture::TextureType texture_type = (Texture::TextureType)scan.get_uint8();
  ScalarType sampled_type = (ScalarType)scan.get_uint8();
  Access access = (Access)scan.get_uint8();

  return (ShaderType *)ShaderType::register_type(ShaderType::Image(texture_type, sampled_type, access));
}

/**
 *
 */
void ShaderType::Sampler::
output(std::ostream &out) const {
  out << "sampler";
}

/**
 * Outputs a signature that compactly but uniquely identifies this type.
 */
void ShaderType::Sampler::
output_signature(std::ostream &out) const {
  out << 's';
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
 * This function is called by the BamReader's factory when a new object of
 * type ShaderType is encountered in the Bam file.  It should create the
 * ShaderType and extract its information from the file.
 */
TypedWritable *ShaderType::Sampler::
make_from_bam(const FactoryParams &params) {
  return (ShaderType *)ShaderType::sampler_type;
}

/**
 *
 */
void ShaderType::SampledImage::
output(std::ostream &out) const {
  if (_sampled_type == ST_int) {
    out << 'i';
  } else if (_sampled_type == ST_uint) {
    out << 'u';
  }
  out << "sampler" << texture_type_suffixes[_texture_type];
  if (_shadow) {
    out << "Shadow";
  }
}

/**
 * Outputs a signature that compactly but uniquely identifies this type.
 */
void ShaderType::SampledImage::
output_signature(std::ostream &out) const {
  out << 't';
  if (_shadow) {
    out << 's';
  }
  out << texture_type_signatures[_texture_type];
}

/**
 * Private implementation of compare_to, only called for types with the same
 * TypeHandle.
 */
int ShaderType::SampledImage::
compare_to_impl(const ShaderType &other) const {
  const SampledImage &other_sampled_image = (const SampledImage &)other;
  if (_texture_type != other_sampled_image._texture_type) {
    return (_texture_type > other_sampled_image._texture_type)
         - (_texture_type < other_sampled_image._texture_type);
  }
  if (_sampled_type != other_sampled_image._sampled_type) {
    return (_sampled_type > other_sampled_image._sampled_type)
         - (_sampled_type < other_sampled_image._sampled_type);
  }
  return (_shadow > other_sampled_image._shadow)
       - (_shadow < other_sampled_image._shadow);
}

/**
 * Returns true if this type contains the given scalar type.
 */
bool ShaderType::SampledImage::
contains_scalar_type(ScalarType type) const {
  return _sampled_type == type;
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderType::SampledImage::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_uint8(_texture_type);
  dg.add_uint8(_sampled_type);
  dg.add_bool(_shadow);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ShaderType is encountered in the Bam file.  It should create the
 * ShaderType and extract its information from the file.
 */
TypedWritable *ShaderType::SampledImage::
make_from_bam(const FactoryParams &params) {
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  Texture::TextureType texture_type = (Texture::TextureType)scan.get_uint8();
  ScalarType sampled_type = (ScalarType)scan.get_uint8();
  bool shadow = scan.get_bool();
  return (ShaderType *)ShaderType::register_type(ShaderType::SampledImage(texture_type, sampled_type, shadow));
}

/**
 *
 */
void ShaderType::StorageBuffer::
output(std::ostream &out) const {
  if ((_access & Access::WRITE_ONLY) == Access::NONE) {
    out << "readonly ";
  }
  if ((_access & Access::READ_ONLY) == Access::NONE) {
    out << "writeonly ";
  }
  out << "buffer";

  if (const ShaderType::Struct *struct_type = _contained_type->as_struct()) {
    out << " { ";
    for (const Struct::Member &member : struct_type->_members) {
      if (member.type != nullptr) {
        out << *member.type << ' ';
      }
      out << member.name << "; ";
    }
    out << '}';
  }
  else if (_contained_type != nullptr) {
    out << ' ' << *_contained_type;
  }
}

/**
 * Outputs a signature that compactly but uniquely identifies this type.
 */
void ShaderType::StorageBuffer::
output_signature(std::ostream &out) const {
  out << 'B';
  _contained_type->output_signature(out);
  out << access_signatures[(size_t)_access];
}

/**
 * Private implementation of compare_to, only called for types with the same
 * TypeHandle.
 */
int ShaderType::StorageBuffer::
compare_to_impl(const ShaderType &other) const {
  const StorageBuffer &other_buffer = (const StorageBuffer &)other;
  if (_contained_type != other_buffer._contained_type) {
    return (_contained_type > other_buffer._contained_type)
         - (_contained_type < other_buffer._contained_type);
  }
  return (_access > other_buffer._access)
       - (_access < other_buffer._access);
}

/**
 * Returns true if this type contains the given scalar type.
 */
bool ShaderType::StorageBuffer::
contains_scalar_type(ScalarType type) const {
  return _contained_type != nullptr && _contained_type->contains_scalar_type(type);
}

/**
 * Replaces any occurrence of the given scalar type with the given other one.
 */
const ShaderType *ShaderType::StorageBuffer::
replace_scalar_type(ScalarType a, ScalarType b) const {
  const ShaderType *contained_type = _contained_type->replace_scalar_type(a, b);
  if (_contained_type != contained_type) {
    return ShaderType::register_type(ShaderType::StorageBuffer(contained_type, _access));
  } else {
    return this;
  }
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderType::StorageBuffer::
write_datagram(BamWriter *manager, Datagram &dg) {
  manager->write_pointer(dg, _contained_type);
  dg.add_uint8((uint8_t)_access);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int ShaderType::StorageBuffer::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = ShaderType::complete_pointers(p_list, manager);
  _contained_type = (ShaderType *)p_list[pi++];
  nassertr(_contained_type->is_registered(), pi);
  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ShaderType is encountered in the Bam file.  It should create the
 * ShaderType and extract its information from the file.
 */
TypedWritable *ShaderType::StorageBuffer::
make_from_bam(const FactoryParams &params) {
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  manager->read_pointer(scan);
  Access access = (Access)scan.get_uint8();

  ShaderType *type = new ShaderType::StorageBuffer(nullptr, access);
  manager->register_change_this(change_this, type);
  return type;
}

#endif  // CPPPARSER
