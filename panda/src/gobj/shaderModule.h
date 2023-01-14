/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModule.h
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#ifndef SHADERMODULE_H
#define SHADERMODULE_H

#include "copyOnWriteObject.h"
#include "bamCacheRecord.h"
#include "shaderType.h"
#include "internalName.h"

/**
 * Represents a single shader module in some intermediate representation for
 * passing to the driver.  This could contain compiled bytecode, or in some
 * cases, preprocessed source code to be given directly to the driver.
 *
 * This class inherits from CopyOnWriteObject so that modules can safely be
 * shared between multiple Shader objects, with a unique copy automatically
 * being created if the Shader needs to manipulate the module.
 */
class EXPCL_PANDA_GOBJ ShaderModule : public CopyOnWriteObject {
PUBLISHED:
  enum class Stage {
    vertex,
    tess_control,
    tess_evaluation,
    geometry,
    fragment,
    compute,
  };

  /**
   * Defines an interface variable.
   */
  struct Variable {
  public:
    int has_location() const { return _location >= 0; }
    int get_location() const { return _location; }

  PUBLISHED:
    const ShaderType *type;
    CPT(InternalName) name;

    MAKE_PROPERTY2(location, has_location, get_location);

  public:
    int _location;
  };

  /**
   * Defines a specialization constant.
   */
  struct SpecializationConstant {
  PUBLISHED:
    const ShaderType *type;
    CPT(InternalName) name;
    uint32_t id;
  };

public:
  ShaderModule(Stage stage);
  virtual ~ShaderModule();

  INLINE Stage get_stage() const;
  INLINE int get_used_capabilities() const;

  INLINE const Filename &get_source_filename() const;
  INLINE void set_source_filename(const Filename &);

  INLINE const SpecializationConstant &get_spec_constant(size_t i) const;
  INLINE size_t get_num_spec_constants() const;

  size_t get_num_inputs() const;
  const Variable &get_input(size_t i) const;
  int find_input(CPT_InternalName name) const;

  size_t get_num_outputs() const;
  const Variable &get_output(size_t i) const;
  int find_output(CPT_InternalName name) const;

  size_t get_num_parameters() const;
  const Variable &get_parameter(size_t i) const;
  int find_parameter(CPT_InternalName name) const;

  typedef pmap<CPT_InternalName, Variable *> VariablesByName;

  virtual bool link_inputs(const ShaderModule *previous);
  virtual void remap_parameter_locations(pmap<int, int> &remap);

PUBLISHED:
  MAKE_PROPERTY(stage, get_stage);
  MAKE_SEQ_PROPERTY(inputs, get_num_inputs, get_input);
  MAKE_SEQ_PROPERTY(outputs, get_num_outputs, get_output);
  MAKE_SEQ_PROPERTY(spec_constants, get_num_spec_constants, get_spec_constant);

  virtual std::string get_ir() const=0;

public:
  /**
   * Indicates which features are used by the shader, which can be used by the
   * driver to check whether cross-compilation is possible, or whether certain
   * transformation steps may need to be applied.
   */
  enum Capabilities : uint64_t {
    C_basic_shader = 1ull << 0,
    C_vertex_texture = 1ull << 1,
    C_sampler_shadow = 1ull << 2, // 1D and 2D only
    C_point_sprites = 1ull << 3,

    // GLSL 1.20
    C_invariant = 1ull << 4,
    C_non_square_matrices = 1ull << 5,

    // GLSL 1.30
    C_unsigned_int = 1ull << 6,
    C_flat_interpolation = 1ull << 7, // also necessary for int varyings
    C_noperspective_interpolation = 1ull << 8, // not supported in GLES
    C_texture_lod = 1ull << 9, // textureLod in vshader doesn't count, textureGrad does
    C_texture_fetch = 1ull << 10, // texelFetch, textureSize, etc.
    C_int_samplers = 1ull << 11, // usampler2D, isampler2D, etc.
    C_sampler_cube_shadow = 1ull << 12,
    C_vertex_id = 1ull << 13,
    C_round_even = 1ull << 14, // roundEven function, also in SM 4.0

    // GLSL 1.40 / SM 4.0
    C_instance_id = 1ull << 15,
    C_buffer_texture = 1ull << 16, // ES 3.20

    // GLSL 1.50 / SM 4.0
    C_geometry_shader = 1ull << 17,
    C_primitive_id = 1ull << 18,

    // GLSL 3.30 / ES 3.00 / SM 4.0
    C_bit_encoding = 1ull << 19,

    // GLSL 4.00 / ES 3.20 / SM 5.0
    C_texture_gather = 1ull << 20,
    C_double = 1ull << 21,
    C_cube_map_array = 1ull << 22,
    C_tessellation_shader = 1ull << 23,
    C_sample_variables = 1ull << 24,
    C_extended_arithmetic = 1ull << 25,
    C_texture_query_lod = 1ull << 26, // not in ES

    // GLSL 4.20 / ES 3.10 / SM 5.0
    C_image_load_store = 1ull << 27,

    // GLSL 4.30 / ES 3.10 / SM 5.0
    C_compute_shader = 1ull << 28,
    C_texture_query_levels = 1ull << 29, // not in ES

    // GLSL 4.40 / ARB_enhanced_layouts
    C_enhanced_layouts = 1ull << 30,

    // GLSL 4.50
    C_derivative_control = 1ull << 31,
    C_texture_query_samples = 1ull << 32,
  };

  static std::string format_stage(Stage stage);
  static void output_capabilities(std::ostream &out, int capabilities);

  virtual void output(std::ostream &out) const;

  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;

protected:
  void fillin(DatagramIterator &scan, BamReader *manager) override;

protected:
  Stage _stage;
  PT(BamCacheRecord) _record;
  //std::pvector<Filename> _source_files;
  Filename _source_filename;
  //time_t _source_modified = 0;
  int _used_caps = 0;

  typedef pvector<Variable> Variables;
  Variables _inputs;
  Variables _outputs;
  Variables _parameters;

  typedef pvector<SpecializationConstant> SpecializationConstants;
  SpecializationConstants _spec_constants;

  friend class Shader;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "ShaderModule",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const ShaderModule &module) {
  module.output(out);
  return out;
}

INLINE std::ostream &operator << (std::ostream &out, ShaderModule::Stage stage) {
  return out << ShaderModule::format_stage(stage);
}

#include "shaderModule.I"

#endif
