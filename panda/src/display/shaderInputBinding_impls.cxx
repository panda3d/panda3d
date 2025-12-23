/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderInputBinding_impls.cxx
 * @author rdb
 * @date 2024-09-22
 */

#include "shaderInputBinding_impls.h"
#include "graphicsStateGuardian.h"

#include "clipPlaneAttrib.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "fogAttrib.h"
#include "lightAttrib.h"
#include "materialAttrib.h"

#include "directionalLight.h"
#include "spotlight.h"
#include "pointLight.h"
#include "sphereLight.h"

using std::string;
using State = ShaderInputBinding::State;

static const LMatrix4 shadow_bias_mat(0.5f, 0.0f, 0.0f, 0.0f,
                                      0.0f, 0.5f, 0.0f, 0.0f,
                                      0.0f, 0.0f, 0.5f, 0.0f,
                                      0.5f, 0.5f, 0.5f, 1.0f);

static PT(Texture) white_texture;

/**
 * Returns a texture that is purely white.
 */
static Texture *
get_white_texture() {
  Texture *tex = white_texture.p();
  if (tex == nullptr) {
    tex = new Texture;
    tex->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_rgba);
    tex->set_clear_color(LVecBase4(1, 1, 1, 1));
    tex->set_minfilter(SamplerState::FT_nearest);
    tex->set_magfilter(SamplerState::FT_nearest);
    white_texture = tex;
  }
  return tex;
}

/**
 * Generate an error message including a description of the specified
 * parameter.  Always returns nullptr.
 */
static nullptr_t
report_parameter_error(const InternalName *name, const ShaderType *type, const char *msg) {
  shader_cat.error()
    << *type << ' ' << *name << ": " << msg << "\n";
  return nullptr;
}

/**
 * Make sure the provided parameter contains the specified number of words.
 * If not, print error message and return false.
 */
static bool
expect_num_words(const InternalName *name, const ShaderType *type, size_t len) {
  const std::string &name_str = name->get_basename();
  size_t n = std::count(name_str.begin(), name_str.end(), '_');
  if (n + 1 != len) {
    std::string msg = "parameter name has wrong number of words, expected ";
    msg += format_string(len);
    report_parameter_error(name, type, msg.c_str());
    return false;
  }
  return true;
}

/**
 * Make sure the provided parameter has a floating point vector type.  If not,
 * print error message and return false.
 */
static bool
expect_float_vector(const InternalName *name, const ShaderType *type,
                    int lo, int hi, bool allow_double = false) {
  int nfloat;
  ShaderType::ScalarType scalar_type = ShaderType::ST_unknown;
  if (const ShaderType::Scalar *scalar = type->as_scalar()) {
    nfloat = 1;
    scalar_type = scalar->get_scalar_type();
  }
  else if (const ShaderType::Vector *vector = type->as_vector()) {
    nfloat = vector->get_num_components();
    scalar_type = vector->get_scalar_type();
  }
  else {
    nfloat = 0;
  }
  if ((scalar_type != ShaderType::ST_float && (!allow_double || scalar_type != ShaderType::ST_double)) || nfloat < lo || nfloat > hi) {
    report_parameter_error(name, type, allow_double ? "expected float or double vector" : "expected float vector");
    return false;
  }
  return true;
}

/**
 * Make sure the provided parameter has a square floating-point matrix type.
 * Otherwise, prints an error message and returns false.
 */
static bool
expect_float_matrix(const InternalName *name, const ShaderType *type, int lo, int hi, bool allow_double = false) {
  ShaderType::ScalarType scalar_type = ShaderType::ST_unknown;
  uint32_t num_rows;
  uint32_t num_columns;
  if (const ShaderType::Matrix *matrix = type->as_matrix()) {
    num_rows = matrix->get_num_rows();
    num_columns = matrix->get_num_columns();
    scalar_type = matrix->get_scalar_type();
  }
  if ((scalar_type != ShaderType::ST_float && (!allow_double || scalar_type != ShaderType::ST_double)) ||
      (int)num_rows < lo || (int)num_rows > hi ||
      (int)num_columns < lo || (int)num_columns > hi) {

    std::string msg = allow_double ? "expected float or double matrix of " : "expected float matrix of ";
    if (lo < hi) {
      msg += "at least ";
    }
    msg += "size ";
    msg += format_string(lo);
    report_parameter_error(name, type, msg.c_str());
    return false;
  }
  return true;
}

/**
 * Convert a single-word coordinate system name into a PART/ARG of a
 * ShaderMatSpec.
 */
static bool
expect_coordinate_system(const InternalName *name, const ShaderType *type,
                         vector_string &pieces, int &next, bool fromflag,
                         Shader::StateMatrix *part, CPT(InternalName) *arg) {

  if (pieces[next] == "" || pieces[next] == "to" || pieces[next] == "rel") {
    report_parameter_error(name, type, "invalid coordinate system name");
    return false;
  }

  string word1 = pieces[next++];
  if (pieces[next] == "of") {
    next++;
  }
  string word2;
  if (pieces[next] != "" && pieces[next] != "to" && pieces[next] != "rel") {
    word2 = pieces[next++];
  }

  Shader::StateMatrix from_single;
  Shader::StateMatrix from_double;
  Shader::StateMatrix to_single;
  Shader::StateMatrix to_double;

  if (word1 == "world") {
    from_single = Shader::SM_world_to_view;
    from_double = Shader::SM_INVALID;
    to_single   = Shader::SM_view_to_world;
    to_double   = Shader::SM_INVALID;
  }
  else if (word1 == "model") {
    from_single = Shader::SM_model_to_view;
    from_double = Shader::SM_view_x_to_view;
    to_single   = Shader::SM_view_to_model;
    to_double   = Shader::SM_view_to_view_x;
  }
  else if (word1 == "clip") {
    from_single = Shader::SM_clip_to_view;
    from_double = Shader::SM_clip_x_to_view;
    to_single   = Shader::SM_view_to_clip;
    to_double   = Shader::SM_view_to_clip_x;
  }
  else if (word1 == "view") {
    from_single = Shader::SM_identity;
    from_double = Shader::SM_view_x_to_view;
    to_single   = Shader::SM_identity;
    to_double   = Shader::SM_view_to_view_x;
  }
  else if (word1 == "apiview") {
    from_single = Shader::SM_apiview_to_view;
    from_double = Shader::SM_apiview_x_to_view;
    to_single   = Shader::SM_view_to_apiview;
    to_double   = Shader::SM_view_to_apiview_x;
  }
  else if (word1 == "apiclip") {
    from_single = Shader::SM_apiclip_to_view;
    from_double = Shader::SM_apiclip_x_to_view;
    to_single   = Shader::SM_view_to_apiclip;
    to_double   = Shader::SM_view_to_apiclip_x;
  }
  else {
    from_single = Shader::SM_view_x_to_view;
    from_double = Shader::SM_view_x_to_view;
    to_single   = Shader::SM_view_to_view_x;
    to_double   = Shader::SM_view_to_view_x;
    word2 = word1;
  }

  if (fromflag) {
    if (word2 == "") {
      part[0] = from_single;
      arg[0] = nullptr;
    } else {
      if (from_double == Shader::SM_INVALID) {
        report_parameter_error(name, type, "invalid coordinate system name");
        return false;
      }
      part[0] = from_double;
      arg[0] = InternalName::make(word2);
    }
  } else {
    if (word2 == "") {
      part[1] = to_single;
      arg[1] = nullptr;
    } else {
      if (to_double == Shader::SM_INVALID) {
        report_parameter_error(name, type, "invalid coordinate system name");
        return false;
      }
      part[1] = to_double;
      arg[1] = InternalName::make(word2);
    }
  }
  return true;
}

/**
 * Returns true if this could be a member of a light structure.
 */
static bool
check_light_struct_member(const string &name, const ShaderType *type) {
  uint32_t num_rows = 1;
  uint32_t min_cols = 3;
  uint32_t max_cols = 4;
  if (name.empty()) {
    return type == ShaderType::void_type;
  } else if (name == "color") {
  } else if (name == "specular") {
  } else if (name == "ambient") {
  } else if (name == "diffuse") {
  } else if (name == "position") {
  } else if (name == "halfVector") {
  } else if (name == "spotDirection") {
  } else if (name == "spotCosCutoff") {
    min_cols = max_cols = 1;
  } else if (name == "spotCutoff") {
    min_cols = max_cols = 1;
  } else if (name == "spotExponent") {
    min_cols = max_cols = 1;
  } else if (name == "attenuation") {
  } else if (name == "constantAttenuation") {
    min_cols = max_cols = 1;
  } else if (name == "linearAttenuation") {
    min_cols = max_cols = 1;
  } else if (name == "quadraticAttenuation") {
    min_cols = max_cols = 1;
  } else if (name == "radius") {
    min_cols = max_cols = 1;
  } else if (name == "shadowViewMatrix") {
    num_rows = 4;
    min_cols = max_cols = 4;
  } else if (name == "shadowMap") {
    return type->as_sampled_image() != nullptr;
  } else {
    return false;
  }

  const ShaderType::Matrix *matrix = type->as_matrix();
  if (matrix != nullptr) {
    if (matrix->get_num_rows() != num_rows ||
        matrix->get_num_columns() < min_cols ||
        matrix->get_num_columns() > max_cols) {
      return false;
    }
  }
  else if (num_rows != 1) {
    return false;
  }
  else {
    uint32_t num_components = 1;
    if (const ShaderType::Vector *vector = type->as_vector()) {
      num_components = vector->get_num_components();
    }
    else if (type->as_scalar() == nullptr) {
      return false;
    }
    if (num_components < min_cols || num_components > max_cols) {
      return false;
    }
  }

  return true;
}

/**
 * Binds to a generic named shader input.
 */
static ShaderInputBinding *
make_shader_input(const ShaderType *type, CPT_InternalName name) {
  if (const ShaderType::SampledImage *sampler = type->as_sampled_image()) {
    Texture::TextureType desired_type = sampler->get_texture_type();
    return new ShaderTextureBinding(std::move(name), desired_type);
  }
  else if (const ShaderType::Image *image = type->as_image()) {
    Texture::TextureType desired_type = image->get_texture_type();
    return new ShaderTextureBinding(std::move(name), desired_type);
  }
  else if (const ShaderType::StorageBuffer *buffer = type->as_storage_buffer()) {
    size_t min_size = 0;
    if (const ShaderType *contained_type = buffer->get_contained_type()) {
      min_size = contained_type->get_size_bytes();
    }
    return new ShaderBufferBinding(std::move(name), min_size);
  }
  else if (const ShaderType::Matrix *matrix = type->as_matrix()) {
    // For historical reasons, we handle non-arrayed matrices differently,
    // which has the additional feature that they can be passed in as a
    // NodePath instead of a matrix.
    int dep = Shader::D_shader_inputs | Shader::D_frame;
    if (matrix->get_num_columns() == 4) {
      uint32_t num_rows = matrix->get_num_rows();
      if (num_rows == 4) {
        if (matrix->get_scalar_type() == ShaderType::ST_double) {
          return ShaderInputBinding::make_data(dep, [=](const State &state, void *into, bool packed) {
            state.gsg->get_target_shader_attrib()->get_shader_input_matrix(name, *(LMatrix4d *)into);
          });
        } else {
          return ShaderInputBinding::make_data(dep, [=](const State &state, void *into, bool packed) {
            state.gsg->get_target_shader_attrib()->get_shader_input_matrix(name, *(LMatrix4f *)into);
          });
        }
      } else {
        if (matrix->get_scalar_type() == ShaderType::ST_double) {
          return ShaderInputBinding::make_data(dep, [=](const State &state, void *into, bool packed) {
            LMatrix4d tmp;
            state.gsg->get_target_shader_attrib()->get_shader_input_matrix(name, tmp);
            memcpy(into, tmp.get_data(), num_rows * sizeof(double) * 4);
          });
        } else {
          return ShaderInputBinding::make_data(dep, [=](const State &state, void *into, bool packed) {
            LMatrix4f tmp;
            state.gsg->get_target_shader_attrib()->get_shader_input_matrix(name, tmp);
            memcpy(into, tmp.get_data(), num_rows * sizeof(float) * 4);
          });
        }
      }
    }
    else if (matrix->get_num_columns() == 3) {
      uint32_t num_rows = matrix->get_num_rows();
      if (num_rows == 3) {
        // Short-cut for most common case
        if (matrix->get_scalar_type() == ShaderType::ST_double) {
          return ShaderInputBinding::make_data(dep, [=](const State &state, void *into, bool packed) {
            LMatrix4d tmp;
            state.gsg->get_target_shader_attrib()->get_shader_input_matrix(name, tmp);
            if (!packed) {
              memcpy(into, tmp.get_data(), sizeof(double) * 4 * 3);
            } else {
              *((LMatrix3d *)into) = tmp.get_upper_3();
            }
          });
        } else {
          return ShaderInputBinding::make_data(dep, [=](const State &state, void *into, bool packed) {
            LMatrix4f tmp;
            state.gsg->get_target_shader_attrib()->get_shader_input_matrix(name, tmp);
            if (!packed) {
              memcpy(into, tmp.get_data(), sizeof(float) * 4 * 3);
            } else {
              *((LMatrix3f *)into) = tmp.get_upper_3();
            }
          });
        }
      } else {
        if (matrix->get_scalar_type() == ShaderType::ST_double) {
          return ShaderInputBinding::make_data(dep, [=](const State &state, void *into, bool packed) {
            LMatrix4d tmp;
            state.gsg->get_target_shader_attrib()->get_shader_input_matrix(name, tmp);
            if (!packed) {
              memcpy(into, tmp.get_data(), num_rows * sizeof(double) * 4);
            } else {
              for (uint32_t i = 0; i < num_rows; ++i) {
                ((LVecBase3d *)into)[i] = tmp.get_row3(i);
              }
            }
          });
        } else {
          return ShaderInputBinding::make_data(dep, [=](const State &state, void *into, bool packed) {
            LMatrix4f tmp;
            state.gsg->get_target_shader_attrib()->get_shader_input_matrix(name, tmp);
            if (!packed) {
              memcpy(into, tmp.get_data(), num_rows * sizeof(float) * 4);
            } else {
              for (uint32_t i = 0; i < num_rows; ++i) {
                ((LVecBase3f *)into)[i] = tmp.get_row3(i);
              }
            }
          });
        }
      }
    }
  }

  ShaderType::ScalarType scalar_type;
  uint32_t arg_dim[3];
  if (type->as_scalar_type(scalar_type, arg_dim[0], arg_dim[1], arg_dim[2])) {
    if (arg_dim[0] > 1 && arg_dim[1] > 1 && arg_dim[1] != arg_dim[2]) {
      shader_cat.error()
        << "Non-square matrix arrays are not supported in custom shader inputs\n";
      return nullptr;
    }

    switch (scalar_type) {
    case ShaderType::ST_float:
      return new ShaderFloatBinding(name, arg_dim[0], arg_dim[1], arg_dim[2]);

    case ShaderType::ST_double:
      return new ShaderDoubleBinding(name, arg_dim[0], arg_dim[1], arg_dim[2]);

    case ShaderType::ST_bool:
      return new ShaderBoolBinding(name, arg_dim[0], arg_dim[1], arg_dim[2]);

    default:
      return new ShaderIntBinding(name, arg_dim[0], arg_dim[1], arg_dim[2]);
    }
  }

  const ShaderType::Struct *struct_type = type->as_struct();

  // This could be a custom user struct or a light struct.  Does it look like
  // a light struct?
  if (struct_type != nullptr && struct_type->get_num_members() > 0) {
    bool may_be_light_struct = true;
    for (size_t i = 0; i < struct_type->get_num_members(); ++i) {
      const ShaderType::Struct::Member &member = struct_type->get_member(i);

      if (!check_light_struct_member(member.name, member.type)) {
        may_be_light_struct = false;
      }
    }
    if (may_be_light_struct) {
      // Yeah, might be.  There's special code in the light struct binding
      // to fall back to a regular struct otherwise.
      return new ShaderLightStructBinding(type, name);
    }
  }

  if (struct_type != nullptr || type->as_array() != nullptr) {
    return new ShaderAggregateBinding(name, type);
  }

  return nullptr;
}

