/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderType.h
 * @author rdb
 * @date 2019-03-12
 */

#ifndef SHADERTYPE_H
#define SHADERTYPE_H

#include "typedObject.h"
#include "geomEnums.h"
#include "pmap.h"
#include "stl_compares.h"
#include "texture.h"

/**
 * This represents a single type as defined in a shader.  There is only ever a
 * single instance in existence for any particular type.
 */
class EXPCL_PANDA_GOBJ ShaderType : public TypedWritable {
public:
  template<class Type>
  static const Type *register_type(Type &&type);
  INLINE bool is_registered() const;

  INLINE int compare_to(const ShaderType &other) const;
  virtual int compare_to_impl(const ShaderType &other) const=0;

  virtual void output(std::ostream &out) const=0;
  virtual void output_signature(std::ostream &out) const=0;

  virtual uint32_t get_align_bytes() const { return 1; }
  virtual uint32_t get_size_bytes() const { return 0; }
  virtual int get_num_interface_locations() const { return 1; }
  virtual int get_num_resources() const { return 0; }

  enum ScalarType {
    ST_unknown,
    ST_float,
    ST_double,
    ST_int,
    ST_uint,
    ST_bool,

#ifdef STDFLOAT_DOUBLE
    ST_stdfloat = ST_double,
#else
    ST_stdfloat = ST_float,
#endif
  };

  enum class Access {
    NONE = 0,
    READ_ONLY = 1,
    WRITE_ONLY = 2,
    READ_WRITE = 3,
  };

private:
  typedef pset<const ShaderType *, indirect_compare_to<const ShaderType *> > Registry;
  static Registry *_registered_types;

PUBLISHED:
  class Void;
  class Scalar;
  class Vector;
  class Matrix;
  class Struct;
  class Array;
  class Resource;
  class Image;
  class Sampler;
  class SampledImage;
  class StorageBuffer;

  // Fundamental types.
  static const ShaderType::Void *void_type;
  static const ShaderType::Scalar *bool_type;
  static const ShaderType::Scalar *int_type;
  static const ShaderType::Scalar *uint_type;
  static const ShaderType::Scalar *float_type;
  static const ShaderType::Scalar *double_type;
  static const ShaderType::Sampler *sampler_type;

public:
  virtual bool is_aggregate_type() const { return false; }
  virtual bool unwrap_array(const ShaderType *&element_type, uint32_t &num_elements) const;
  virtual bool contains_opaque_type() const { return false; }
  virtual bool contains_scalar_type(ScalarType type) const { return false; }
  virtual bool as_scalar_type(ScalarType &type,
                              uint32_t &num_elements,
                              uint32_t &num_rows,
                              uint32_t &num_columns) const { return false; }
  virtual const ShaderType *replace_scalar_type(ScalarType a, ScalarType b) const { return this; }
  virtual const ShaderType *replace_type(const ShaderType *a, const ShaderType *b) const;
  virtual const ShaderType *merge(const ShaderType *other) const;

  INLINE static constexpr uint32_t get_scalar_size_bytes(ScalarType scalar_type);

  virtual const Scalar *as_scalar() const { return nullptr; }
  virtual const Vector *as_vector() const { return nullptr; }
  virtual const Matrix *as_matrix() const { return nullptr; }
  virtual const Struct *as_struct() const { return nullptr; }
  virtual const Array *as_array() const { return nullptr; }
  virtual const Resource *as_resource() const { return nullptr; }
  virtual const Image *as_image() const { return nullptr; }
  virtual const Sampler *as_sampler() const { return nullptr; }
  virtual const SampledImage *as_sampled_image() const { return nullptr; }
  virtual const StorageBuffer *as_storage_buffer() const { return nullptr; }

  static void register_with_read_factory();
  virtual bool require_fully_complete() const override;
  static TypedWritable *change_this(TypedWritable *old_ptr, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {
    init_type();
    return get_type();
  }

private:
  static TypeHandle _type_handle;
};

constexpr ShaderType::Access operator & (ShaderType::Access a, ShaderType::Access b) {
  return (ShaderType::Access)((unsigned int)a & (unsigned int)b);
}

constexpr ShaderType::Access operator | (ShaderType::Access a, ShaderType::Access b) {
  return (ShaderType::Access)((unsigned int)a | (unsigned int)b);
}

std::ostream &operator << (std::ostream &out, ShaderType::ScalarType scalar_type);

INLINE std::ostream &operator << (std::ostream &out, const ShaderType &stype) {
  stype.output(out);
  return out;
}

/**
 * The void type, invalid for most uses.
 */
class EXPCL_PANDA_GOBJ ShaderType::Void final : public ShaderType {
public:
  virtual void output(std::ostream &out) const override;
  virtual void output_signature(std::ostream &out) const override;

private:
  virtual int compare_to_impl(const ShaderType &other) const override;

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

/**
 * A numeric scalar type, like int or float.
 */
class EXPCL_PANDA_GOBJ ShaderType::Scalar final : public ShaderType {
public:
  INLINE Scalar(ScalarType scalar_type);

