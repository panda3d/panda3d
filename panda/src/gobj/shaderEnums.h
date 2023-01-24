/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderEnums.h
 * @author rdb
 * @date 2023-01-22
 */

#ifndef SHADERENUMS_H
#define SHADERENUMS_H

#include "copyOnWriteObject.h"
#include "bamCacheRecord.h"
#include "shaderType.h"
#include "internalName.h"

/**
 * Provides scoping for various enum types that are used by both ShaderModule
 * and Shader classes.
 */
class EXPCL_PANDA_GOBJ ShaderEnums {
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
   * Indicates which features are used by the shader, which can be used by the
   * driver to check whether cross-compilation is possible, or whether certain
   * transformation steps may need to be applied.
   */
  enum Capabilities : uint64_t {
    // Always set, indicates that we support basic vertex+fragment shaders.
    C_basic_shader = 1ull << 0,

    C_vertex_texture = 1ull << 1,
    C_point_coord = 1ull << 2,
    C_standard_derivatives = 1ull << 3,
    C_shadow_samplers = 1ull << 4, // 1D and 2D only

    // GLSL 1.20
    C_non_square_matrices = 1ull << 5,

    // GLSL 1.30 / SM 3.0
    C_texture_lod = 1ull << 6, // textureLod in vshader doesn't count, textureGrad does

    /**
     * This cap deserves some explanation: it refers to the new shader model
     * exposed by hardware capable of OpenGL 3.0 / DirectX 10 (SM 4.0) and above
     * or GL_EXT_gpu_shader4, adding features like:
     * - "true" (non-emulated) 32-bit integers and operations
     * - unsigned integers
     * - integer samplers (if C_texture_integer is also present)
     * - integer varyings
     * - integer bitwise operations
     * - flat interpolation
     * - roundEven
     * - texelFetch
     * Because all these features require the new shader model and are not
     * offered individually, they have been wrapped up in a single bit flag.
     * A shader with this bit set will not work with DirectX 9.
     */
    C_unified_model = 1ull << 7,

    C_noperspective_interpolation = 1ull << 8, // not supported in GLES
    C_texture_array = 1ull << 9,
    C_texture_integer = 1ull << 10, // usampler2D, isampler2D, etc.
    C_texture_query_size = 1ull << 11, // textureSize, etc. (could be emulated)
    C_sampler_cube_shadow = 1ull << 12,
    C_vertex_id = 1ull << 13,
    C_draw_buffers = 1ull << 14, // MRT
    C_clip_distance = 1ull << 15, // gl_ClipDistance

    // GLSL 1.40 / SM 4.0
    C_instance_id = 1ull << 16, // ES 3.00
    C_texture_buffer = 1ull << 17, // ES 3.20

    // GLSL 1.50 / ES 3.20 / SM 4.0
    C_geometry_shader = 1ull << 18,
    C_primitive_id = 1ull << 19,

    // GLSL 3.30 / ES 3.00 / SM 4.0
    C_bit_encoding = 1ull << 20,

    // GLSL 4.00 / SM 4.1
    C_texture_query_lod = 1ull << 21,
    C_texture_gather_red = 1ull << 22,

    // GLSL 4.00 / ES 3.10 / SM 5.0
    C_texture_gather_any = 1ull << 23,
    C_extended_arithmetic = 1ull << 24,
    C_double = 1ull << 25, // Not in ES

    // GLSL 4.00 / ES 3.20 / SM 5.0
    C_cube_map_array = 1ull << 26,
    C_geometry_shader_instancing = 1ull << 27,
    C_tessellation_shader = 1ull << 28,
    C_sample_variables = 1ull << 29,
    C_multisample_interpolation = 1ull << 30,
    C_dynamic_indexing = 1ull << 31, // SM 5.1

    // GLSL 4.20 / ES 3.10 / SM 5.0
    C_atomic_counters = 1ull << 32,
    C_image_load_store = 1ull << 33,
    C_image_atomic = 1ull << 34, // ES 3.20 or OES_shader_image_atomic

    // GLSL 4.30 / ES 3.10 / SM 5.0
    C_image_query_size = 1ull << 35,
    C_texture_query_levels = 1ull << 36, // not in ES
    C_storage_buffer = 1ull << 37,
    C_compute_shader = 1ull << 38,

    // GLSL 4.40 / ARB_enhanced_layouts
    C_enhanced_layouts = 1ull << 39,

    // GLSL 4.50
    C_cull_distance = 1ull << 40,
    C_derivative_control = 1ull << 41,
    C_texture_query_samples = 1ull << 42,
  };

  static std::string format_stage(Stage stage);
  static void output_capabilities(std::ostream &out, int capabilities);
};

INLINE std::ostream &operator << (std::ostream &out, ShaderEnums::Stage stage) {
  return out << ShaderEnums::format_stage(stage);
}

#endif