/**
 *
 */
static ShaderInputBinding *
make_matrix(const ShaderType *type, Shader::StateMatrix input,
            CPT_InternalName arg, bool transpose = false, size_t offset = 0) {
  ShaderType::ScalarType scalar_type;
  uint32_t num_elements;
  uint32_t num_rows;
  uint32_t num_cols;
  if (!type->as_scalar_type(scalar_type, num_elements, num_rows, num_cols) ||
      scalar_type != ShaderType::ST_float ||
      num_rows > 4 || num_cols > 4 || num_elements != 1) {
    return nullptr;
  }

  return new ShaderMatrixBinding(input, arg, transpose, offset, num_rows, num_cols);
}

/**
 *
 */
static ShaderInputBinding *
make_matrix_compose(const ShaderType *type, Shader::StateMatrix input0,
                    CPT_InternalName arg0, Shader::StateMatrix input1,
                    CPT_InternalName arg1, bool transpose = false, size_t offset = 0) {

  // clip == apiclip in OpenGL, and the apiclip matrices are cached.
  if (input0 == Shader::SM_view_to_clip) {
    input0 = Shader::SM_view_to_apiclip;
  }
  else if (input0 == Shader::SM_clip_to_view) {
    input0 = Shader::SM_apiclip_to_view;
  }

  if (input1 == Shader::SM_view_to_clip) {
    input1 = Shader::SM_view_to_apiclip;
  }
  else if (input1 == Shader::SM_clip_to_view) {
    input1 = Shader::SM_apiclip_to_view;
  }

  if (input0 == Shader::SM_world_to_view && input1 == Shader::SM_view_to_apiview) {
    input0 = Shader::SM_world_to_apiview;
    input1 = Shader::SM_identity;
  }

  if (input1 == Shader::SM_identity) {
    return make_matrix(type, input0, arg0, transpose, offset);
  }
  if (input0 == Shader::SM_identity) {
    return make_matrix(type, input1, arg1, transpose, offset);
  }

  ShaderType::ScalarType scalar_type;
  uint32_t num_elements;
  uint32_t num_rows;
  uint32_t num_cols;
  if (!type->as_scalar_type(scalar_type, num_elements, num_rows, num_cols) ||
      scalar_type != ShaderType::ST_float || num_elements != 1 ||
      num_rows > 4 || num_cols < 3 || num_cols > 4) {
    return nullptr;
  }

  return new ShaderMatrixComposeBinding(input0, arg0, input1, arg1, transpose, offset, num_rows, num_cols);
}

/**
 *
 */
static ShaderInputBinding *
make_transform_table(const ShaderType *type, bool transpose) {
  ShaderType::ScalarType scalar_type;
  uint32_t num_elements;
  uint32_t num_rows;
  uint32_t num_cols;
  if (!type->as_scalar_type(scalar_type, num_elements, num_rows, num_cols) ||
      num_cols != 4) {
    return nullptr;
  }

  if (num_rows == 4) {
    return ShaderInputBinding::make_data(Shader::D_vertex_data,
                                         [=](const State &state, void *into, bool packed) {

      const TransformTable *table = state.gsg->get_data_reader()->get_transform_table();
      LMatrix4f *matrices = (LMatrix4f *)into;
      size_t i = 0;
      if (table != nullptr) {
        size_t num_transforms = std::min((size_t)num_elements, table->get_num_transforms());
        for (; i < num_transforms; ++i) {
#ifdef STDFLOAT_DOUBLE
          LMatrix4 matrix;
          table->get_transform(i)->get_matrix(matrix);
          matrices[i] = LCAST(float, matrix);
#else
          table->get_transform(i)->get_matrix(matrices[i]);
#endif
        }
      }
      for (; i < num_elements; ++i) {
        matrices[i] = LMatrix4f::ident_mat();
      }
    });
  }
  else if (num_rows == 3) {
    // Reduced size, used by shader generator
    nassertr(transpose, nullptr);

    return ShaderInputBinding::make_data(Shader::D_vertex_data,
                                         [=](const State &state, void *into, bool packed) {

      const TransformTable *table = state.gsg->get_data_reader()->get_transform_table();
      LVecBase4f *vectors = (LVecBase4f *)into;
      size_t i = 0;
      if (table != nullptr) {
        size_t num_transforms = std::min((size_t)num_elements, table->get_num_transforms());
        for (; i < num_transforms; ++i) {
          LMatrix4f matrix;
#ifdef STDFLOAT_DOUBLE
          LMatrix4d matrixd;
          table->get_transform(i)->get_matrix(matrixd);
          matrix = LCAST(float, matrixd);
#else
          table->get_transform(i)->get_matrix(matrix);
#endif
          vectors[i * 3 + 0] = matrix.get_col(0);
          vectors[i * 3 + 1] = matrix.get_col(1);
          vectors[i * 3 + 2] = matrix.get_col(2);
        }
      }
      for (; i < num_elements; ++i) {
        vectors[i * 3 + 0].set(1, 0, 0, 0);
        vectors[i * 3 + 1].set(0, 1, 0, 0);
        vectors[i * 3 + 2].set(0, 0, 1, 0);
      }
    });
  }
  else {
    return nullptr;
  }
}

/**
 *
 */
static ShaderInputBinding *
make_slider_table(const ShaderType *type) {
  const ShaderType *element_type;
  uint32_t num_elements;
  type->unwrap_array(element_type, num_elements);
  nassertr(element_type == ShaderType::float_type, nullptr);

  return ShaderInputBinding::make_data(Shader::D_vertex_data,
                                       [=](const State &state, void *into, bool packed) {

    const SliderTable *table = state.gsg->get_data_reader()->get_slider_table();
    float *sliders = (float *)into;
    memset(sliders, 0, num_elements * sizeof(float));
    if (table != nullptr) {
      size_t num_transforms = std::min((size_t)num_elements, table->get_num_sliders());
      for (size_t i = 0; i < num_transforms; ++i) {
        sliders[i] = table->get_slider(i)->get_slider();
      }
    }
  });
}

/**
 *
 */
static ShaderInputBinding *
make_frame_time(const ShaderType *type) {
  if (type == ShaderType::float_type) {
    return ShaderInputBinding::make_data(Shader::D_frame,
                                         [](const State &state, void *into, bool packed) {
      *(float *)into = ClockObject::get_global_clock()->get_frame_time();
    });
  }
  if (type == ShaderType::double_type) {
    return ShaderInputBinding::make_data(Shader::D_frame,
                                         [](const State &state, void *into, bool packed) {
      *(double *)into = ClockObject::get_global_clock()->get_frame_time();
    });
  }
  return nullptr;
}

/**
 *
 */
static ShaderInputBinding *
make_color(const ShaderType *type) {
  return ShaderInputBinding::make_data(Shader::D_color,
                                       [](const State &state, void *into, bool packed) {

    const ColorAttrib *target_color = (const ColorAttrib *)
      state.gsg->get_target_state()->get_attrib_def(ColorAttrib::get_class_slot());
    if (target_color->get_color_type() == ColorAttrib::T_flat) {
      *(LVecBase4f *)into = LCAST(float, target_color->get_color());
    } else {
      *(LVecBase4f *)into = LVecBase4f(1, 1, 1, 1);
    }
  });
}

/**
 *
 */
static ShaderInputBinding *
make_color_scale(const ShaderType *type) {
  return ShaderInputBinding::make_data(Shader::D_colorscale,
                                       [](const State &state, void *into, bool packed) {

    const ColorScaleAttrib *target_color_scale = (const ColorScaleAttrib *)
      state.gsg->get_target_state()->get_attrib_def(ColorScaleAttrib::get_class_slot());
    if (!target_color_scale->is_identity()) {
      *(LVecBase4f *)into = LCAST(float, target_color_scale->get_scale());
    } else {
      *(LVecBase4f *)into = LVecBase4f(1, 1, 1, 1);
    }
  });
}

/**
 *
 */
static ShaderInputBinding *
make_texture_stage(const ShaderType *type, size_t index, CPT(InternalName) suffix = nullptr) {
  const ShaderType::SampledImage *sampled_image_type = type->as_sampled_image();
  if (sampled_image_type == nullptr) {
    return nullptr;
  }
  Texture::TextureType desired_type = sampled_image_type->get_texture_type();

  return ShaderInputBinding::make_texture(Shader::D_texture,
                                          [=](const State &state, SamplerState &sampler, int &view) {

    // We get the TextureAttrib directly from the _target_rs, not the
    // filtered TextureAttrib in _target_texture.
    const TextureAttrib *texattrib;
    state.gsg->get_target_state()->get_attrib_def(texattrib);

    PT(Texture) tex;
    if (index < (size_t)texattrib->get_num_on_stages()) {
      TextureStage *stage = texattrib->get_on_stage(index);
      sampler = texattrib->get_on_sampler(stage);
      view += stage->get_tex_view_offset();

      tex = texattrib->get_on_texture(stage);
      if (!suffix.is_null()) {
        tex = tex->load_related(suffix);
      }

      if (tex->get_texture_type() != desired_type) {
        shader_cat.error()
          << "Texture " << *tex << " at stage " << stage
          << " does not match type desired by shader\n";
      }
    } else {
      tex = get_white_texture();
    }

    return tex;
  });
}

/**
 *
 */
static ShaderInputBinding *
make_texture_matrix(const ShaderType *type, size_t index, bool inverse, bool transpose) {
  ShaderType::ScalarType scalar_type;
  uint32_t num_elements;
  uint32_t num_rows;
  uint32_t num_cols;
  if (!type->as_scalar_type(scalar_type, num_elements, num_rows, num_cols) ||
      num_rows != 4 || num_cols != 4) {
    return nullptr;
  }

  return ShaderInputBinding::make_data(Shader::D_tex_matrix,
                                       [=](const State &state, void *into, bool packed) {

    const TexMatrixAttrib *tma;
    const TextureAttrib *ta;

    uint32_t num_stages = 0;
    if (state.gsg->get_target_state()->get_attrib(ta) && state.gsg->get_target_state()->get_attrib(tma)) {
      num_stages = std::min(num_elements, (uint32_t)ta->get_num_on_stages());
    }

    uint32_t i = index;
    for (; i < num_stages; ++i) {
      ((LMatrix4f *)into)[i] = LCAST(float, tma->get_mat(ta->get_on_stage(i)));
      if (inverse) {
        ((LMatrix4f *)into)[i].invert_in_place();
      }
      if (transpose) {
        ((LMatrix4f *)into)[i].transpose_in_place();
      }
    }
    for (; i < index + num_elements; ++i) {
      ((LMatrix4f *)into)[i] = LCAST(float, LMatrix4::ident_mat());
    }
  });
}

/**
 *
 */
static ShaderInputBinding *
make_fog(const ShaderType *type) {
  const ShaderType::Struct *struct_type = type->as_struct();
  nassertr_always(struct_type != nullptr, nullptr);

  // Unfortunately the user could have jumbled up the members of this
  // struct.  Should we try and bless a certain ordering, and have a fast
  // path for that?
  int color_offset = -1;
  int density_offset = -1;
  int start_offset = -1;
  int end_offset = -1;
  int scale_offset = -1;

  bool success = true;
  for (size_t i = 0; i < struct_type->get_num_members(); ++i) {
    const ShaderType::Struct::Member &member = struct_type->get_member(i);
    if (member.type == ShaderType::void_type) {
      continue;
    }

    CPT(InternalName) fqname = InternalName::make(member.name);
    if (member.name == "color") {
      success = success && expect_float_vector(fqname, member.type, 3, 4);
      color_offset = member.offset;
    }
    else if (member.name == "density") {
      success = success && expect_float_vector(fqname, member.type, 1, 1);
      density_offset = member.offset;
    }
    else if (member.name == "start") {
      success = success && expect_float_vector(fqname, member.type, 1, 1);
      start_offset = member.offset;
    }
    else if (member.name == "end") {
      success = success && expect_float_vector(fqname, member.type, 1, 1);
      end_offset = member.offset;
    }
    else if (member.name == "scale") {
      success = success && expect_float_vector(fqname, member.type, 1, 1);
      scale_offset = member.offset;
    }
    else {
      report_parameter_error(fqname, member.type, "unrecognized fog attribute");
      success = false;
    }
  }

  if (!success) {
    return nullptr;
  }

  return ShaderInputBinding::make_data(Shader::D_fog | Shader::D_frame,
                                       [=](const State &state, void *into, bool packed) {

    LVecBase4f color(1, 1, 1, 1);
    PN_stdfloat density = 0, start = 1, end = 1, scale = 1;

    const FogAttrib *target_fog;
    if (state.gsg->get_target_state()->get_attrib(target_fog) && target_fog->get_fog() != nullptr) {
      Fog *fog = target_fog->get_fog();
      color = LCAST(float, fog->get_color());
      density = fog->get_exp_density();
      fog->get_linear_range(start, end);
      scale = 1.0f / (end - start);
    }

    unsigned char *p = (unsigned char *)into;
    if (color_offset >= 0) *(LVecBase4f *)(p + color_offset) = color;
    if (density_offset >= 0) *(float *)(p + density_offset) = density;
    if (start_offset >= 0) *(float *)(p + start_offset) = start;
    if (end_offset >= 0) *(float *)(p + end_offset) = end;
    if (scale_offset >= 0) *(float *)(p + scale_offset) = scale;
  });
}

/**
 *
 */