  INLINE ScalarType get_scalar_type() const;
  virtual bool contains_scalar_type(ScalarType type) const override;
  virtual const ShaderType *replace_scalar_type(ScalarType a, ScalarType b) const override;
  virtual bool as_scalar_type(ScalarType &type, uint32_t &num_elements,
                              uint32_t &num_rows, uint32_t &num_columns) const override;

  const Scalar *as_scalar() const override { return this; }

  virtual void output(std::ostream &out) const override;
  virtual void output_signature(std::ostream &out) const override;

private:
  virtual int compare_to_impl(const ShaderType &other) const override;

  virtual uint32_t get_align_bytes() const override;
  virtual uint32_t get_size_bytes() const override;

  const ScalarType _scalar_type;

protected:
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

/**
 * Multiple scalar types.
 */
class EXPCL_PANDA_GOBJ ShaderType::Vector final : public ShaderType {
public:
  INLINE Vector(ScalarType scalar_type, uint32_t num_components);
  Vector(const Vector &copy) = default;

  INLINE ScalarType get_scalar_type() const;
  INLINE uint32_t get_num_components() const;

  virtual bool contains_scalar_type(ScalarType type) const override;
  virtual bool as_scalar_type(ScalarType &type, uint32_t &num_elements,
                              uint32_t &num_rows, uint32_t &num_columns) const override;
  virtual const ShaderType *replace_scalar_type(ScalarType a, ScalarType b) const override;
  virtual const ShaderType *merge(const ShaderType *other) const override;

  virtual int get_num_interface_locations() const override;

  const Vector *as_vector() const override { return this; }

  virtual void output(std::ostream &out) const override;
  virtual void output_signature(std::ostream &out) const override;

private:
  virtual int compare_to_impl(const ShaderType &other) const override;

  virtual uint32_t get_align_bytes() const override;
  virtual uint32_t get_size_bytes() const override;

  const ScalarType _scalar_type;
  const uint32_t _num_components;

protected:
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

/**
 * Matrix consisting of multiple column vectors.
 */
class EXPCL_PANDA_GOBJ ShaderType::Matrix final : public ShaderType {
public:
  INLINE Matrix(ScalarType scalar_type, uint32_t num_rows, uint32_t num_columns);

  INLINE ScalarType get_scalar_type() const;
  INLINE uint32_t get_num_rows() const;
  INLINE uint32_t get_num_columns() const;

  virtual bool contains_scalar_type(ScalarType type) const override;
  virtual bool as_scalar_type(ScalarType &type, uint32_t &num_elements,
                              uint32_t &num_rows, uint32_t &num_columns) const override;
  virtual const ShaderType *replace_scalar_type(ScalarType a, ScalarType b) const override;

  virtual int get_num_interface_locations() const override;

  const Matrix *as_matrix() const override { return this; }

  virtual void output(std::ostream &out) const override;
  virtual void output_signature(std::ostream &out) const override;

private:
  virtual int compare_to_impl(const ShaderType &other) const override;

  virtual uint32_t get_align_bytes() const override;
  virtual uint32_t get_size_bytes() const override;

  const ScalarType _scalar_type;
  const uint32_t _num_rows;
  const uint32_t _num_columns;

protected:
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

/**
 * A structure type, with named members.
 */
class EXPCL_PANDA_GOBJ ShaderType::Struct final : public ShaderType {
public:
  struct Member;

  INLINE size_t get_num_members() const;
  INLINE const Member &get_member(size_t i) const;
  INLINE bool has_member(const std::string &name) const;
  void add_member(const ShaderType *type, std::string name);
  void add_member(const ShaderType *type, std::string name, uint32_t offset);

  void merge_member_by_name(std::string name, const ShaderType *type);

  virtual void output(std::ostream &out) const override;
  virtual void output_signature(std::ostream &out) const override;
  virtual int compare_to_impl(const ShaderType &other) const override;

  virtual uint32_t get_align_bytes() const override;
  virtual uint32_t get_size_bytes() const override;
  virtual int get_num_interface_locations() const override;
  virtual int get_num_resources() const override;

  bool is_aggregate_type() const override { return true; }
  virtual bool contains_opaque_type() const override;
  virtual bool contains_scalar_type(ScalarType type) const override;
  virtual const ShaderType *replace_scalar_type(ScalarType a, ScalarType b) const override;
  virtual const ShaderType *replace_type(const ShaderType *a, const ShaderType *b) const override;
  virtual const ShaderType *merge(const ShaderType *other) const override;
  const Struct *as_struct() const override { return this; }

PUBLISHED:
  MAKE_SEQ_PROPERTY(members, get_num_members, get_member);

  struct Member {
    const ShaderType *type;
    std::string name;
    uint32_t offset;
  };

private:
  pvector<Member> _members;

protected:
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager) override;
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

/**
 * An array type.
 */
class EXPCL_PANDA_GOBJ ShaderType::Array final : public ShaderType {
public:
  INLINE Array(const ShaderType *element_type, uint32_t num_elements);

  INLINE const ShaderType *get_element_type() const;
  INLINE uint32_t get_num_elements() const;

  virtual bool unwrap_array(const ShaderType *&element_type, uint32_t &num_elements) const override;

  virtual bool contains_opaque_type() const override;
  virtual bool contains_scalar_type(ScalarType type) const override;
  virtual bool as_scalar_type(ScalarType &type, uint32_t &num_elements,
                              uint32_t &num_rows, uint32_t &num_columns) const override;
  virtual const ShaderType *replace_scalar_type(ScalarType a, ScalarType b) const override;
  virtual const ShaderType *replace_type(const ShaderType *a, const ShaderType *b) const override;
  virtual const ShaderType *merge(const ShaderType *other) const override;

  virtual void output(std::ostream &out) const override;
  virtual void output_signature(std::ostream &out) const override;
  virtual int compare_to_impl(const ShaderType &other) const override;

  uint32_t get_stride_bytes() const;
  virtual uint32_t get_align_bytes() const override;
  virtual uint32_t get_size_bytes() const override;
  virtual int get_num_interface_locations() const override;
  virtual int get_num_resources() const override;

  bool is_aggregate_type() const override { return true; }
  const Array *as_array() const override { return this; }

PUBLISHED:
  MAKE_PROPERTY(element_type, get_element_type);
  MAKE_PROPERTY(num_elements, get_num_elements);

private:
  const ShaderType *_element_type;
  uint32_t _num_elements;

protected:
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager) override;
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

/**
 * Base class for all resources.
 */
class EXPCL_PANDA_GOBJ ShaderType::Resource : public ShaderType {
public:
  virtual int get_num_resources() const override { return 1; }

  virtual bool contains_opaque_type() const override { return true; }

  const Resource *as_resource() const override final { return this; }
};

/**
 * Image type.
 */
class EXPCL_PANDA_GOBJ ShaderType::Image final : public ShaderType::Resource {
public:
  INLINE Image(Texture::TextureType texture_type, ScalarType sampled_type, Access access);

  INLINE Texture::TextureType get_texture_type() const;
  INLINE ScalarType get_sampled_type() const;
  INLINE Access get_access() const;
  INLINE bool is_writable() const;

  virtual void output(std::ostream &out) const override;
  virtual void output_signature(std::ostream &out) const override;
  virtual int compare_to_impl(const ShaderType &other) const override;

  virtual bool contains_scalar_type(ScalarType type) const override;

  const Image *as_image() const override { return this; }

PUBLISHED:
  MAKE_PROPERTY(texture_type, get_texture_type);
  MAKE_PROPERTY(sampled_type, get_sampled_type);
  MAKE_PROPERTY(access, get_access);
  MAKE_PROPERTY(writable, is_writable);

private:
  Texture::TextureType _texture_type;
  ScalarType _sampled_type;
  Access _access;

protected:
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

/**
 * Sampler state.
 */
class EXPCL_PANDA_GOBJ ShaderType::Sampler final : public ShaderType::Resource {
private:
  Sampler() = default;

public:
  virtual void output(std::ostream &out) const override;
  virtual void output_signature(std::ostream &out) const override;
  virtual int compare_to_impl(const ShaderType &other) const override;

  const Sampler *as_sampler() const override { return this; }

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

/**
 * Sampled image type.
 */
class EXPCL_PANDA_GOBJ ShaderType::SampledImage final : public ShaderType::Resource {
public:
  INLINE SampledImage(Texture::TextureType texture_type, ScalarType sampled_type,
                      bool shadow = false);

  INLINE Texture::TextureType get_texture_type() const;
  INLINE ScalarType get_sampled_type() const;
  INLINE bool is_shadow() const;

  virtual void output(std::ostream &out) const override;
  virtual void output_signature(std::ostream &out) const override;
  virtual int compare_to_impl(const ShaderType &other) const override;

  virtual bool contains_scalar_type(ScalarType type) const override;

  const SampledImage *as_sampled_image() const override { return this; }

private:
  Texture::TextureType _texture_type;
  ScalarType _sampled_type;
  bool _shadow = false;

protected:
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

/**
 * Opaque storage buffer (SSBO) storing a given type, which is usually a struct
 * or an array.
 */
class EXPCL_PANDA_GOBJ ShaderType::StorageBuffer final : public ShaderType::Resource {
public:
  INLINE StorageBuffer(const ShaderType *contained_type, Access access);

  INLINE const ShaderType *get_contained_type() const;
  INLINE Access get_access() const;

  virtual void output(std::ostream &out) const override;
  virtual void output_signature(std::ostream &out) const override;
  virtual int compare_to_impl(const ShaderType &other) const override;

  virtual bool contains_scalar_type(ScalarType type) const override;
  virtual const ShaderType *replace_scalar_type(ScalarType a, ScalarType b) const override;

  const StorageBuffer *as_storage_buffer() const override { return this; }

PUBLISHED:
  MAKE_PROPERTY(contained_type, get_contained_type);
  MAKE_PROPERTY(access, get_access);

private:
  const ShaderType *_contained_type;
  Access _access;

protected:
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager) override;
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

#ifndef CPPPARSER
#include "shaderType.I"
#endif

#endif  // SHADERTYPE_H