static ShaderInputBinding *
make_material(const ShaderType *type) {
  const ShaderType::Struct *struct_type = type->as_struct();
  nassertr_always(struct_type != nullptr, nullptr);

  // Unfortunately the user could have jumbled up the members of this
  // struct.  Should we try and bless a certain ordering, and have a fast
  // path for that?
  int base_color_offset = -1;
  int ambient_offset = -1;
  int diffuse_offset = -1;
  int emission_offset = -1;
  int specular_offset = -1;
  int shininess_offset = -1;
  int roughness_offset = -1;
  int metallic_offset = -1;
  int ior_offset = -1;

  bool success = true;
  for (size_t i = 0; i < struct_type->get_num_members(); ++i) {
    const ShaderType::Struct::Member &member = struct_type->get_member(i);
    if (member.type == ShaderType::void_type) {
      continue;
    }

    CPT(InternalName) fqname = InternalName::make(member.name);
    if (member.name == "baseColor") {
      success = success && expect_float_vector(fqname, member.type, 4, 4);
      base_color_offset = member.offset;
    }
    else if (member.name == "ambient") {
      success = success && expect_float_vector(fqname, member.type, 4, 4);
      ambient_offset = member.offset;
    }
    else if (member.name == "diffuse") {
      success = success && expect_float_vector(fqname, member.type, 4, 4);
      diffuse_offset = member.offset;
    }
    else if (member.name == "emission") {
      success = success && expect_float_vector(fqname, member.type, 4, 4);
      emission_offset = member.offset;
    }
    else if (member.name == "specular") {
      success = success && expect_float_vector(fqname, member.type, 3, 3);
      specular_offset = member.offset;
    }
    else if (member.name == "shininess") {
      success = success && expect_float_vector(fqname, member.type, 1, 1);
      shininess_offset = member.offset;
    }
    else if (member.name == "roughness") {
      success = success && expect_float_vector(fqname, member.type, 1, 1);
      roughness_offset = member.offset;
    }
    else if (member.name == "metallic") {
      if (member.type != ShaderType::bool_type &&
          member.type != ShaderType::float_type) {
        report_parameter_error(fqname, member.type, "expected bool or float");
        success = false;
      }
      metallic_offset = member.offset;
    }
    else if (member.name == "refractiveIndex") {
      success = success && expect_float_vector(fqname, member.type, 1, 1);
      ior_offset = member.offset;
    }
    else {
      report_parameter_error(fqname, member.type, "unrecognized material attribute");
      success = false;
    }
  }

  if (!success) {
    return nullptr;
  }

  return ShaderInputBinding::make_data(Shader::D_material | Shader::D_frame,
                                       [=](const State &state, void *into, bool packed) {

    LVecBase4f base_color(0, 0, 0, 0);
    LVecBase4f ambient(1, 1, 1, 1);
    LVecBase4f diffuse(1, 1, 1, 1);
    LVecBase4f emission(0, 0, 0, 0);
    LVecBase4f specular(0, 0, 0, 0);
    float shininess = 0;
    float metallic = 0;
    float ior = 0;
    float roughness = 1;

    const MaterialAttrib *target_material;
    if (state.gsg->get_target_state()->get_attrib(target_material) && !target_material->is_off()) {
      Material *m = target_material->get_material();
      base_color = LCAST(float, m->get_base_color());
      ambient = LCAST(float, m->get_ambient());
      diffuse = LCAST(float, m->get_diffuse());
      emission = LCAST(float, m->get_emission());
      specular = LCAST(float, m->get_specular());
      shininess = m->get_shininess();
      metallic = m->get_metallic();
      ior = m->get_refractive_index();
      roughness = m->get_roughness();
    }

    unsigned char *p = (unsigned char *)into;
    if (base_color_offset >= 0) *(LVecBase4f *)(p + base_color_offset) = base_color;
    if (ambient_offset >= 0) *(LVecBase4f *)(p + ambient_offset) = ambient;
    if (diffuse_offset >= 0) *(LVecBase4f *)(p + diffuse_offset) = diffuse;
    if (emission_offset >= 0) *(LVecBase4f *)(p + emission_offset) = emission;
    if (specular_offset >= 0) *(LVecBase4f *)(p + specular_offset) = specular;
    if (shininess_offset >= 0) *(float *)(p + shininess_offset) = shininess;
    if (metallic_offset >= 0) *(float *)(p + metallic_offset) = metallic;
    if (ior_offset >= 0) *(float *)(p + ior_offset) = ior;
    if (roughness_offset >= 0) *(float *)(p + roughness_offset) = roughness;
  });
}

/**
 *
 */
static ShaderInputBinding *
make_light_ambient(const ShaderType *type) {
  return ShaderInputBinding::make_data(Shader::D_frame | Shader::D_light,
                                       [](const State &state, void *into, bool packed) {
    const LightAttrib *target_light;
    if (state.gsg->get_target_state()->get_attrib(target_light) && target_light->has_any_on_light()) {
      *(LVecBase4f *)into = LCAST(float, target_light->get_ambient_contribution());
    } else {
      // There are no lights at all.  This means, to follow the fixed-
      // function model, we pretend there is an all-white ambient light.
      *(LVecBase4f *)into = LVecBase4f(1, 1, 1, 1);
    }
  });
}

/**
 * Returns a mask indicating which state changes should cause the parameter to
 * be respecified.
 */
int ShaderMatrixBinding::
get_state_dep() const {
  return Shader::get_matrix_deps(_input);
}

/**
 * Sets up anything necessary to prepare this binding to be used with the given
 * shader.
 */
void ShaderMatrixBinding::
setup(Shader *shader) {
  _cache_index = shader->add_matrix_cache_item(_input, _arg, Shader::get_matrix_deps(_input));
}

/**
 * Fetches the part of the shader input that is plain numeric data.
 */
void ShaderMatrixBinding::
fetch_data(const State &state, void *into, bool packed) const {
  LMatrix4f m = LCAST(float, state.matrix_cache[_cache_index]);
  if (_transpose) {
    m.transpose_in_place();
  }
  if (packed && _num_cols != 4) {
    for (size_t i = 0; i < _num_rows; ++i) {
      memcpy((float *)into + i * _num_cols, m.get_data() + i * 4, _num_cols * sizeof(float));
    }
  } else {
    memcpy(into, m.get_data(), _num_cols * 4 * sizeof(float));
  }
}

/**
 * Returns a mask indicating which state changes should cause the parameter to
 * be respecified.
 */
int ShaderMatrixComposeBinding::
get_state_dep() const {
  return (Shader::get_matrix_deps(_input0)
        | Shader::get_matrix_deps(_input1));
}

/**
 * Sets up anything necessary to prepare this binding to be used with the given
 * shader.
 */
void ShaderMatrixComposeBinding::
setup(Shader *shader) {
  _cache_index0 = shader->add_matrix_cache_item(_input0, _arg0, Shader::get_matrix_deps(_input0));
  _cache_index1 = shader->add_matrix_cache_item(_input1, _arg1, Shader::get_matrix_deps(_input1));
}

/**
 * Fetches the part of the shader input that is plain numeric data.
 */
void ShaderMatrixComposeBinding::
fetch_data(const State &state, void *into, bool packed) const {
  LMatrix4f m;
  m.multiply(LCAST(float, state.matrix_cache[_cache_index0]),
             LCAST(float, state.matrix_cache[_cache_index1]));
  if (_transpose) {
    m.transpose_in_place();
  }
  if (packed && _num_cols != 4) {
    for (size_t i = 0; i < _num_rows; ++i) {
      memcpy((float *)into + i * _num_cols, m.get_data() + i * 4, _num_cols * sizeof(float));
    }
  } else {
    memcpy(into, m.get_data(), _num_rows * 4 * sizeof(float));
  }
}

/**
 * Returns a mask indicating which state changes should cause the parameter to
 * be respecified.
 */
int ShaderPointParamsBinding::
get_state_dep() const {
  return (Shader::D_scene
        | Shader::D_render_mode
        | Shader::D_transform
        | Shader::get_matrix_deps(Shader::SM_point_attenuation));
}

/**
 * Sets up anything necessary to prepare this binding to be used with the given
 * shader.
 */
void ShaderPointParamsBinding::
setup(Shader *shader) {
  _cache_index = shader->add_matrix_cache_item(Shader::SM_point_attenuation, nullptr,
    Shader::get_matrix_deps(Shader::SM_point_attenuation));
}

/**
 * Fetches the part of the shader input that is plain numeric data.
 */
void ShaderPointParamsBinding::
fetch_data(const State &state, void *into, bool packed) const {
  const RenderModeAttrib *target_render_mode;
  state.gsg->get_target_state()->get_attrib_def(target_render_mode);

  PN_stdfloat thickness = target_render_mode->get_thickness();
  if (target_render_mode->get_perspective()) {
    LMatrix4 m = state.matrix_cache[_cache_index];
    PN_stdfloat scale = state.gsg->get_internal_transform()->get_scale()[1];

    LVector3 height = LVector3(0, thickness, 1) * m;
    height[1] *= scale;
    height[2] *= scale;
    *(LVecBase4f *)into = LCAST(float, height);
  } else {
    ((LVecBase4f *)into)->set(thickness, thickness, 0, 0);
  }
}

/**
 * Returns a mask indicating which state changes should cause the parameter to
 * be respecified.
 */
int ShaderPackedLightBinding::
get_state_dep() const {
  return Shader::D_light | Shader::D_frame |
         Shader::get_matrix_deps(Shader::SM_world_to_view);
}

/**
 * Sets up anything necessary to prepare this binding to be used with the given
 * shader.
 */
void ShaderPackedLightBinding::
setup(Shader *shader) {
  _world_mat_cache_index = shader->add_matrix_cache_item(Shader::SM_world_to_view, nullptr, Shader::get_matrix_deps(Shader::SM_world_to_view));
}

/**
 * Fetches the part of the shader input that is plain numeric data.
 */
void ShaderPackedLightBinding::
fetch_data(const State &state, void *into, bool packed) const {
  // The light matrix contains COLOR, ATTENUATION, VIEWVECTOR, POSITION
  LVecBase4f *data = (LVecBase4f *)into;

  // We don't count ambient lights, which would be pretty silly to handle
  // via this mechanism.
  const LightAttrib *target_light;
  if (state.gsg->get_target_state()->get_attrib(target_light) &&
      _index < target_light->get_num_non_ambient_lights()) {
    NodePath np = target_light->get_on_light(_index);
    nassertv(!np.is_empty());
    PandaNode *node = np.node();
    Light *light = node->as_light();
    nassertv(light != nullptr);
    data[0] = LCAST(float, light->get_color());
    data[1] = LVecBase4f(LCAST(float, light->get_attenuation()), 0);

    LMatrix4 mat = np.get_net_transform()->get_mat() *
      state.matrix_cache[_world_mat_cache_index];

    if (node->is_of_type(DirectionalLight::get_class_type())) {
      LVecBase3 d = mat.xform_vec(((const DirectionalLight *)node)->get_direction());
      d.normalize();
      data[2] = LVecBase4f(LCAST(float, d), 0);
      data[3] = LVecBase4f(LCAST(float, -d), 0);
    }
    else if (node->is_of_type(LightLensNode::get_class_type())) {
      const Lens *lens = ((const LightLensNode *)node)->get_lens();

      LPoint3 p = mat.xform_point(lens->get_nodal_point());
      data[3] = LVecBase4f(p[0], p[1], p[2], 1.0f);

      // For shadowed point light we need to store near/far.
      // For spotlight we need to store cutoff angle.
      if (node->is_of_type(Spotlight::get_class_type())) {
        PN_stdfloat cutoff = ccos(deg_2_rad(lens->get_hfov() * 0.5f));
        LVecBase3 d = -(mat.xform_vec(lens->get_view_vector()));
        data[1][3] = ((const Spotlight *)node)->get_exponent();
        data[2] = LVecBase4f(LCAST(float, d), cutoff);
      }
      else if (node->is_of_type(PointLight::get_class_type())) {
        data[1][3] = lens->get_far();
        data[3][3] = lens->get_near();

        if (node->is_of_type(SphereLight::get_class_type())) {
          data[2][3] = ((const SphereLight *)node)->get_radius();
        }
      }
    }
  } else {
    // Apply the default OpenGL lights otherwise.
    // Special exception for light 0, which defaults to white.
    if (_index == 0) {
      data[0].set(1, 1, 1, 1);
    } else {
      data[0].set(0, 0, 0, 0);
    }
    data[1].set(1, 0, 0, 0);
    data[2].set(0, 0, 0, 0);
    data[3].set(0, 0, 0, 0);
  }
}

/**
 * Returns a mask indicating which state changes should cause the parameter to
 * be respecified.
 */
int ShaderLegacyLightBinding::
get_state_dep() const {
  return Shader::D_shader_inputs |
         Shader::D_view_transform |
         Shader::D_frame |
         Shader::get_matrix_deps(_matrix);
}

/**
 * Sets up anything necessary to prepare this binding to be used with the given
 * shader.
 */
void ShaderLegacyLightBinding::
setup(Shader *shader) {
  _mat_cache_index = shader->add_matrix_cache_item(_matrix, _arg, Shader::get_matrix_deps(_matrix));
}

/**
 * Fetches the part of the shader input that is plain numeric data.
 */
void ShaderLegacyDirectionalLightBinding::
fetch_data(const State &state, void *into, bool packed) const {
  // The dlight matrix contains COLOR, SPECULAR, DIRECTION, PSEUDOHALFANGLE
  const NodePath &np = state.gsg->get_target_shader_attrib()->get_shader_input_nodepath(_input);
  nassertv(!np.is_empty());
  DirectionalLight *lt;
  DCAST_INTO_V(lt, np.node());
  LColor const &c = lt->get_color();
  LColor const &s = lt->get_specular_color();
  const LMatrix4 &cached_mat = state.matrix_cache[_mat_cache_index];
  LMatrix4 t = np.get_net_transform()->get_mat() *
               state.gsg->get_scene()->get_world_transform()->get_mat();
  LVecBase3 d = -(t.xform_vec(lt->get_direction()));
  d.normalize();
  d = cached_mat.xform_vec(d);
  d.normalize();
  LVecBase3 h = d + LVecBase3(0,-1,0);
  h.normalize();
  h = cached_mat.xform_vec(d);
  h.normalize();
  LVecBase4f *v = (LVecBase4f *)into;
  v[0].set(c[0], c[1], c[2], c[3]);
  v[1].set(s[0], s[1], s[2], c[3]);
  v[2].set(d[0], d[1], d[2], 0);
  v[3].set(h[0], h[1], h[2], 0);
}

/**
 * Fetches the part of the shader input that is plain numeric data.
 */
void ShaderLegacyPointLightBinding::
fetch_data(const State &state, void *into, bool packed) const {
  // The plight matrix contains COLOR, SPECULAR, POINT, ATTENUATION
  const NodePath &np = state.gsg->get_target_shader_attrib()->get_shader_input_nodepath(_input);
  nassertv(!np.is_empty());
  PointLight *lt;
  DCAST_INTO_V(lt, np.node());
  LColor const &c = lt->get_color();
  LColor const &s = lt->get_specular_color();
  LMatrix4 t = np.get_net_transform()->get_mat() *
               state.gsg->get_scene()->get_world_transform()->get_mat();
  LVecBase3 p = (t.xform_point(lt->get_point()));
  p = state.matrix_cache[_mat_cache_index].xform_point(p);
  LVecBase3 a = lt->get_attenuation();
  Lens *lens = lt->get_lens(0);
  PN_stdfloat lnear = lens->get_near();
  PN_stdfloat lfar = lens->get_far();
  LVecBase4f *v = (LVecBase4f *)into;
  v[0].set(c[0], c[1], c[2], c[3]);
  v[1].set(s[0], s[1], s[2], s[3]);
  v[2].set(p[0], p[1], p[2], lnear);
  v[3].set(a[0], a[1], a[2], lfar);
}

/**
 * Fetches the part of the shader input that is plain numeric data.
 */
void ShaderLegacySpotlightBinding::
fetch_data(const State &state, void *into, bool packed) const {
  // The slight matrix contains COLOR, SPECULAR, POINT, DIRECTION
  const NodePath &np = state.gsg->get_target_shader_attrib()->get_shader_input_nodepath(_input);
  nassertv(!np.is_empty());
  Spotlight *lt;
  DCAST_INTO_V(lt, np.node());
  Lens *lens = lt->get_lens();
  nassertv(lens != nullptr);
  LColor const &c = lt->get_color();
  LColor const &s = lt->get_specular_color();
  PN_stdfloat cutoff = ccos(deg_2_rad(lens->get_hfov() * 0.5f));
  const LMatrix4 &cached_mat = state.matrix_cache[_mat_cache_index];
  LMatrix4 t = np.get_net_transform()->get_mat() *
               state.gsg->get_scene()->get_world_transform()->get_mat();
  LVecBase3 p = t.xform_point(lens->get_nodal_point());
  p = cached_mat.xform_point(p);
  LVecBase3 d = -(t.xform_vec(lens->get_view_vector()));
  d = cached_mat.xform_vec(d);
  d.normalize();
  LVecBase4f *v = (LVecBase4f *)into;
  v[0].set(c[0], c[1], c[2], c[3]);
  v[1].set(s[0], s[1], s[2], s[3]);
  v[2].set(p[0], p[1], p[2], 0);
  v[3].set(d[0], d[1], d[2], cutoff);
}

/**
 *
 */
ShaderLightStructBinding::
ShaderLightStructBinding(const ShaderType *type, const InternalName *input) {
  const ShaderType *element_type;
  if (input != nullptr) {
    element_type = type;
    _count = 1;
    _input = input;
    _stride = 0;
  } else {
    uint32_t num_elements;
    type->unwrap_array(element_type, num_elements);
    _count = num_elements;
    if (num_elements > 1) {
      _stride = ((const ShaderType::Array *)type)->get_stride_bytes();
    } else {
      _stride = 0;
    }
  }

  const ShaderType::Struct *struct_type = element_type->as_struct();
  nassertv_always(struct_type != nullptr);

  for (size_t i = 0; i < struct_type->get_num_members(); ++i) {
    const ShaderType::Struct::Member &member = struct_type->get_member(i);
    if (member.type == ShaderType::void_type) {
      continue;
    }

    if (member.name == "color") {
      _color_offset = member.offset;
    }
    else if (member.name == "specular") {
      _specular_offset = member.offset;
    }
    else if (member.name == "ambient") {
      _ambient_offset = member.offset;
    }
    else if (member.name == "diffuse") {
      _diffuse_offset = member.offset;
    }
    else if (member.name == "position") {
      _position_offset = member.offset;
    }
    else if (member.name == "halfVector") {
      _half_vector_offset = member.offset;
    }
    else if (member.name == "spotDirection") {
      _spot_direction_offset = member.offset;
    }
    else if (member.name == "spotCosCutoff") {
      _spot_cos_cutoff_offset = member.offset;
    }
    else if (member.name == "spotCutoff") {
      _spot_cutoff_offset = member.offset;
    }
    else if (member.name == "spotExponent") {
      _spot_exponent_offset = member.offset;
    }
    else if (member.name == "attenuation") {
      _attenuation_offset = member.offset;
    }
    else if (member.name == "constantAttenuation") {
      _constant_attenuation_offset = member.offset;
    }
    else if (member.name == "linearAttenuation") {
      _linear_attenuation_offset = member.offset;
    }
    else if (member.name == "quadraticAttenuation") {
      _quadratic_attenuation_offset = member.offset;
    }
    else if (member.name == "radius") {
      _radius_offset = member.offset;
    }
    else if (member.name == "shadowViewMatrix") {
      _shadow_view_matrix_offset = member.offset;
    }
    else if (member.name == "shadowMap") {
      const ShaderType::SampledImage *sampler = member.type->as_sampled_image();
      nassertd(sampler != nullptr) continue;
      _cube_shadow_map = sampler->get_texture_type() == Texture::TT_cube_map;
    }
  }
}

/**
 * Returns a mask indicating which state changes should cause the parameter to
 * be respecified.
 */
int ShaderLightStructBinding::
get_state_dep() const {
  int dep = Shader::D_frame |
            Shader::get_matrix_deps(Shader::SM_world_to_apiview) |
            Shader::get_matrix_deps(Shader::SM_apiview_to_world);
  if (_input != nullptr) {
    return Shader::D_shader_inputs | dep;
  } else {
    return Shader::D_light | dep;
  }
}

/**
 * Sets up anything necessary to prepare this binding to be used with the given
 * shader.
 */
void ShaderLightStructBinding::
setup(Shader *shader) {
  _world_to_apiview_mat_cache_index = shader->add_matrix_cache_item(Shader::SM_world_to_apiview, nullptr, Shader::get_matrix_deps(Shader::SM_world_to_apiview));
  _apiview_to_world_mat_cache_index = shader->add_matrix_cache_item(Shader::SM_apiview_to_world, nullptr, Shader::get_matrix_deps(Shader::SM_apiview_to_world));
}

/**
 * Fetches the part of the shader input that is plain numeric data.
 */
void ShaderLightStructBinding::
fetch_data(const State &state, void *into, bool packed) const {
  if (_input != nullptr) {
    // Fetch shader input.
    if (state.gsg->get_target_shader_attrib()->has_shader_input(_input)) {
      const NodePath &np = state.gsg->get_target_shader_attrib()->get_shader_input_nodepath(_input);
      fetch_light(state, np, into);
    } else {
      // Maybe it's a data struct after all, even though it looked like a light
      // struct initially.
      fetch_from_input(state.gsg->get_target_shader_attrib(), into);
    }
  } else {
    // Fetch array from state.
    size_t num_lights = 0;
    const LightAttrib *target_light;
    if (state.gsg->get_target_state()->get_attrib(target_light)) {
      num_lights = std::min(_count, target_light->get_num_non_ambient_lights());
    }

    size_t i = 0;
    for (i = 0; i < num_lights; ++i) {
      NodePath light = target_light->get_on_light(i);
      fetch_light(state, light, into);
      into = (unsigned char *)into + _stride;
    }
    // Apply the default OpenGL lights otherwise.
    // Special exception for light 0, which defaults to white.
    for (; i < _count; ++i) {
      fetch_light(state, NodePath(), into);
      if (i == 0 && _color_offset >= 0) {
        *(LVecBase4f *)((unsigned char *)into + _color_offset) = LVecBase4f(1, 1, 1, 1);
      }
      into = (unsigned char *)into + _stride;
    }
  }
}

/**
 * Fetches the part of the shader input that is plain numeric data.
 */
void ShaderLightStructBinding::
fetch_light(const State &state, const NodePath &np, void *into) const {
  PandaNode *node = nullptr;
  if (!np.is_empty()) {
    node = np.node();
  }

  LVecBase4f color(0, 0, 0, 1);
  LVecBase4f specular(0, 0, 0, 1);
  LVecBase4f ambient(0, 0, 0, 1);
  LVecBase4f diffuse(0, 0, 0, 1);
  LVecBase4f position(0, 0, 1, 0);
  LVecBase4f half_vector(0, 0, 1, 0);
  LVecBase4f spot_direction(0, 0, -1, 0);
  float spot_cos_cutoff = -1;
  float spot_cutoff = 180;
  float spot_exponent = 0;
  LVecBase3f attenuation(1, 0, 0);
  float radius = 0;
  LMatrix4f shadow_view_matrix = LCAST(float, shadow_bias_mat);

  if (node != nullptr) {
    Light *light = node->as_light();
    nassertv(light != nullptr);

    color = LCAST(float, light->get_color());
    specular = LCAST(float, light->get_specular_color());

    if (node->is_ambient_light()) {
      ambient = color;
      diffuse.set(0, 0, 0, 1);
      position.set(0, 0, 0, 0);
      half_vector.set(0, 0, 0, 0);
      spot_direction.set(0, 0, 0, 0);
    } else {
      ambient.set(0, 0, 0, 1);
      diffuse = color;

      CPT(TransformState) net_transform = np.get_net_transform();
      LMatrix4 mat = net_transform->get_mat() *
        state.matrix_cache[_world_to_apiview_mat_cache_index];

      LightLensNode *light;
      DCAST_INTO_V(light, node);
      Lens *lens = light->get_lens();
      nassertv(lens != nullptr);

      if (node->is_of_type(DirectionalLight::get_class_type())) {
        DirectionalLight *light;
        DCAST_INTO_V(light, node);

        LVector3 dir = -(light->get_direction() * mat);
        position.set(dir[0], dir[1], dir[2], 0);

        dir.normalize();
        dir += LVector3(0, 0, 1);
        dir.normalize();
        half_vector.set(dir[0], dir[1], dir[2], 1);
      }
      else {
        LPoint3 pos = lens->get_nodal_point() * mat;
        position.set(pos[0], pos[1], pos[2], 1);

        pos.normalize();
        pos += LVector3(0, 0, 1);
        pos.normalize();
        half_vector.set(pos[0], pos[1], pos[2], 1);
      }

      if (node->is_of_type(Spotlight::get_class_type())) {
        float cutoff = lens->get_hfov() * 0.5f;
        spot_cos_cutoff = ccos(deg_2_rad(cutoff));
        spot_cutoff = cutoff;
      }
      spot_exponent = light->get_exponent();

      LVector3 dir = lens->get_view_vector() * mat;
      spot_direction.set(dir[0], dir[1], dir[2], 0);

      if (_shadow_view_matrix_offset >= 0) {
        CPT(TransformState) inv_net_transform = net_transform->get_inverse();
        LMatrix4 t = state.matrix_cache[_apiview_to_world_mat_cache_index] *
          inv_net_transform->get_mat() *
          LMatrix4::convert_mat(state.gsg->get_coordinate_system(), lens->get_coordinate_system());

        if (!node->is_of_type(PointLight::get_class_type())) {
          t *= lens->get_projection_mat() * shadow_bias_mat;
        }
        shadow_view_matrix = LCAST(float, t);
      }
    }

    attenuation = LCAST(float, light->get_attenuation());
    if (_radius_offset >= 0 && node->is_of_type(SphereLight::get_class_type())) {
      radius = ((const SphereLight *)node)->get_radius();
    }
  }

  unsigned char *p = (unsigned char *)into;

  if (_color_offset >= 0) *(LVecBase4f *)(p + _color_offset) = color;
  if (_specular_offset >= 0) *(LVecBase4f *)(p + _specular_offset) = specular;
  if (_ambient_offset >= 0) *(LVecBase4f *)(p + _ambient_offset) = ambient;
  if (_diffuse_offset >= 0) *(LVecBase4f *)(p + _diffuse_offset) = diffuse;
  if (_position_offset >= 0) *(LVecBase4f *)(p + _position_offset) = position;
  if (_half_vector_offset >= 0) *(LVecBase4f *)(p + _half_vector_offset) = half_vector;
  if (_spot_direction_offset >= 0) *(LVecBase4f *)(p + _spot_direction_offset) = spot_direction;
  if (_spot_cos_cutoff_offset >= 0) *(float *)(p + _spot_cos_cutoff_offset) = spot_cos_cutoff;
  if (_spot_cutoff_offset >= 0) *(float *)(p + _spot_cutoff_offset) = spot_cutoff;
  if (_spot_exponent_offset >= 0) *(float *)(p + _spot_exponent_offset) = spot_exponent;
  if (_attenuation_offset >= 0) *(LVecBase3f *)(p + _attenuation_offset) = attenuation;
  if (_constant_attenuation_offset >= 0) *(float *)(p + _constant_attenuation_offset) = attenuation[0];
  if (_linear_attenuation_offset >= 0) *(float *)(p + _linear_attenuation_offset) = attenuation[1];
  if (_quadratic_attenuation_offset >= 0) *(float *)(p + _quadratic_attenuation_offset) = attenuation[2];
  if (_radius_offset >= 0) *(float *)(p + _radius_offset) = radius;
  if (_shadow_view_matrix_offset >= 0) *(LMatrix4f *)(p + _shadow_view_matrix_offset) = shadow_view_matrix;
}

/**
 * Fetches the light information from a shader input, if it doesn't turn out
 * to be a light structure after all.
 */
void ShaderLightStructBinding::
fetch_from_input(const ShaderAttrib *target_shader, void *into) const {
  InternalName *input = (InternalName *)_input.p();
  unsigned char *p = (unsigned char *)into;
  if (_color_offset >= 0) {
    LVecBase4 v = target_shader->get_shader_input_vector(input->append("color"));
    *(LVecBase4f *)(p + _color_offset) = LCAST(float, v);
  }
  if (_specular_offset >= 0) {
    LVecBase4 v = target_shader->get_shader_input_vector(input->append("specular"));
    *(LVecBase4f *)(p + _specular_offset) = LCAST(float, v);
  }
  if (_ambient_offset >= 0) {
    LVecBase4 v = target_shader->get_shader_input_vector(input->append("ambient"));
    *(LVecBase4f *)(p + _ambient_offset) = LCAST(float, v);
  }
  if (_diffuse_offset >= 0) {
    LVecBase4 v = target_shader->get_shader_input_vector(input->append("diffuse"));
    *(LVecBase4f *)(p + _diffuse_offset) = LCAST(float, v);
  }
  if (_position_offset >= 0) {
    LVecBase4 v = target_shader->get_shader_input_vector(input->append("position"));
    *(LVecBase4f *)(p + _position_offset) = LCAST(float, v);
  }
  if (_half_vector_offset >= 0) {
    LVecBase4 v = target_shader->get_shader_input_vector(input->append("halfVector"));
    *(LVecBase4f *)(p + _half_vector_offset) = LCAST(float, v);
  }
  if (_spot_direction_offset >= 0) {
    LVecBase4 v = target_shader->get_shader_input_vector(input->append("spotDirection"));
    *(LVecBase4f *)(p + _spot_direction_offset) = LCAST(float, v);
  }
  if (_spot_cos_cutoff_offset >= 0) {
    LVecBase4 v = target_shader->get_shader_input_vector(input->append("spotCosCutoff"));
    *(float *)(p + _spot_cos_cutoff_offset) = v[0];
  }
  if (_spot_cutoff_offset >= 0) {
    LVecBase4 v = target_shader->get_shader_input_vector(input->append("spotCutoff"));
    *(float *)(p + _spot_cutoff_offset) = v[0];
  }
  if (_spot_exponent_offset >= 0) {
    LVecBase4 v = target_shader->get_shader_input_vector(input->append("spotExponent"));
    *(float *)(p + _spot_exponent_offset) = v[0];
  }
  if (_attenuation_offset >= 0) {
    LVecBase4 v = target_shader->get_shader_input_vector(input->append("attenuation"));
    *(LVecBase4f *)(p + _attenuation_offset) = LCAST(float, v);
  }
  if (_constant_attenuation_offset >= 0) {
    LVecBase4 v = target_shader->get_shader_input_vector(input->append("constantAttenuation"));
    *(float *)(p + _constant_attenuation_offset) = v[0];
  }
  if (_linear_attenuation_offset >= 0) {
    LVecBase4 v = target_shader->get_shader_input_vector(input->append("linearAttenuation"));
    *(float *)(p + _linear_attenuation_offset) = v[0];
  }
  if (_quadratic_attenuation_offset >= 0) {
    LVecBase4 v = target_shader->get_shader_input_vector(input->append("quadraticAttenuation"));
    *(float *)(p + _quadratic_attenuation_offset) = v[0];
  }
  if (_radius_offset >= 0) {
    LVecBase4 v = target_shader->get_shader_input_vector(input->append("radius"));
    *(float *)(p + _radius_offset) = v[0];
  }
  if (_shadow_view_matrix_offset >= 0) {
    target_shader->get_shader_input_matrix(input->append("shadowViewMatrix"), *(LMatrix4f *)(p + _shadow_view_matrix_offset));
  }
}

/**
 * Returns an opaque resource identifier that can later be used to fetch the
 * nth resource, which is of the given type.
 */
ShaderInputBinding::ResourceId ShaderLightStructBinding::
get_resource_id(int index) const {
  if (_input != nullptr) {
    nassertr(index == 0, 0);
  }
  return index;
}

/**
 * Fetches the texture associated with this shader input.
 */
PT(Texture) ShaderLightStructBinding::
fetch_texture(const State &state, ResourceId resource_id, SamplerState &sampler, int &view) const {
  NodePath light;
  if (_input != nullptr) {
    if (state.gsg->get_target_shader_attrib()->has_shader_input(_input)) {
      light = state.gsg->get_target_shader_attrib()->get_shader_input_nodepath(_input);
    } else {
      // Maybe it's a data struct after all, even though it looked like a light
      // struct initially.
      return state.gsg->get_target_shader_attrib()->get_shader_input_texture(((InternalName *)_input.p())->append("shadowMap"), &sampler);
    }
  } else {
    const LightAttrib *target_light;
    size_t index = (size_t)resource_id;
    if (state.gsg->get_target_state()->get_attrib(target_light) && index < target_light->get_num_non_ambient_lights()) {
      light = target_light->get_on_light(index);
    }
  }

  PT(Texture) tex;
  if (!light.is_empty()) {
    tex = state.gsg->get_shadow_map(light);
  } else {
    // There is no such light assigned.  Bind a dummy shadow map.
    tex = state.gsg->get_dummy_shadow_map(_cube_shadow_map);
  }
  if (tex != nullptr) {
    sampler = tex->get_default_sampler();
  }
  return tex;
}

/**
 * Returns a mask indicating which state changes should cause the parameter to
 * be respecified.
 */
int ShaderTextureStagesBinding::
get_state_dep() const {
  return Shader::D_texture;
}

/**
 * Returns an opaque resource identifier that can later be used to fetch the
 * nth resource, which is of the given type.
 */
ShaderInputBinding::ResourceId ShaderTextureStagesBinding::
get_resource_id(int index) const {
  return index;
}

/**
 * Fetches the texture associated with this shader input.
 */
PT(Texture) ShaderTextureStagesBinding::
fetch_texture(const State &state, ResourceId resource_id, SamplerState &sampler, int &view) const {
  int index = (int)resource_id;

  // We get the TextureAttrib directly from the _target_rs, not the
  // filtered TextureAttrib in _target_texture.
  const TextureAttrib *texattrib;
  if (state.gsg->get_target_state()->get_attrib(texattrib) && index < texattrib->get_num_on_stages()) {
    int si = 0;
    for (int i = 0; i < texattrib->get_num_on_stages(); ++i) {
      TextureStage *stage = texattrib->get_on_stage(i);
      TextureStage::Mode mode = stage->get_mode();

      if ((1 << mode) & _mode_mask) {
        if (si++ == index) {
          sampler = texattrib->get_on_sampler(stage);
          view += stage->get_tex_view_offset();
          PT(Texture) tex = texattrib->get_on_texture(stage);
          if (tex->get_texture_type() != _desired_type) {
            shader_cat.error()
              << "Texture " << *tex
              << " does not match type desired by shader\n";
          }
          return tex;
        }
      }
    }
  }

  return _default_texture;
}

/**
 * Returns a mask indicating which state changes should cause the parameter to
 * be respecified.
 */
int ShaderTextureBinding::
get_state_dep() const {
  return Shader::D_frame | Shader::D_shader_inputs;
}

/**
 * Returns an opaque resource identifier that can later be used to fetch the
 * nth resource, which is of the given type.
 */
ShaderInputBinding::ResourceId ShaderTextureBinding::
get_resource_id(int index) const {
  return (ResourceId)_input.p();
}

/**
 * Fetches the texture associated with this shader input.
 */
PT(Texture) ShaderTextureBinding::
fetch_texture(const State &state, ResourceId resource_id, SamplerState &sampler, int &view) const {
  const InternalName *name = (const InternalName *)resource_id;
  PT(Texture) tex = state.gsg->get_target_shader_attrib()->get_shader_input_texture(name, &sampler);
#ifndef NDEBUG
  if (!_shown_error && tex->get_texture_type() != _desired_type) {
    _shown_error = true;
    shader_cat.error()
      << "Sampler type of shader input '" << *name << "' does not "
         "match type of texture " << *tex << ".\n";
    // TODO: also check whether shadow sampler textures have shadow filter
    // enabled.
  }
#endif
  return tex;
}

/**
 * Fetches the texture that should be bound as a storage image.
 */
PT(Texture) ShaderTextureBinding::
fetch_texture_image(const State &state, ResourceId resource_id, ShaderType::Access &access, int &z, int &n) const {
  const InternalName *name = (const InternalName *)resource_id;
  PT(Texture) tex = state.gsg->get_target_shader_attrib()->get_shader_input_texture_image(name, access, z, n);
#ifndef NDEBUG
  if (!_shown_error && tex->get_texture_type() != _desired_type) {
    _shown_error = true;
    shader_cat.error()
      << "Sampler type of shader input '" << *name << "' does not "
         "match type of texture " << *tex << ".\n";
    // TODO: also check whether shadow sampler textures have shadow filter
    // enabled.
  }
#endif
  return tex;
}

/**
 * Returns a mask indicating which state changes should cause the parameter to
 * be respecified.
 */
int ShaderBufferBinding::
get_state_dep() const {
  // We don't specify D_frame, because we don't (yet) support updating shader
  // buffers from the CPU.
  return Shader::D_shader_inputs;
}

/**
 * Returns an opaque resource identifier that can later be used to fetch the
 * nth resource, which is of the given type.
 */
ShaderInputBinding::ResourceId ShaderBufferBinding::
get_resource_id(int index) const {
  return (ResourceId)_input.p();
}

/**
 * Fetches the shader buffer associated with the given resource identifier,
 * which was previously returned by get_resource_id.
 */
PT(ShaderBuffer) ShaderBufferBinding::
fetch_shader_buffer(const State &state, ResourceId resource_id) const {
  const InternalName *name = (const InternalName *)resource_id;
  PT(ShaderBuffer) buffer = state.gsg->get_target_shader_attrib()->get_shader_input_buffer(name);
#ifndef NDEBUG
  if (!_shown_error && buffer->get_data_size_bytes() < _min_size) {
    _shown_error = true;
    shader_cat.error()
      << *buffer << " is too small for shader input " << *name
      << " (expected at least " << _min_size << " bytes)\n";
  }
#endif
  return buffer;
}

/**
 * Returns a mask indicating which state changes should cause the parameter to
 * be respecified.
 */
int ShaderDataBinding::
get_state_dep() const {
  // We specify SD_frame because a PTA may be modified by the app from
  // frame to frame, and we have no way to know.  So, we must respecify a
  // PTA at least once every frame.
  return Shader::D_frame | Shader::D_shader_inputs;
}

/**
 * Fetches the part of the shader input that is plain numeric data.
 */
void ShaderFloatBinding::
fetch_data(const State &state, void *into, bool packed) const {
  Shader::ShaderPtrData ptr_data;
  if (!state.gsg->get_target_shader_attrib()->get_shader_input_ptr(_input, ptr_data)) {
    return;
  }

  int total_rows = std::min(_num_elements * _num_rows, (int)ptr_data._size / _num_cols);
  if (total_rows == 1 || _num_cols == 4) {
    packed = true;
  }

  float *data = (float *)into;

  switch (ptr_data._type) {
  case ShaderType::ST_int:
    // Convert int data to float data.
    if (packed) {
      for (int i = 0; i < total_rows * _num_cols; ++i) {
        data[i] = (float)(((int *)ptr_data._ptr)[i]);
      }
    } else {
      const int *from_data = (const int *)ptr_data._ptr;
      for (int i = 0; i < total_rows; ++i) {
        for (int c = 0; c < _num_cols; ++c) {
          data[i * 4 + c] = (float)*from_data++;
        }
      }
    }
    return;

  case ShaderType::ST_uint:
    // Convert unsigned int data to float data.
    if (packed) {
      for (int i = 0; i < total_rows * _num_cols; ++i) {
        data[i] = (float)(((unsigned int *)ptr_data._ptr)[i]);
      }
    } else {
      const unsigned int *from_data = (const unsigned int *)ptr_data._ptr;
      for (int i = 0; i < total_rows; ++i) {
        for (int c = 0; c < _num_cols; ++c) {
          data[i * 4 + c] = (float)*from_data++;
        }
      }
    }
    return;

  case ShaderType::ST_double:
    // Downgrade double data to float data.
    if (packed) {
      for (int i = 0; i < total_rows * _num_cols; ++i) {
        data[i] = (float)(((double *)ptr_data._ptr)[i]);
      }
    } else {
      const double *from_data = (const double *)ptr_data._ptr;
      for (int i = 0; i < total_rows; ++i) {
        for (int c = 0; c < _num_cols; ++c) {
          data[i * 4 + c] = (float)*from_data++;
        }
      }
    }
    return;

  case ShaderType::ST_float:
    if (packed) {
      // No conversion needed.
      memcpy(data, ptr_data._ptr, total_rows * _num_cols * sizeof(float));
      return;
      /*if (always_copy) {
        memcpy(data, ptr_data._ptr, total_rows * _num_cols * sizeof(float));
        return;
      } else {
        return (float *)ptr_data._ptr;
      }*/
    } else {
      const float *from_data = (const float *)ptr_data._ptr;
      for (int i = 0; i < total_rows; ++i) {
        for (int c = 0; c < _num_cols; ++c) {
          data[i * 4 + c] = (float)*from_data++;
        }
      }
    }
    return;

  default:
#ifndef NDEBUG
    shader_cat.error()
      << "Invalid ShaderPtrData type " << (int)ptr_data._type
      << " for shader input '" << *_input << "'\n";
#endif
  }
}

/**
 * Fetches the part of the shader input that is plain numeric data.
 */
void ShaderDoubleBinding::
fetch_data(const State &state, void *into, bool packed) const {
  Shader::ShaderPtrData ptr_data;
  if (!state.gsg->get_target_shader_attrib()->get_shader_input_ptr(_input, ptr_data)) {
    return;
  }

  int total_rows = std::min(_num_elements * _num_rows, (int)ptr_data._size / _num_cols);
  if (total_rows == 1 || _num_cols == 4) {
    packed = true;
  }

  double *data = (double *)into;

  switch (ptr_data._type) {
  case ShaderType::ST_int:
    // Convert int data to double data.
    if (packed) {
      for (int i = 0; i < total_rows * _num_cols; ++i) {
        data[i] = (double)(((int *)ptr_data._ptr)[i]);
      }
    } else {
      const int *from_data = (const int *)ptr_data._ptr;
      for (int i = 0; i < total_rows; ++i) {
        for (int c = 0; c < _num_cols; ++c) {
          data[i * 4 + c] = (double)*from_data++;
        }
      }
    }
    return;

  case ShaderType::ST_uint:
    // Convert int data to double data.
    if (packed) {
      for (int i = 0; i < total_rows * _num_cols; ++i) {
        data[i] = (double)(((unsigned int *)ptr_data._ptr)[i]);
      }
    } else {
      const int *from_data = (const int *)ptr_data._ptr;
      for (int i = 0; i < total_rows; ++i) {
        for (int c = 0; c < _num_cols; ++c) {
          data[i * 4 + c] = (double)*from_data++;
        }
      }
    }
    return;

  case ShaderType::ST_double:
    if (packed) {
      // No conversion needed.
      //if (always_copy) {
        memcpy(data, ptr_data._ptr, total_rows * _num_cols * sizeof(double));
        return;
      //} else {
      //  return (double *)ptr_data._ptr;
      //}
    } else {
      const double *from_data = (const double *)ptr_data._ptr;
      for (int i = 0; i < total_rows; ++i) {
        for (int c = 0; c < _num_cols; ++c) {
          data[i * 4 + c] = (double)*from_data++;
        }
      }
    }
    return;

  case ShaderType::ST_float:
    // Upgrade float data to double data.
    if (packed) {
      for (int i = 0; i < total_rows * _num_cols; ++i) {
        data[i] = (double)(((float *)ptr_data._ptr)[i]);
      }
    } else {
      const float *from_data = (const float *)ptr_data._ptr;
      for (int i = 0; i < total_rows; ++i) {
        for (int c = 0; c < _num_cols; ++c) {
          data[i * 4 + c] = (double)*from_data++;
        }
      }
    }
    return;

  default:
#ifndef NDEBUG
    shader_cat.error()
      << "Invalid ShaderPtrData type " << (int)ptr_data._type
      << " for shader input '" << *_input << "'\n";
#endif
  }
}

/**
 * Fetches the part of the shader input that is plain numeric data.
 */
void ShaderIntBinding::
fetch_data(const State &state, void *into, bool packed) const {
  Shader::ShaderPtrData ptr_data;
  if (!state.gsg->get_target_shader_attrib()->get_shader_input_ptr(_input, ptr_data)) {
    return;
  }

  if (ptr_data._type != ShaderType::ST_int &&
      ptr_data._type != ShaderType::ST_uint &&
      ptr_data._type != ShaderType::ST_bool) {
    shader_cat.error()
      << "Cannot pass floating-point data to integer shader input '" << *_input << "'\n";
    return;
  }

  int total_rows = std::min(_num_elements * _num_rows, (int)ptr_data._size / _num_cols);
  if (total_rows == 1 || _num_cols == 4) {
    packed = true;
  }

  if (packed) {
    memcpy(into, ptr_data._ptr, total_rows * _num_cols * sizeof(int));
  } else {
    int *data = (int *)into;
    const int *from_data = (const int *)ptr_data._ptr;
    for (int i = 0; i < total_rows; ++i) {
      for (int c = 0; c < _num_cols; ++c) {
        data[i * 4 + c] = *from_data++;
      }
    }
  }
}

/**
 * Fetches the part of the shader input that is plain numeric data.
 */
void ShaderBoolBinding::
fetch_data(const State &state, void *into, bool packed) const {
  Shader::ShaderPtrData ptr_data;
  if (!state.gsg->get_target_shader_attrib()->get_shader_input_ptr(_input, ptr_data)) {
    return;
  }

  int total_rows = std::min(_num_elements * _num_rows, (int)ptr_data._size / _num_cols);
  if (total_rows == 1 || _num_cols == 4) {
    packed = true;
  }

  uint32_t *data = (uint32_t *)into;

  switch (ptr_data._type) {
  case ShaderType::ST_int:
  case ShaderType::ST_uint:
  case ShaderType::ST_bool:
    // Convert int data to bool data.
    if (packed) {
      for (int i = 0; i < total_rows * _num_cols; ++i) {
        data[i] = (uint32_t)(((unsigned int *)ptr_data._ptr)[i] != 0);
      }
    } else {
      const int *from_data = (const int *)ptr_data._ptr;
      for (int i = 0; i < total_rows; ++i) {
        for (int c = 0; c < _num_cols; ++c) {
          data[i * 4 + c] = (uint32_t)(*from_data++ != 0);
        }
      }
    }
    return;

  case ShaderType::ST_double:
    // Convert double data to bool data.
    if (packed) {
      for (int i = 0; i < total_rows * _num_cols; ++i) {
        data[i] = (uint32_t)(((double *)ptr_data._ptr)[i] != 0.0);
      }
    } else {
      const double *from_data = (const double *)ptr_data._ptr;
      for (int i = 0; i < total_rows; ++i) {
        for (int c = 0; c < _num_cols; ++c) {
          data[i * 4 + c] = (uint32_t)(*from_data++ != 0.0);
        }
      }
    }
    return;

  case ShaderType::ST_float:
    // Convert float data to bool data.
    if (packed) {
      for (int i = 0; i < total_rows * _num_cols; ++i) {
        data[i] = (uint32_t)(((float *)ptr_data._ptr)[i] != 0.0f);
      }
    } else {
      const float *from_data = (const float *)ptr_data._ptr;
      for (int i = 0; i < total_rows; ++i) {
        for (int c = 0; c < _num_cols; ++c) {
          data[i * 4 + c] = (uint32_t)(*from_data++ != 0.0f);
        }
      }
    }
    return;

  default:
#ifndef NDEBUG
    shader_cat.error()
      << "Invalid ShaderPtrData type " << (int)ptr_data._type
      << " for shader input '" << *_input << "'\n";
#endif
  }
}

/**
 * Returns a mask indicating which state changes should cause the parameter to
 * be respecified.
 */
int ShaderAggregateBinding::
get_state_dep() const {
  return Shader::D_frame | Shader::D_shader_inputs;
}

/**
 * Fetches the part of the shader input that is plain numeric data.
 */
void ShaderAggregateBinding::
fetch_data(const State &state, void *into, bool packed) const {
  // Note that the offsets are calculated for a non-packed layout.  That means
  // we have too much padding if we want packed data.  It's probably not worth
  // engineering a solution for that.
  for (const DataMember &member : _data_members) {
    member._binding->fetch_data(state, (unsigned char *)into + member._offset, packed);
  }
}

/**
 * Returns an opaque resource identifier that can later be used to fetch the
 * nth resource, which is of the given type.
 */
ShaderInputBinding::ResourceId ShaderAggregateBinding::
get_resource_id(int index) const {
  nassertr(index >= 0 && (size_t)index < _resources.size(), 0);
  return (ResourceId)_resources[index].p();
}

/**
 * Fetches the texture associated with this shader input.
 */
PT(Texture) ShaderAggregateBinding::
fetch_texture(const State &state, ResourceId resource_id, SamplerState &sampler, int &view) const {
  const InternalName *name = (const InternalName *)resource_id;
  return state.gsg->get_target_shader_attrib()->get_shader_input_texture(name, &sampler);
}

/**
 * Fetches the texture that should be bound as a storage image.
 */
PT(Texture) ShaderAggregateBinding::
fetch_texture_image(const State &state, ResourceId resource_id, ShaderType::Access &access, int &z, int &n) const {
  const InternalName *name = (const InternalName *)resource_id;
  return state.gsg->get_target_shader_attrib()->get_shader_input_texture_image(name, access, z, n);
}

/**
 * Fetches the shader buffer associated with the given resource identifier,
 * which was previously returned by get_resource_id.
 */
PT(ShaderBuffer) ShaderAggregateBinding::
fetch_shader_buffer(const State &state, ResourceId resource_id) const {
  // GLSL does not support SSBOs in structs, but they can be in arrays, and we
  // might add support for another shader language that does support this.
  const InternalName *name = (const InternalName *)resource_id;
  return state.gsg->get_target_shader_attrib()->get_shader_input_buffer(name);
}

/**
 * Unwraps the aggregate type, storing the individual members.
 */
void ShaderAggregateBinding::
r_collect_members(const InternalName *name, const ShaderType *type, size_t offset) {
  ShaderType::ScalarType scalar_type;
  uint32_t arg_dim[3];
  if (type->as_scalar_type(scalar_type, arg_dim[0], arg_dim[1], arg_dim[2])) {
    ShaderDataBinding *binding;
    switch (scalar_type) {
    case ShaderType::ST_float:
      binding = new ShaderFloatBinding(name, arg_dim[0], arg_dim[1], arg_dim[2]);
      break;

    case ShaderType::ST_double:
      binding = new ShaderDoubleBinding(name, arg_dim[0], arg_dim[1], arg_dim[2]);
      break;

    case ShaderType::ST_bool:
      binding = new ShaderBoolBinding(name, arg_dim[0], arg_dim[1], arg_dim[2]);
      break;

    default:
      binding = new ShaderIntBinding(name, arg_dim[0], arg_dim[1], arg_dim[2]);
      break;
    }

    _data_members.push_back({binding, offset});
  }
  else if (const ShaderType::Struct *struct_type = type->as_struct()) {
    for (size_t i = 0; i < struct_type->get_num_members(); ++i) {
      const ShaderType::Struct::Member &member = struct_type->get_member(i);

      PT(InternalName) fqname = ((InternalName *)name)->append(member.name);
      r_collect_members(fqname, member.type, offset + member.offset);
    }
  }
  else if (const ShaderType::Array *array_type = type->as_array()) {
    size_t basename_size = name->get_basename().size();
    char *buffer = (char *)alloca(basename_size + 14);
    memcpy(buffer, name->get_basename().c_str(), basename_size);

    const ShaderType *element_type = array_type->get_element_type();
    size_t stride = (size_t)array_type->get_stride_bytes();

    for (uint32_t ai = 0; ai < array_type->get_num_elements(); ++ai) {
      sprintf(buffer + basename_size, "[%d]", (int)ai);

      PT(InternalName) fqname = name->get_parent()->append(buffer);
      r_collect_members(fqname, element_type, offset);
      offset += stride;
    }
  }
  else if (type->as_resource() != nullptr) {
    _resources.push_back(name);
  }
}

/**
 * Creates the appropriate binding for the input with the given name and type.
 */
ShaderInputBinding *
make_binding_glsl(const InternalName *name, const ShaderType *type) {
  std::string name_str = name->get_name();

  // Split it at the underscores.
  vector_string pieces;
  tokenize(name_str, pieces, "_");
  nassertr(!pieces.empty(), nullptr);

  // Check if it has a p3d_ prefix - if so, assign special meaning.
  if (pieces[0] == "p3d") {
    // Check for matrix inputs.
    bool transpose = false;
    bool inverse = false;
    string matrix_name = pieces[1];

    // Check for and chop off any "Transpose" or "Inverse" suffix.
    if (matrix_name.size() > 6 + 9 &&
        matrix_name.compare(matrix_name.size() - 9, 9, "Transpose") == 0) {
      transpose = true;
      matrix_name = matrix_name.substr(0, matrix_name.size() - 9);
    }
    if (matrix_name.size() > 6 + 7 &&
        matrix_name.compare(matrix_name.size() - 7, 7, "Inverse") == 0) {
      inverse = true;
      matrix_name = matrix_name.substr(0, matrix_name.size() - 7);
    }

    // Now if the suffix that is left over is "Matrix", we know that it is
    // supposed to be a matrix input.
    if (matrix_name.size() > 6 &&
        matrix_name.compare(matrix_name.size() - 6, 6, "Matrix") == 0) {

      if (!expect_float_matrix(name, type, 3, 4)) {
        return nullptr;
      }

      Shader::StateMatrix part[2] = {
        Shader::SM_identity,
        Shader::SM_identity,
      };
      if (matrix_name == "ModelViewProjectionMatrix") {
        if (inverse) {
          part[0] = Shader::SM_apiclip_to_apiview;
          part[1] = Shader::SM_apiview_to_model;
        } else {
          part[0] = Shader::SM_model_to_apiview;
          part[1] = Shader::SM_apiview_to_apiclip;
        }
      }
      else if (matrix_name == "ModelViewMatrix") {
        part[0] = inverse ? Shader::SM_apiview_to_model
                          : Shader::SM_model_to_apiview;
      }
      else if (matrix_name == "ProjectionMatrix") {
        part[0] = inverse ? Shader::SM_apiclip_to_apiview
                          : Shader::SM_apiview_to_apiclip;
      }
      else if (matrix_name == "NormalMatrix") {
        // This is really the upper 3x3 of the ModelViewMatrixInverseTranspose.
        transpose = !transpose;
        part[0] = inverse ? Shader::SM_model_to_apiview
                          : Shader::SM_apiview_to_model;

        if (!expect_float_matrix(name, type, 3, 3)) {
          return nullptr;
        }
      }
      else if (matrix_name == "ModelMatrix") {
        if (inverse) {
          part[0] = Shader::SM_world_to_view;
          part[1] = Shader::SM_view_to_model;
        } else {
          part[0] = Shader::SM_model_to_view;
          part[1] = Shader::SM_view_to_world;
        }
      }
      else if (matrix_name == "ViewMatrix") {
        if (inverse) {
          part[0] = Shader::SM_apiview_to_view;
          part[1] = Shader::SM_view_to_world;
        } else {
          part[0] = Shader::SM_world_to_view;
          part[1] = Shader::SM_view_to_apiview;
        }
      }
      else if (matrix_name == "ViewProjectionMatrix") {
        if (inverse) {
          part[0] = Shader::SM_apiclip_to_view;
          part[1] = Shader::SM_view_to_world;
        } else {
          part[0] = Shader::SM_world_to_view;
          part[1] = Shader::SM_view_to_apiclip;
        }
      }
      else if (matrix_name == "TextureMatrix") {
        // We may support 2-D texmats later, but let's make sure that people
        // don't think they can just use a mat3 to get the 2-D version.
        if (!expect_float_matrix(name, type, 4, 4)) {
          return nullptr;
        }

        return make_texture_matrix(type, 0, inverse, transpose);
      }
      else {
        return report_parameter_error(name, type, "unrecognized matrix name");
      }

      return make_matrix_compose(type, part[0], nullptr, part[1], nullptr, transpose);
    }
    if (pieces[1].compare(0, 7, "Texture") == 0) {
      const ShaderType *element_type;
      uint32_t num_elements;
      type->unwrap_array(element_type, num_elements);

      const ShaderType::SampledImage *sampled_image_type = element_type->as_sampled_image();
      if (sampled_image_type == nullptr) {
        return report_parameter_error(name, type, "expected sampled image");
      }

      if (pieces[1].size() > 7 && isdigit(pieces[1][7])) {
        // p3d_Texture0, p3d_Texture1, etc.
        std::string tail;
        int stage = string_to_int(pieces[1].substr(7), tail);
        if (!tail.empty()) {
          string msg = "unexpected '" + tail + "'";
          return report_parameter_error(name, type, msg.c_str());
        }

        return make_texture_stage(type, stage);
      }
      else {
        // p3d_Texture[] or p3d_TextureModulate[], etc.
        Texture *default_texture = get_white_texture();
        unsigned int mode_mask;
        if (pieces[1].size() == 7) {
          mode_mask = ~0u;
        }
        else if (pieces[1].compare(7, string::npos, "FF") == 0) {
          mode_mask = (1 << TextureStage::M_normal) - 1;
        }
        else if (pieces[1].compare(7, string::npos, "Modulate") == 0) {
          mode_mask = (1 << TextureStage::M_modulate)
                    | (1 << TextureStage::M_modulate_glow)
                    | (1 << TextureStage::M_modulate_gloss);
        }
        else if (pieces[1].compare(7, string::npos, "Add") == 0) {
          static PT(Texture) default_add_tex;
          if (default_add_tex == nullptr) {
            PT(Texture) tex = new Texture("default-add");
            tex->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_luminance);
            tex->set_clear_color(LColor(0, 0, 0, 1));
            default_add_tex = std::move(tex);
          }
          default_texture = default_add_tex.p();
          mode_mask = (1 << TextureStage::M_add);
        }
        else if (pieces[1].compare(7, string::npos, "Normal") == 0) {
          mode_mask = (1 << TextureStage::M_normal)
                    | (1 << TextureStage::M_normal_height);
        }
        else if (pieces[1].compare(7, string::npos, "Height") == 0) {
          mode_mask = (1 << TextureStage::M_height)
                    | (1 << TextureStage::M_normal_height);
        }
        else if (pieces[1].compare(7, string::npos, "MetallicRoughness") == 0 ||
                 pieces[1].compare(7, string::npos, "Selector") == 0) {
          mode_mask = (1 << TextureStage::M_metallic_roughness)
                    | (1 << TextureStage::M_occlusion_metallic_roughness);
        }
        else if (pieces[1].compare(7, string::npos, "Gloss") == 0) {
          mode_mask = (1 << TextureStage::M_gloss)
                    | (1 << TextureStage::M_modulate_gloss)
                    | (1 << TextureStage::M_normal_gloss);
        }
        else if (pieces[1].compare(7, string::npos, "Emission") == 0) {
          mode_mask = (1 << TextureStage::M_emission);
        }
        else if (pieces[1].compare(7, string::npos, "Occlusion") == 0) {
          mode_mask = (1 << TextureStage::M_occlusion)
                    | (1 << TextureStage::M_occlusion_metallic_roughness);
        }
        else {
          return report_parameter_error(name, type, "unrecognized parameter name");
        }

        if (pieces[1].size() > 7 && (mode_mask & (1 << TextureStage::M_normal_height)) != 0) {
          static PT(Texture) default_normal_height_tex;
          if (default_normal_height_tex == nullptr) {
            PT(Texture) tex = new Texture("default-normal-height");
            tex->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_rgba);
            tex->set_clear_color(LColor(0.5, 0.5, 1, 0));
            default_normal_height_tex = std::move(tex);
          }
          default_texture = default_normal_height_tex.p();
        }

        return new ShaderTextureStagesBinding(sampled_image_type->get_texture_type(), num_elements, default_texture, mode_mask);
      }
    }
    if (pieces[1] == "Material") {
      const ShaderType::Struct *struct_type = type->as_struct();
      if (struct_type == nullptr) {
        return report_parameter_error(name, type, "expected struct");
      }
      return make_material(type);
    }
    if (pieces[1] == "ColorScale") {
      if (!expect_float_vector(name, type, 3, 4)) {
        return nullptr;
      }
      return make_color_scale(type);
    }
    if (pieces[1] == "Color") {
      if (!expect_float_vector(name, type, 3, 4)) {
        return nullptr;
      }
      return make_color(type);
    }
    if (pieces[1] == "ClipPlane") {
      const ShaderType *element_type;
      uint32_t num_elements;
      type->unwrap_array(element_type, num_elements);
      if (!expect_float_vector(name, element_type, 4, 4)) {
        return nullptr;
      }

      return ShaderInputBinding::make_data(Shader::D_clip_planes | Shader::D_view_transform,
                                           [=](const State &state, void *into, bool packed) {

        LPlanef *planes = (LPlanef *)into;

        size_t i = 0;

        const ClipPlaneAttrib *cpa;
        if (state.gsg->get_target_state()->get_attrib(cpa)) {
          SceneSetup *scene_setup = state.gsg->get_scene();
          size_t num_planes = std::min((size_t)num_elements, (size_t)cpa->get_num_on_planes());
          for (; i < num_planes; ++i) {
            const NodePath &plane = cpa->get_on_plane(i);
            nassertv(!plane.is_empty());
            const PlaneNode *plane_node;
            DCAST_INTO_V(plane_node, plane.node());

            CPT(TransformState) transform =
              scene_setup->get_cs_world_transform()->compose(
                plane.get_transform(scene_setup->get_scene_root().get_parent()));

            planes[i] = LCAST(float, plane_node->get_plane() * transform->get_mat());
          }
        }

        for (; i < num_elements; ++i) {
          // Fill the remainder with zeroes.
          planes[i].set(0, 0, 0, 0);
        }
      });
    }
    if (pieces[1] == "TexAlphaOnly") {
      // This is a hack so we can support both F_alpha and other formats in the
      // default shader, to fix font rendering in GLES2
      const ShaderType *element_type;
      uint32_t num_elements = 1;
      type->unwrap_array(element_type, num_elements);

      return ShaderInputBinding::make_data(Shader::D_texture | Shader::D_frame,
                                           [=](const State &state, void *into, bool packed) {

        const TextureAttrib *ta;

        uint32_t num_stages = 0;
        if (state.gsg->get_target_state()->get_attrib(ta)) {
          num_stages = std::min(num_elements, (uint32_t)ta->get_num_on_stages());
        }

        uint32_t i = 0;
        for (; i < num_stages; ++i) {
          TextureStage *ts = ta->get_on_stage(i);
          PN_stdfloat v = (ta->get_on_texture(ts)->get_format() == Texture::F_alpha);
          ((LVecBase4f *)into)[i].set(v, v, v, 0);
        }
        for (; i < num_elements; ++i) {
          ((LVecBase4f *)into)[i].set(0, 0, 0, 0);
        }
      });
    }
    if (pieces[1] == "Fog") {
      const ShaderType::Struct *struct_type = type->as_struct();
      if (struct_type == nullptr) {
        return report_parameter_error(name, type, "expected struct");
      }
      return make_fog(struct_type);
    }
    if (pieces[1] == "LightModel") {
      const ShaderType::Struct *struct_type = type->as_struct();
      if (struct_type == nullptr || struct_type->get_num_members() != 1) {
        return report_parameter_error(name, type, "expected struct with 1 member");
      }

      const ShaderType::Struct::Member &member = struct_type->get_member(0);
      if (member.name != "ambient") {
        return report_parameter_error(name, type, "expected 'ambient' member");
      }

      CPT(InternalName) fqname = ((InternalName *)name)->append(member.name);
      if (!expect_float_vector(fqname, member.type, 3, 4)) {
        return nullptr;
      }

      return make_light_ambient(member.type);
    }
    if (pieces[1] == "NumLights") {
      if (type != ShaderType::int_type) {
        return report_parameter_error(name, type, "expected int");
      }
      return ShaderInputBinding::make_data(Shader::D_light,
                                           [](const State &state, void *into, bool packed) {
        size_t num_lights = 0;
        const LightAttrib *target_light;
        if (state.gsg->get_target_state()->get_attrib(target_light)) {
          num_lights = target_light->get_num_non_ambient_lights();
        }
        *(int *)into = (int)num_lights;
      });
    }
    if (pieces[1] == "LightSource") {
      const ShaderType::Array *array = type->as_array();
      if (array == nullptr) {
        return report_parameter_error(name, type, "expected array of structs");
      }

      const ShaderType::Struct *struct_type = array->get_element_type()->as_struct();
      if (struct_type == nullptr) {
        return report_parameter_error(name, type, "expected array of structs");
      }

      bool success = true;
      for (size_t i = 0; i < struct_type->get_num_members(); ++i) {
        const ShaderType::Struct::Member &member = struct_type->get_member(i);

        if (!check_light_struct_member(member.name, member.type)) {
          PT(InternalName) fqname = ((InternalName *)name)->append(member.name);
          report_parameter_error(fqname, member.type, "not a valid light struct member");
          success = false;
        }
      }
      if (!success) {
        return nullptr;
      }

      return new ShaderLightStructBinding(type);
    }
    if (pieces[1] == "TransformTable") {
      const ShaderType *element_type;
      uint32_t num_elements;
      type->unwrap_array(element_type, num_elements);

      const ShaderType::Matrix *matrix = element_type->as_matrix();
      if (matrix == nullptr ||
          matrix->get_num_rows() != 4 ||
          matrix->get_num_columns() != 4 ||
          matrix->get_scalar_type() != ShaderType::ST_float) {
        return report_parameter_error(name, type, "expected mat4[]");
      }

      return make_transform_table(type, false);
    }
    if (pieces[1] == "SliderTable") {
      const ShaderType *element_type;
      uint32_t num_elements;
      type->unwrap_array(element_type, num_elements);

      if (element_type != ShaderType::float_type) {
        return report_parameter_error(name, type, "expected float");
      }

      return make_slider_table(type);
    }

    return report_parameter_error(name, type, "unrecognized parameter name");
  }

  if (pieces[0] == "osg") {
    if (!expect_num_words(name, type, 2)) {
      return nullptr;
    }

    // These inputs are supported by OpenSceneGraph.  We can support them as
    // well, to increase compatibility.
    if (pieces[1] == "ViewMatrix") {
      return make_matrix(type, Shader::SM_world_to_apiview, nullptr);
    }
    else if (pieces[1] == "InverseViewMatrix" || pieces[1] == "ViewMatrixInverse") {
      return make_matrix_compose(type, Shader::SM_apiview_to_view, nullptr,
                                       Shader::SM_view_to_world, nullptr);
    }
    else if (pieces[1] == "FrameTime") {
      if (!expect_float_vector(name, type, 1, 1, true)) {
        return nullptr;
      }
      return make_frame_time(type);
    }
    else if (pieces[1] == "DeltaFrameTime") {
      if (type == ShaderType::float_type) {
        return ShaderInputBinding::make_data(Shader::D_frame,
                                             [](const State &state, void *into, bool packed) {
          *(float *)into = ClockObject::get_global_clock()->get_dt();
        });
      }
      else if (type == ShaderType::double_type) {
        return ShaderInputBinding::make_data(Shader::D_frame,
                                             [](const State &state, void *into, bool packed) {
          *(double *)into = ClockObject::get_global_clock()->get_dt();
        });
      }
      else {
        return report_parameter_error(name, type, "expected float");
      }
    }
    else if (pieces[1] == "FrameNumber") {
      if (type == ShaderType::int_type) {
        return ShaderInputBinding::make_data(Shader::D_frame,
                                             [](const State &state, void *into, bool packed) {
          *(int *)into = ClockObject::get_global_clock()->get_frame_count();
        });
      } else {
        return report_parameter_error(name, type, "expected int");
      }
    }
    else {
      return report_parameter_error(name, type, "unrecognized parameter name");
    }
  }

  // Check for mstrans, wstrans, vstrans, cstrans, mspos, wspos, vspos, cspos
  if (pieces[0].size() >= 5 &&
      (pieces[0].compare(1, std::string::npos, "strans") == 0 ||
       pieces[0].compare(1, std::string::npos, "spos") == 0)) {
    pieces.push_back("to");

    switch (pieces[0][0]) {
    case 'm':
      pieces.push_back("model");
      break;
    case 'w':
      pieces.push_back("world");
      break;
    case 'v':
      pieces.push_back("view");
      break;
    case 'c':
      pieces.push_back("clip");
      break;
    default:
      return nullptr;
    }
    if (pieces[0].compare(1, std::string::npos, "strans") == 0) {
      pieces[0] = "trans";
    } else {
      pieces[0] = "row3";
    }
  }
  else if (pieces[0].size() == 3 && // mat_modelproj et al
           (pieces[0] == "mat" || pieces[0] == "inv" ||
            pieces[0] == "tps" || pieces[0] == "itp")) {
    std::string trans = pieces[0];
    std::string matrix = pieces[1];
    pieces.clear();
    if (matrix == "modelview") {
      tokenize("trans_model_to_apiview", pieces, "_");
    }
    else if (matrix == "projection") {
      tokenize("trans_apiview_to_apiclip", pieces, "_");
    }
    else if (matrix == "modelproj") {
      tokenize("trans_model_to_apiclip", pieces, "_");
    }
    else {
      return report_parameter_error(name, type, "unrecognized matrix name");
    }
    if (trans == "mat") {
      pieces[0] = "trans";
    } else if (trans == "inv") {
      string t = pieces[1];
      pieces[1] = pieces[3];
      pieces[3] = t;
    } else if (trans == "tps") {
      pieces[0] = "tpose";
    } else if (trans == "itp") {
      string t = pieces[1];
      pieces[1] = pieces[3];
      pieces[3] = t;
      pieces[0] = "tpose";
    }
  }

  // Implement the Cg-style transform-matrix generator.
  if (pieces[0] == "trans" ||
      pieces[0] == "tpose" ||
      pieces[0] == "row0" ||
      pieces[0] == "row1" ||
      pieces[0] == "row2" ||
      pieces[0] == "row3" ||
      pieces[0] == "col0" ||
      pieces[0] == "col1" ||
      pieces[0] == "col2" ||
      pieces[0] == "col3") {

    bool transpose = false;
    int offset = 0;
    if (pieces[0] == "trans") {
      if (!expect_float_matrix(name, type, 3, 4)) {
        return nullptr;
      }
    }
    else if (pieces[0] == "tpose") {
      if (!expect_float_matrix(name, type, 3, 4)) {
        return nullptr;
      }
      transpose = true;
    }
    else {
      if (!expect_float_vector(name, type, 4, 4)) {
        return nullptr;
      }
      if (pieces[0][0] == 'r') {
        offset = (pieces[0][3] - '0') * 4;
      }
      else if (pieces[0][0] == 'c') {
        offset = pieces[0][3] - '0';
      }
      else {
        nassertr(false, nullptr);
      }
    }

    Shader::StateMatrix part[2];
    CPT(InternalName) arg[2];

    int next = 1;
    pieces.push_back("");
    if (!expect_coordinate_system(name, type, pieces, next, true, part, arg)) {
      return nullptr;
    }
    if (pieces[next] != "to" && pieces[next] != "rel") {
      return report_parameter_error(name, type, "expected 'to' or 'rel'");
    }
    ++next;
    if (!expect_coordinate_system(name, type, pieces, next, false, part, arg)) {
      return nullptr;
    }
    if (pieces.size() > (size_t)(next + 1)) {
      return report_parameter_error(name, type,
        "unexpected extra words after parameter name");
    }

    return make_matrix_compose(type, part[0], arg[0], part[1], arg[1], transpose, offset);
  }

  // If we get here, it's not a specially recognized input, but just a regular
  // user-defined input.
  return make_shader_input(type, name);
}

/**
 * Creates the appropriate binding for the input with the given name and type.
 */
ShaderInputBinding *
make_binding_cg(const InternalName *name, const ShaderType *type) {
  std::string name_str = name->get_name();

  // Split it at the underscores.
  vector_string pieces;
  tokenize(name_str, pieces, "_");
  nassertr(!pieces.empty(), nullptr);

  // Check for mstrans, wstrans, vstrans, cstrans, mspos, wspos, vspos, cspos
  if (pieces[0].size() >= 5 &&
      (pieces[0].compare(1, std::string::npos, "strans") == 0 ||
       pieces[0].compare(1, std::string::npos, "spos") == 0)) {
    pieces.push_back("to");

    switch (pieces[0][0]) {
    case 'm':
      pieces.push_back("model");
      break;
    case 'w':
      pieces.push_back("world");
      break;
    case 'v':
      pieces.push_back("view");
      break;
    case 'c':
      pieces.push_back("clip");
      break;
    default:
      return nullptr;
    }
    if (pieces[0].compare(1, std::string::npos, "strans") == 0) {
      pieces[0] = "trans";
    } else {
      pieces[0] = "row3";
    }
  }
  else if (pieces[0].size() == 3 && // mat_modelproj et al
           (pieces[0] == "mat" || pieces[0] == "inv" ||
            pieces[0] == "tps" || pieces[0] == "itp")) {
    std::string trans = pieces[0];
    std::string matrix = pieces[1];
    pieces.clear();
    if (matrix == "modelview") {
      tokenize("trans_model_to_apiview", pieces, "_");
    }
    else if (matrix == "projection") {
      tokenize("trans_apiview_to_apiclip", pieces, "_");
    }
    else if (matrix == "modelproj") {
      tokenize("trans_model_to_apiclip", pieces, "_");
    }
    else if (matrix == "shadow") {
      if (!expect_num_words(name, type, 3) ||
          !expect_float_matrix(name, type, 4, 4)) {
        return nullptr;
      }
      return make_matrix_compose(type, Shader::SM_view_to_world, nullptr,
                                 Shader::SM_world_to_apiclip_light_i,
                                 InternalName::make(pieces[2]), true);
    }
    else {
      return report_parameter_error(name, type, "unrecognized matrix name");
    }
    if (trans == "mat") {
      pieces[0] = "trans";
    } else if (trans == "inv") {
      string t = pieces[1];
      pieces[1] = pieces[3];
      pieces[3] = t;
    } else if (trans == "tps") {
      pieces[0] = "tpose";
    } else if (trans == "itp") {
      string t = pieces[1];
      pieces[1] = pieces[3];
      pieces[3] = t;
      pieces[0] = "tpose";
    }
  }

  // Implement the Cg-style transform-matrix generator.
  if (pieces[0] == "trans" ||
      pieces[0] == "tpose" ||
      pieces[0] == "row0" ||
      pieces[0] == "row1" ||
      pieces[0] == "row2" ||
      pieces[0] == "row3" ||
      pieces[0] == "col0" ||
      pieces[0] == "col1" ||
      pieces[0] == "col2" ||
      pieces[0] == "col3") {

    bool transpose = false;
    int offset = 0;
    if (pieces[0] == "trans") {
      if (!expect_float_matrix(name, type, 3, 4)) {
        return nullptr;
      }
    }
    else if (pieces[0] == "tpose") {
      if (!expect_float_matrix(name, type, 3, 4)) {
        return nullptr;
      }
      transpose = true;
    }
    else {
      if (!expect_float_vector(name, type, 4, 4)) {
        return nullptr;
      }
      if (pieces[0][0] == 'r') {
        offset = (pieces[0][3] - '0') * 4;
      }
      else if (pieces[0][0] == 'c') {
        offset = pieces[0][3] - '0';
      }
      else {
        nassertr(false, nullptr);
      }
    }

    Shader::StateMatrix part[2];
    CPT(InternalName) arg[2];

    int next = 1;
    pieces.push_back("");
    if (!expect_coordinate_system(name, type, pieces, next, true, part, arg)) {
      return nullptr;
    }
    if (pieces[next] != "to" && pieces[next] != "rel") {
      return report_parameter_error(name, type, "expected 'to' or 'rel'");
    }
    ++next;
    if (!expect_coordinate_system(name, type, pieces, next, false, part, arg)) {
      return nullptr;
    }
    if (pieces.size() > (size_t)(next + 1)) {
      return report_parameter_error(name, type,
        "unexpected extra words after parameter name");
    }

    return make_matrix_compose(type, part[0], arg[0], part[1], arg[1], !transpose, offset);
  }

  // Other Cg-specific inputs.
  //bool k_prefix = false;
  //if (name_str.size() >= 2 && name_str.substr(0, 2) == "__") {
  //  return true;
  //}

  // Special parameter: attr_material or attr_color
  if (pieces[0] == "attr") {
    if (!expect_num_words(name, type,  2)) {
      return nullptr;
    }

    if (pieces[1] == "material") {
      if (!expect_float_matrix(name, type, 4, 4)) {
        return nullptr;
      }
      return ShaderInputBinding::make_data(Shader::D_material | Shader::D_frame,
                                           [=](const State &state, void *into, bool packed) {

        LVecBase4f &ambient = ((LVecBase4f *)into)[0];
        LVecBase4f &diffuse = ((LVecBase4f *)into)[1];
        LVecBase4f &emission = ((LVecBase4f *)into)[2];
        LVecBase4f &specular = ((LVecBase4f *)into)[3];

        const MaterialAttrib *target_material;
        if (state.gsg->get_target_state()->get_attrib(target_material) && !target_material->is_off()) {
          Material *m = target_material->get_material();
          ambient = LCAST(float, m->get_ambient());
          diffuse = LCAST(float, m->get_diffuse());
          emission = LCAST(float, m->get_emission());
          specular = LCAST(float, m->get_specular());
          specular[3] = m->get_shininess();
        } else {
          ambient.set(1, 1, 1, 1);
          diffuse.set(1, 1, 1, 1);
          emission.set(0, 0, 0, 0);
          specular.set(0, 0, 0, 0);
        }
      });
    }
    else if (pieces[1] == "color") {
      if (!expect_float_vector(name, type, 3, 4)) {
        return nullptr;
      }
      return make_color(type);
    }
    else if (pieces[1] == "colorscale") {
      if (!expect_float_vector(name, type, 3, 4)) {
        return nullptr;
      }
      return make_color_scale(type);
    }
    else if (pieces[1] == "fog") {
      if (!expect_float_vector(name, type, 3, 4)) {
        return nullptr;
      }
      return ShaderInputBinding::make_data(Shader::D_fog | Shader::D_frame,
                                           [](const State &state, void *into, bool packed) {

        LVecBase4f &params = *(LVecBase4f *)into;

        const FogAttrib *target_fog;
        if (state.gsg->get_target_state()->get_attrib(target_fog) && target_fog->get_fog() != nullptr) {
          Fog *fog = target_fog->get_fog();
          PN_stdfloat start, end;
          fog->get_linear_range(start, end);
          params.set(fog->get_exp_density(), start, end, 1.0f / (end - start));
        } else {
          params.set(0, 1, 1, 1);
        }
      });
    }
    else if (pieces[1] == "fogcolor") {
      if (!expect_float_vector(name, type, 3, 4)) {
        return nullptr;
      }
      return ShaderInputBinding::make_data(Shader::D_fog | Shader::D_frame,
                                           [](const State &state, void *into, bool packed) {

        LVecBase4f &color = *(LVecBase4f *)into;

        const FogAttrib *target_fog;
        if (state.gsg->get_target_state()->get_attrib(target_fog) && target_fog->get_fog() != nullptr) {
          Fog *fog = target_fog->get_fog();
          color = LCAST(float, fog->get_color());
        } else {
          color.set(1, 1, 1, 1);
        }
      });
    }
    else if (pieces[1] == "ambient") {
      if (!expect_float_vector(name, type, 3, 4)) {
        return nullptr;
      }
      return make_light_ambient(type);
    }
    else if (pieces[1].compare(0, 5, "light") == 0) {
      int index = atoi(pieces[1].c_str() + 5);
      if (!expect_float_matrix(name, type, 4, 4)) {
        return nullptr;
      }
      return new ShaderPackedLightBinding((size_t)index);
    }
    else if (pieces[1].compare(0, 5, "lspec") == 0) {
      int index = atoi(pieces[1].c_str() + 5);
      if (!expect_float_vector(name, type, 3, 4)) {
        return nullptr;
      }
      return ShaderInputBinding::make_data(Shader::D_light | Shader::D_frame,
                                           [=](const State &state, void *into, bool packed) {

        // We don't count ambient lights, which would be pretty silly to handle
        // via this mechanism.
        const LightAttrib *target_light;
        if (state.gsg->get_target_state()->get_attrib(target_light) &&
            index < (int)target_light->get_num_non_ambient_lights()) {
          NodePath np = target_light->get_on_light(index);
          nassertv(!np.is_empty());
          PandaNode *node = np.node();
          Light *light = node->as_light();
          nassertv(light != nullptr);

          *(LVecBase4f *)into = LCAST(float, light->get_specular_color());
        } else {
          *(LVecBase4f *)into = LVecBase4f(0, 0, 0, 1);
        }
      });
    }
    else if (pieces[1] == "pointparams") {
      if (!expect_float_vector(name, type, 3, 4)) {
        return nullptr;
      }
      return new ShaderPointParamsBinding;
    }
    else {
      return report_parameter_error(name, type, "unrecognized parameter name");
    }
  }

  // Keywords to access light properties.
  if (pieces[0] == "alight") {
    if (!expect_num_words(name, type, 2) ||
        !expect_float_vector(name, type, 3, 4)) {
      return nullptr;
    }

    CPT(InternalName) input = InternalName::make(pieces[1]);
    return ShaderInputBinding::make_data(Shader::D_shader_inputs | Shader::D_frame,
                                         [=](const State &state, void *into, bool packed) {
      const NodePath &np = state.gsg->get_target_shader_attrib()->get_shader_input_nodepath(input);
      nassertv(!np.is_empty());
      Light *light = np.node()->as_light();
      nassertv(light != nullptr);
      *(LVecBase4f *)into = LCAST(float, light->get_color());
    });
  }

  if (pieces[0] == "satten") {
    if (!expect_num_words(name, type, 2) ||
        !expect_float_vector(name, type, 3, 4)) {
      return nullptr;
    }

    CPT(InternalName) input = InternalName::make(pieces[1]);
    return ShaderInputBinding::make_data(Shader::D_shader_inputs | Shader::D_frame,
                                         [=](const State &state, void *into, bool packed) {
      const NodePath &np = state.gsg->get_target_shader_attrib()->get_shader_input_nodepath(input);
      nassertv(!np.is_empty());
      Light *light = np.node()->as_light();
      nassertv(light != nullptr);
      *(LVecBase4f *)into = LVecBase4f(LCAST(float, light->get_attenuation()), light->get_exponent());
    });
  }

  if (pieces[0] == "dlight" || pieces[0] == "plight" || pieces[0] == "slight") {
    if (!expect_float_matrix(name, type, 4, 4)) {
      return nullptr;
    }
    int next = 1;
    pieces.push_back("");
    if (pieces[next] == "") {
      return report_parameter_error(name, type, "expected light input name");
    }
    Shader::StateMatrix part[2];
    CPT(InternalName) arg[2] {InternalName::make(pieces[next]), nullptr};
    next += 1;
    if (pieces[next] != "to" && pieces[next] != "rel") {
      return report_parameter_error(name, type, "expected 'to' or 'rel'");
    }
    if (!expect_coordinate_system(name, type, pieces, next, true, part, arg)) {
      return nullptr;
    }
    if ((int)pieces.size() > next) {
      return report_parameter_error(name, type,
        "unexpected extra words after parameter name");
    }

    if (pieces[0] == "dlight") {
      return new ShaderLegacyDirectionalLightBinding(std::move(arg[0]), part[1], std::move(arg[1]));
    }
    else if (pieces[0] == "plight") {
      return new ShaderLegacyPointLightBinding(std::move(arg[0]), part[1], std::move(arg[1]));
    }
    else if (pieces[0] == "slight") {
      return new ShaderLegacySpotlightBinding(std::move(arg[0]), part[1], std::move(arg[1]));
    }
    else {
      return nullptr;
    }
  }

  if (pieces[0] == "texmat") {
    if (!expect_num_words(name, type, 2) ||
        !expect_float_matrix(name, type, 4, 4)) {
      return nullptr;
    }
    return make_texture_matrix(type, atoi(pieces[1].c_str()), false, false);
  }

  if (pieces[0] == "texscale") {
    if (!expect_num_words(name, type, 2) ||
        !expect_float_vector(name, type, 3, 4)) {
      return nullptr;
    }

    int stage = atoi(pieces[1].c_str());
    return ShaderInputBinding::make_data(Shader::D_texture | Shader::D_tex_matrix,
                                         [=](const State &state, void *into, bool packed) {

      const TextureAttrib *ta;
      const TexMatrixAttrib *tma;
      if (state.gsg->get_target_state()->get_attrib(ta) && state.gsg->get_target_state()->get_attrib(tma) && stage < ta->get_num_on_stages()) {
        LVecBase3 scale = tma->get_transform(ta->get_on_stage(stage))->get_scale();
        ((LVecBase4f *)into)->set(scale[0], scale[1], scale[2], 0);
      } else {
        ((LVecBase4f *)into)->set(0, 0, 0, 1);
      }
    });
  }

  if (pieces[0] == "texcolor") {
    if (!expect_num_words(name, type, 2) ||
        !expect_float_vector(name, type, 3, 4)) {
      return nullptr;
    }

    // D_frame because the user may change a TextureStage's color without a
    // state change
    int stage = atoi(pieces[1].c_str());
    return ShaderInputBinding::make_data(Shader::D_texture | Shader::D_frame,
                                         [=](const State &state, void *into, bool packed) {

      const TextureAttrib *ta;
      if (state.gsg->get_target_state()->get_attrib(ta) && stage < ta->get_num_on_stages()) {
        TextureStage *ts = ta->get_on_stage(stage);
        *(LVecBase4f *)into = LCAST(float, ts->get_color());
      } else {
        ((LVecBase4f *)into)->set(0, 0, 0, 1);
      }
    });
  }

  if (pieces[0] == "texconst") {
    if (!expect_num_words(name, type, 2) ||
        !expect_float_vector(name, type, 3, 4)) {
      return nullptr;
    }

    // D_frame because the user may change a TextureStage's constant without
    // a state change
    int stage = atoi(pieces[1].c_str());
    return ShaderInputBinding::make_data(Shader::D_texture | Shader::D_tex_gen | Shader::D_frame,
                                         [=](const State &state, void *into, bool packed) {

      const TextureAttrib *ta;
      const TexGenAttrib *tga;
      if (state.gsg->get_target_state()->get_attrib(ta) && state.gsg->get_target_state()->get_attrib(tga) && stage < ta->get_num_on_stages()) {
        LVecBase3 value = tga->get_constant_value(ta->get_on_stage(stage));
        ((LVecBase4f *)into)->set(value[0], value[1], value[2], 1);
      } else {
        ((LVecBase4f *)into)->set(0, 0, 0, 1);
      }
    });
  }

  if (pieces[0] == "plane") {
    if (!expect_num_words(name, type, 2) ||
        !expect_float_vector(name, type, 4, 4)) {
      return nullptr;
    }

    // D_frame because the user may change a PlaneNode's plane without a
    // state change
    CPT(InternalName) input = InternalName::make(pieces[1]);
    return ShaderInputBinding::make_data(Shader::D_frame | Shader::D_shader_inputs,
                                         [=](const State &state, void *into, bool packed) {
      const NodePath &np = state.gsg->get_target_shader_attrib()->get_shader_input_nodepath(name);
      nassertv(!np.is_empty());
      const PlaneNode *plane_node;
      DCAST_INTO_V(plane_node, np.node());
      *(LVecBase4f *)into = LCAST(float, plane_node->get_plane());
    });
  }

  if (pieces[0] == "clipplane") {
    if (!expect_num_words(name, type, 2) ||
        !expect_float_vector(name, type, 4, 4)) {
      return nullptr;
    }

    // D_frame because the user may change a PlaneNode's plane without a
    // state change
    int index = atoi(pieces[1].c_str());
    return ShaderInputBinding::make_data(Shader::D_clip_planes | Shader::D_frame,
                                         [=](const State &state, void *into, bool packed) {
      const ClipPlaneAttrib *cpa;
      state.gsg->get_target_state()->get_attrib_def(cpa);
      if (index >= cpa->get_num_on_planes()) {
        (*(LVecBase4f *)into).set(0, 0, 0, 0);
        return;
      }
      const NodePath &np = cpa->get_on_plane(index);
      nassertv(!np.is_empty());
      const PlaneNode *plane_node;
      DCAST_INTO_V(plane_node, np.node());

      // Transform plane to world space
      CPT(TransformState) transform = np.get_net_transform();
      LPlane plane = plane_node->get_plane();
      if (!transform->is_identity()) {
        plane.xform(transform->get_mat());
      }
      *(LVecBase4f *)into = LCAST(float, plane);
    });
  }

  // Keywords to access unusual parameters.
  if (pieces[0] == "sys") {
    if (!expect_num_words(name, type, 2)) {
      return nullptr;
    }
    if (pieces[1] == "pixelsize" || pieces[1] == "windowsize") {
      if (!expect_float_vector(name, type, 2, 2)) {
        return nullptr;
      }
      return ShaderInputBinding::make_data(Shader::D_scene,
                                           [=](const State &state, void *into, bool packed) {
        const DisplayRegion *region = state.gsg->get_current_display_region();
        *(LVecBase2f *)into = LCAST(float, region->get_pixel_size());
      });
    }
    if (pieces[1] == "time") {
      if (!expect_float_vector(name, type, 1, 1, true)) {
        return nullptr;
      }
      return make_frame_time(type);
    }
    return report_parameter_error(name, type, "unrecognized parameter name");
  }

  // Keywords to access textures.
  if (pieces[0] == "tex") {
    CPT(InternalName) suffix;
    if (pieces.size() == 3) {
      suffix = InternalName::make(std::string("-") + pieces[2]);
      shader_cat.warning()
        << "Parameter " << *name << ": use of a texture suffix is deprecated.\n";
    }
    if (!expect_num_words(name, type, 2)) {
      return nullptr;
    }
    if (type->as_sampled_image() == nullptr) {
      return report_parameter_error(name, type, "expected sampler type");
    }
    return make_texture_stage(type, atoi(pieces[1].c_str()), std::move(suffix));
  }

  if (pieces[0] == "shadow") {
    if (!expect_num_words(name, type, 2)) {
      return nullptr;
    }
    const ShaderType::SampledImage *sampler = type->as_sampled_image();
    if (sampler == nullptr) {
      return report_parameter_error(name, type, "expected sampler type");
    }
    size_t index = (size_t)atoi(pieces[1].c_str());
    bool is_cube = sampler->get_texture_type() == Texture::TT_cube_map;

    return ShaderInputBinding::make_texture(Shader::D_frame | Shader::D_light,
                                            [=](const State &state, SamplerState &sampler, int &view) {

      const LightAttrib *target_light;
      state.gsg->get_target_state()->get_attrib_def(target_light);

      PT(Texture) tex;

      size_t num_lights = target_light->get_num_non_ambient_lights();
      if (index >= 0 && (size_t)index < num_lights) {
        tex = state.gsg->get_shadow_map(target_light->get_on_light(index));
      } else {
        // There is no such light assigned.  Bind a dummy shadow map.
        tex = state.gsg->get_dummy_shadow_map(is_cube);
      }
      if (tex != nullptr) {
        sampler = tex->get_default_sampler();
      }
      return tex;
    });
  }

  // Keywords to fetch texture parameter data.
  if (pieces[0] == "texpad") {
    if (!expect_num_words(name, type, 2) ||
        !expect_float_vector(name, type, 3, 4)) {
      return nullptr;
    }
    CPT(InternalName) input = InternalName::make(pieces[1]);
    return ShaderInputBinding::make_data(Shader::D_frame | Shader::D_shader_inputs,
                                         [=](const State &state, void *into, bool packed) {
      Texture *tex = state.gsg->get_target_shader_attrib()->get_shader_input_texture(input);
      nassertv(tex != nullptr);
      int sx = tex->get_x_size() - tex->get_pad_x_size();
      int sy = tex->get_y_size() - tex->get_pad_y_size();
      int sz = tex->get_z_size() - tex->get_pad_z_size();
      double cx = (sx * 0.5) / tex->get_x_size();
      double cy = (sy * 0.5) / tex->get_y_size();
      double cz = (sz * 0.5) / tex->get_z_size();
      ((LVecBase4f *)into)->set(cx, cy, cz, 0);
    });
  }

  if (pieces[0] == "texpix") {
    if (!expect_num_words(name, type, 2) ||
        !expect_float_vector(name, type, 2, 4)) {
      return nullptr;
    }
    CPT(InternalName) input = InternalName::make(pieces[1]);
    return ShaderInputBinding::make_data(Shader::D_frame | Shader::D_shader_inputs,
                                         [=](const State &state, void *into, bool packed) {
      Texture *tex = state.gsg->get_target_shader_attrib()->get_shader_input_texture(input);
      nassertv(tex != nullptr);
      double px = 1.0 / tex->get_x_size();
      double py = 1.0 / tex->get_y_size();
      double pz = 1.0 / tex->get_z_size();
      ((LVecBase4f *)into)->set(px, py, pz, 0);
    });
  }

  if (pieces[0] == "tbl") {
    const ShaderType *element_type;
    uint32_t num_elements;
    if (!expect_num_words(name, type, 2) ||
        !type->unwrap_array(element_type, num_elements)) {
      return report_parameter_error(name, type, "expected array");
    }

    if (pieces[1] == "transforms") {
      const ShaderType::Matrix *matrix = element_type->as_matrix();
      if (matrix == nullptr ||
          matrix->get_num_rows() < 3 ||
          matrix->get_num_columns() != 4 ||
          matrix->get_scalar_type() != ShaderType::ST_float) {
        return report_parameter_error(name, type, "expected float3x4[] or float4x4[]");
      }

      return make_transform_table(type, true);
    }
    else if (pieces[1] == "sliders") {
      return make_slider_table(type);
    }
    else {
      return report_parameter_error(name, type, "unrecognized parameter name");
    }
  }

  // Previously, custom shader inputs needed the k_ prefix, so we have to
  // strip it now.
  if (pieces[0] == "k") {
    //k_prefix = true;
    name_str = name_str.substr(2);
    return make_shader_input(type, InternalName::make(name_str));
  }

  // If we get here, it's not a specially recognized input, but just a regular
  // user-defined input.
  return make_shader_input(type, name);
}
