/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVInjectVertexTransformPass.cxx
 * @author rdb
 * @date 2026-02-17
 */

#include "spirVInjectVertexTransformPass.h"

/**
 *
 */
SpirVInjectVertexTransformPass::
SpirVInjectVertexTransformPass(bool make_new_entry_points, bool use_ssbo,
                               uint32_t buffer_binding, uint32_t buffer_set) :
  _make_new_entry_points(make_new_entry_points),
  _transform_block_binding(buffer_binding),
  _transform_block_set(buffer_set),
  _use_ssbo(use_ssbo) {
}

/**
 * Sets up animation support. locations is a bitmask of all vertex input
 * locations that should be transformed, point_locations the subset that is
 * transformed as a point.  The given index_location and weight_location
 * are for the index/weight vertex columns, which will be created with the given
 * locations if they don't yet exist.
 */
void SpirVInjectVertexTransformPass::
setup_animation(uint32_t locations, uint32_t point_locations,
                int index_location, int weight_location) {
  _anim_locations = locations;
  _anim_point_locations = point_locations;
  _transform_index_location = index_location;
  _transform_weight_location = weight_location;
}

/**
 * Sets up instancing via a vertex attribute.
 */
void SpirVInjectVertexTransformPass::
setup_instancing_attrib(int instance_mat_location) {
  _instance_mat_location = instance_mat_location;
}

/**
 *
 */
void SpirVInjectVertexTransformPass::
mark_model_matrix(uint32_t id, bool inverse, bool transpose) {
  _matrix_vars[id] = MatrixVar {0u, inverse, transpose};
}

/**
 * Called before any of the instructions are read.  Perform any pre-processing
 * based on the result database and the input arguments here.
 */
void SpirVInjectVertexTransformPass::
preprocess() {
  for (uint32_t id = 0; id < get_id_bound(); ++id) {
    const Definition &def = _db.get_definition(id);

    if (def.is_variable() && def._storage_class == spv::StorageClassInput) {
      if (def._location >= 0) {
        if (_transform_index_location == def._location) {
          const ShaderType::Vector *vector = def._type->as_vector();
          if (vector != nullptr &&
              vector->get_num_components() >= 4 &&
              (vector->get_scalar_type() == ShaderType::ST_uint || vector->get_scalar_type() == ShaderType::ST_int)) {
            _transform_index_var_id = id;
          }
        }
        else if (_transform_weight_location == def._location) {
          const ShaderType::Vector *vector = def._type->as_vector();
          if (vector != nullptr &&
              vector->get_num_components() >= 4 &&
              vector->get_scalar_type() == ShaderType::ST_float) {
            _transform_weight_var_id = id;
          }
        }
        else if (_instance_mat_location == def._location) {
          _instance_mat_var_id = id;
        }
        else if (_anim_locations & (1u << def._location)) {
          // This will be transformed.
          uint32_t new_id = allocate_id();
          _vertex_input_ids[id] = new_id;
        }
      }
      else if (def._builtin == spv::BuiltInInstanceIndex) {
        _instance_index_var_id = id;
      }
    }
  }
}

/**
 * Transforms an OpEntryPoint.
 * Return true to keep the instruction, false to omit it.
 */
bool SpirVInjectVertexTransformPass::
transform_entry_point(spv::ExecutionModel model, uint32_t id, const char *name, pvector<uint32_t> &vars) {
  if (model == spv::ExecutionModelVertex) {
    _todo_entry_points.insert({id, EntryPoint {std::string(name), vars}});
    return false;
  }
  return true;
}

/**
 *
 */
bool SpirVInjectVertexTransformPass::
transform_annotation_op(Instruction op) {
  // Move the decoration to point to a yet-to-be-constructed copy.
  if (op.opcode == spv::OpDecorate) {
    uint32_t var_id = op.args[0];

    auto it = _vertex_input_ids.find(var_id);
    if (it != _vertex_input_ids.end()) {
      op.args[0] = it->second;
    }
  }
  return SpirVTransformPass::transform_annotation_op(op);
}

/**
 *
 */
bool SpirVInjectVertexTransformPass::
transform_definition_op(Instruction op) {
  if (op.opcode == spv::OpVariable) {
    uint32_t var_id = op.args[1];
    spv::StorageClass storage_class = (spv::StorageClass)op.args[2];
    uint32_t new_id = 0;
    if (storage_class == spv::StorageClassInput) {
      auto it = _vertex_input_ids.find(var_id);
      if (it != _vertex_input_ids.end()) {
        if (it->second == 0) {
          it->second = allocate_id();
        }
        new_id = it->second;
      }
    }
    else if (storage_class == spv::StorageClassUniformConstant) {
      auto it = _matrix_vars.find(var_id);
      if (it != _matrix_vars.end()) {
        if (it->second._id == 0) {
          it->second._id = allocate_id();
        }
        new_id = it->second._id;
      }
    }

    if (new_id != 0u) {
      // We swap the ids so that the new private variable definition gets the
      // id that the input used to have, so that all loads from this point on
      // now point to the private variable.
      const ShaderType *type = resolve_pointer_type(op.args[0]);

      Definition &old_def = _db.modify_definition(var_id);
      Definition def = std::move(_db.modify_definition(var_id));
      old_def.clear();

      if (shader_cat.is_spam()) {
        shader_cat.spam()
          << "Changing storage class for variable " << var_id
          << " to Private and creating new variable " << new_id << "\n";
      }

      push_id(var_id);
      define_variable(type, spv::StorageClassPrivate);

      op.args[1] = new_id;
      _db.modify_definition(op.args[1]) = std::move(def);
    }
  }
  return SpirVTransformPass::transform_definition_op(op);
}

/**
 *
 */
bool SpirVInjectVertexTransformPass::
transform_function_op(Instruction op) {
  if (op.opcode == spv::OpLabel && !_make_new_entry_points) {
    auto it = _todo_entry_points.find(_current_function_id);
    if (it != _todo_entry_points.end()) {
      EntryPoint &entry_point = it->second;

      // First label in an existing entry point we're modifying.  Write out the
      // label and then insert the writes to the private vars.
      _new_functions.push_back((2 << spv::WordCountShift) | spv::OpLabel);
      _new_functions.push_back(op.args[0]);

      inject_animation(entry_point._vars);
      inject_instancing();

      pvector<uint32_t> entry_point_vars;
      bool has_transform_index = false;
      bool has_transform_weight = false;
      bool has_instance_matrix = false;
      bool has_instance_index = false;
      for (uint32_t var_id : entry_point._vars) {
        auto it = _vertex_input_ids.find(var_id);
        if (it != _vertex_input_ids.end()) {
          entry_point_vars.push_back(it->second);
        } else {
          entry_point_vars.push_back(var_id);
        }

        if (var_id == _transform_index_var_id) {
          has_transform_index = true;
        }
        if (var_id == _transform_weight_var_id) {
          has_transform_weight = true;
        }
        if (var_id == _instance_mat_var_id) {
          has_instance_matrix = true;
        }
        if (var_id == _instance_index_var_id) {
          has_instance_index = true;
        }
      }

      if (!has_transform_index) {
        entry_point_vars.push_back(_transform_index_var_id);
      }
      if (!has_transform_weight) {
        entry_point_vars.push_back(_transform_weight_var_id);
      }
      if (!_matrix_vars.empty()) {
        if (_instance_mat_var_id != 0) {
          if (!has_instance_matrix) {
            entry_point_vars.push_back(_instance_mat_var_id);
          }
        } else {
          if (!has_instance_index) {
            entry_point_vars.push_back(_instance_index_var_id);
          }
        }
      }

      add_entry_point(spv::ExecutionModelVertex, _current_function_id, entry_point._name, entry_point_vars);
      _todo_entry_points.erase(it);
      return false;
    }
  }

  return SpirVTransformPass::transform_function_op(op);
}

/**
 * Called after all instructions have been read, this does any post-processing
 * needed (such as updating the result database to reflect the transformations,
 * adding names/decorations, etc.)
 */
void SpirVInjectVertexTransformPass::
postprocess() {
  if (_todo_entry_points.empty()) {
    return;
  }

  for (auto &pair : _todo_entry_points) {
    uint32_t orig_func_id = pair.first;
    EntryPoint &entry_point = pair.second;

    // Determine the new entry point interface.
    pvector<uint32_t> entry_point_vars;
    bool has_transform_index = false;
    bool has_transform_weight = false;
    bool has_instance_matrix = false;
    bool has_instance_index = false;
    for (uint32_t var_id : entry_point._vars) {
      auto it = _vertex_input_ids.find(var_id);
      if (it != _vertex_input_ids.end()) {
        entry_point_vars.push_back(it->second);
      } else {
        entry_point_vars.push_back(var_id);
      }

      if (var_id == _transform_index_var_id) {
        has_transform_index = true;
      }
      if (var_id == _transform_weight_var_id) {
        has_transform_weight = true;
      }
      if (var_id == _instance_mat_var_id) {
        has_instance_matrix = true;
      }
      if (var_id == _instance_index_var_id) {
        has_instance_index = true;
      }
    }

    pvector<uint32_t> anim_entry_point_vars(entry_point_vars);

    {
      uint32_t func_id = op_function(nullptr);
      op_label();
      inject_instancing_noop();
      inject_animation_noop(entry_point._vars);
      op_function_call(orig_func_id);
      op_return();
      op_function_end();
      add_entry_point(spv::ExecutionModelVertex, func_id, entry_point._name, entry_point_vars);
    }

    if (_anim_locations != 0u) {
      uint32_t func_id = op_function(nullptr);
      op_label();
      inject_instancing_noop();
      inject_animation(entry_point._vars);
      op_function_call(orig_func_id);
      op_return();
      op_function_end();

      if (!has_transform_index) {
        anim_entry_point_vars.push_back(_transform_index_var_id);
      }
      if (!has_transform_weight) {
        anim_entry_point_vars.push_back(_transform_weight_var_id);
      }
      add_entry_point(spv::ExecutionModelVertex, func_id, entry_point._name + "_anim", anim_entry_point_vars);
    }

    if (!_matrix_vars.empty()) {
      uint32_t func_id = op_function(nullptr);
      op_label();
      inject_instancing();
      inject_animation_noop(entry_point._vars);
      op_function_call(orig_func_id);
      op_return();
      op_function_end();

      if (_instance_mat_var_id != 0u) {
        if (!has_instance_matrix) {
          entry_point_vars.push_back(_instance_mat_var_id);
        }
      } else {
        if (!has_instance_index) {
          entry_point_vars.push_back(_instance_index_var_id);
        }
      }
      add_entry_point(spv::ExecutionModelVertex, func_id, entry_point._name + "_inst", entry_point_vars);
    }

    if (!_matrix_vars.empty() && _anim_locations != 0u) {
      uint32_t func_id = op_function(nullptr);
      op_label();
      inject_instancing();
      inject_animation(entry_point._vars);
      op_function_call(orig_func_id);
      op_return();
      op_function_end();

      if (_instance_mat_var_id != 0u) {
        if (!has_instance_matrix) {
          anim_entry_point_vars.push_back(_instance_mat_var_id);
        }
      } else {
        if (!has_instance_index) {
          anim_entry_point_vars.push_back(_instance_index_var_id);
        }
      }
      add_entry_point(spv::ExecutionModelVertex, func_id, entry_point._name + "_anim_inst", anim_entry_point_vars);
    }
  }
  _todo_entry_points.clear();
}

/**
 * Injects the function preamble that applies the transformations.
 */
void SpirVInjectVertexTransformPass::
inject_animation(const pvector<uint32_t> &vars) {
  if (_anim_locations == 0u) {
    return;
  }

  // Make sure the skinning matrix buffer is defined.
  if (_transform_block_var_id == 0) {
    define_transform_block();
  }

  if (_transform_index_var_id == 0u) {
    const ShaderType *uvec4_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_uint, 4));
    _transform_index_var_id = define_variable(uvec4_type, spv::StorageClassInput);
    decorate(_transform_index_var_id, spv::DecorationLocation, _transform_index_location);
  }

  const ShaderType *vec4_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));
  if (_transform_weight_var_id == 0u) {
    _transform_weight_var_id = define_variable(vec4_type, spv::StorageClassInput);
    decorate(_transform_weight_var_id, spv::DecorationLocation, _transform_weight_location);
  }

  uint32_t transform_index = op_load(_transform_index_var_id);
  uint32_t transform_weight = op_load(_transform_weight_var_id);

  // How many components do we have?
  const ShaderType::Vector *transform_index_type = _db.get_definition(_transform_index_var_id)._type->as_vector();
  const ShaderType::Vector *transform_weight_type = _db.get_definition(_transform_weight_var_id)._type->as_vector();
  nassertv(transform_index_type != nullptr && transform_weight_type != nullptr);
  uint32_t num_components = std::min(transform_index_type->get_num_components(), transform_weight_type->get_num_components());

  if (_use_ssbo) {
    // Create an offset variable, add it to the indices.
    if (_transform_offset_var_id == 0u) {
      _transform_offset_var_id = define_variable(ShaderType::UINT, spv::StorageClassUniformConstant);
    }

    // Must broadcast to a vector before we can add it.
    uint32_t offset = op_load(_transform_offset_var_id);
    uint32_t vec_offset = op_composite_construct(transform_index_type,
      pvector<uint32_t>(transform_index_type->get_num_components(), offset));
    transform_index = op_add(transform_index, vec_offset);
  }

  // We can't add matrices together, we have to add them per vector.
  uint32_t accum_matrix_row0 = 0;
  uint32_t accum_matrix_row1 = 0;
  uint32_t accum_matrix_row2 = 0;

  uint32_t zero = define_int_constant(0);

  for (uint32_t i = 0; i < num_components; ++i) {
    // matrix = ubo.m[transform_index[i]]
    uint32_t matrix = op_load(op_access_chain(_transform_block_var_id, {zero, op_composite_extract(transform_index, {i})}));

    // matrix *= transform_weight[i]
    matrix = op_multiply(matrix, op_composite_extract(transform_weight, {i}));

    // Split into rows
    uint32_t matrix_row0 = op_composite_extract(matrix, {0});
    uint32_t matrix_row1 = op_composite_extract(matrix, {1});
    uint32_t matrix_row2 = op_composite_extract(matrix, {2});
    if (i == 0) {
      accum_matrix_row0 = matrix_row0;
      accum_matrix_row1 = matrix_row1;
      accum_matrix_row2 = matrix_row2;
    } else {
      accum_matrix_row0 = op_add(accum_matrix_row0, matrix_row0);
      accum_matrix_row1 = op_add(accum_matrix_row1, matrix_row1);
      accum_matrix_row2 = op_add(accum_matrix_row2, matrix_row2);
    }
  }

  const ShaderType *mat3x4_type = ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 4));
  uint32_t mat3x4 = op_composite_construct(mat3x4_type, {accum_matrix_row0, accum_matrix_row1, accum_matrix_row2});

  for (uint32_t private_vec_ptr : vars) {
    auto it = _vertex_input_ids.find(private_vec_ptr);
    if (it != _vertex_input_ids.end()) {
      uint32_t input_vec_ptr = it->second;

      const Definition &vec_def = _db.get_definition(input_vec_ptr);
      const ShaderType::Vector *vec_type = vec_def._type->as_vector();
      if (vec_type == nullptr || vec_type->get_num_components() < 3) {
        continue;
      }

      bool is_point = (_anim_point_locations & (1u << vec_def._location)) != 0;
      uint32_t w = define_float_constant(is_point);

      uint32_t orig_vec = op_load(input_vec_ptr);
      if (vec_type->get_num_components() == 3) {
        // Expand to vec4.
        orig_vec = op_composite_construct(vec4_type, {orig_vec, w});
      }

      uint32_t result = op_multiply(orig_vec, mat3x4);
      if (vec_type->get_num_components() == 4) {
        // It comes out as a vec3, so expand to vec4 again.
        result = op_composite_construct(vec4_type, {result, w});
      }

      op_store(private_vec_ptr, result);
    }
  }
}

/**
 * Injects the function preamble that does not apply the transformations.
 */
void SpirVInjectVertexTransformPass::
inject_animation_noop(const pvector<uint32_t> &vars) {
  for (uint32_t private_vec_ptr : vars) {
    auto it = _vertex_input_ids.find(private_vec_ptr);
    if (it != _vertex_input_ids.end()) {
      uint32_t input_vec_ptr = it->second;
      op_store(private_vec_ptr, op_load(input_vec_ptr));
    }
  }
}

/**
 *
 */
void SpirVInjectVertexTransformPass::
inject_instancing() {
  if (_matrix_vars.empty()) {
    return;
  }

  uint32_t inst_mat3x4;

  if (_instance_mat_location >= 0) {
    // Make sure the instance matrix vertex input is defined.
    if (_instance_mat_var_id == 0u) {
      _instance_mat_var_id = define_variable(ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 4)), spv::StorageClassInput);
      decorate(_instance_mat_var_id, spv::DecorationLocation, (uint32_t)_instance_mat_location);
    }

    inst_mat3x4 = op_load(_instance_mat_var_id);
  } else {
    // Make sure the transform SSBO is defined.
    if (_transform_block_var_id == 0) {
      define_transform_block();
    }
    if (_instance_index_var_id == 0) {
      _instance_index_var_id = define_variable(ShaderType::INT, spv::StorageClassInput);
      decorate(_instance_index_var_id, spv::DecorationBuiltIn, spv::BuiltInInstanceIndex);
    }

    uint32_t inst_index = op_load(_instance_index_var_id);
    uint32_t zero = define_int_constant(0);
    inst_mat3x4 = op_load(op_access_chain(_transform_block_var_id, {zero, inst_index}));
  }

  const ShaderType *mat3_type = ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 3));
  const ShaderType *vec4_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));

  // Do we need to invert the matrix?  We know that it is affine, so we
  // only have to invert the upper 3x3, which is more efficient.  We write
  // out the math for transpose(inverse(transpose(inst_mat))).
  bool need_inverse = false;
  bool need_inverse_translation = false;
  for (auto &pair : _matrix_vars) {
    if (pair.second._inverse) {
      need_inverse = true;

      const ShaderType::Matrix *matrix = _db.get_definition(pair.first)._type->as_matrix();
      if (pair.second._transpose ? matrix->get_num_columns() == 4 : matrix->get_num_rows() == 4) {
        need_inverse_translation = true;
      }
    }
  }
  uint32_t inv_inst_mat3 = 0u;
  uint32_t inv_inst_mat4 = 0u;
  uint32_t inv_inst_mat3x4 = 0u;
  if (need_inverse) {
    const ShaderType *mat3x4_type = ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 4));
    const ShaderType *vec3_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 3));

    uint32_t r0 = op_composite_construct(vec3_type, {
      op_sub(op_multiply(op_composite_extract(inst_mat3x4, {1, 1}), op_composite_extract(inst_mat3x4, {2, 2})),
             op_multiply(op_composite_extract(inst_mat3x4, {2, 1}), op_composite_extract(inst_mat3x4, {1, 2}))),
      op_sub(op_multiply(op_composite_extract(inst_mat3x4, {2, 1}), op_composite_extract(inst_mat3x4, {0, 2})),
             op_multiply(op_composite_extract(inst_mat3x4, {0, 1}), op_composite_extract(inst_mat3x4, {2, 2}))),
      op_sub(op_multiply(op_composite_extract(inst_mat3x4, {0, 1}), op_composite_extract(inst_mat3x4, {1, 2})),
             op_multiply(op_composite_extract(inst_mat3x4, {1, 1}), op_composite_extract(inst_mat3x4, {0, 2}))),
    });
    uint32_t r1 = op_composite_construct(vec3_type, {
      op_sub(op_multiply(op_composite_extract(inst_mat3x4, {1, 2}), op_composite_extract(inst_mat3x4, {2, 0})),
             op_multiply(op_composite_extract(inst_mat3x4, {2, 2}), op_composite_extract(inst_mat3x4, {1, 0}))),
      op_sub(op_multiply(op_composite_extract(inst_mat3x4, {2, 2}), op_composite_extract(inst_mat3x4, {0, 0})),
             op_multiply(op_composite_extract(inst_mat3x4, {0, 2}), op_composite_extract(inst_mat3x4, {2, 0}))),
      op_sub(op_multiply(op_composite_extract(inst_mat3x4, {0, 2}), op_composite_extract(inst_mat3x4, {1, 0})),
             op_multiply(op_composite_extract(inst_mat3x4, {1, 2}), op_composite_extract(inst_mat3x4, {0, 0}))),
    });
    uint32_t r2 = op_composite_construct(vec3_type, {
      op_sub(op_multiply(op_composite_extract(inst_mat3x4, {1, 0}), op_composite_extract(inst_mat3x4, {2, 1})),
             op_multiply(op_composite_extract(inst_mat3x4, {2, 0}), op_composite_extract(inst_mat3x4, {1, 1}))),
      op_sub(op_multiply(op_composite_extract(inst_mat3x4, {2, 0}), op_composite_extract(inst_mat3x4, {0, 1})),
             op_multiply(op_composite_extract(inst_mat3x4, {0, 0}), op_composite_extract(inst_mat3x4, {2, 1}))),
      op_sub(op_multiply(op_composite_extract(inst_mat3x4, {0, 0}), op_composite_extract(inst_mat3x4, {1, 1})),
             op_multiply(op_composite_extract(inst_mat3x4, {1, 0}), op_composite_extract(inst_mat3x4, {0, 1}))),
    });
    uint32_t one = define_float_constant(1);
    uint32_t inv_det = op_div(one, op_dot(r0, op_composite_construct(vec3_type, {
      op_composite_extract(inst_mat3x4, {0, 0}),
      op_composite_extract(inst_mat3x4, {1, 0}),
      op_composite_extract(inst_mat3x4, {2, 0}),
    })));

    if (need_inverse_translation) {
      // Handle the translation.
      uint32_t t = op_composite_construct(vec3_type, {
        op_composite_extract(inst_mat3x4, {0, 3}),
        op_composite_extract(inst_mat3x4, {1, 3}),
        op_composite_extract(inst_mat3x4, {2, 3}),
      });

      inv_inst_mat3x4 = op_multiply(op_composite_construct(mat3x4_type, {
        op_composite_construct(vec4_type, {r0, op_negate(op_dot(r0, t))}),
        op_composite_construct(vec4_type, {r1, op_negate(op_dot(r1, t))}),
        op_composite_construct(vec4_type, {r2, op_negate(op_dot(r2, t))}),
      }), inv_det);
    }
    else {
      // Don't bother constructing the last column.
      inv_inst_mat3 = op_multiply(op_composite_construct(mat3_type, {r0, r1, r2}), inv_det);
    }
  }
  uint32_t inst_mat3 = 0u;
  uint32_t inst_mat4 = 0u;

  for (auto &pair : _matrix_vars) {
    bool inverse = pair.second._inverse;
    bool transpose = pair.second._transpose;
    uint32_t orig_mat_ptr = pair.second._id;
    uint32_t result_mat_ptr = pair.first;

    uint32_t orig_mat = op_load(orig_mat_ptr);
    uint32_t inst_mat = 0u;

    // Compose the right matrix for multiplying into the target matrix.
    const ShaderType::Matrix *matrix = _db.get_definition(orig_mat_ptr)._type->as_matrix();
    uint32_t mat_size = transpose ? matrix->get_num_columns() : matrix->get_num_rows();

    if (matrix->get_num_rows() == 3 && matrix->get_num_columns() == 4 && transpose) {
      // Special case of affine mat * affine mat.
      pvector<uint32_t> rows {
        op_composite_extract(orig_mat, {0}),
        op_composite_extract(orig_mat, {1}),
        op_composite_extract(orig_mat, {2}),
      };

      uint32_t orig_mat3 = op_composite_construct(mat3_type, {
        op_vector_shuffle(rows[0], rows[0], {0, 1, 2}),
        op_vector_shuffle(rows[1], rows[1], {0, 1, 2}),
        op_vector_shuffle(rows[2], rows[2], {0, 1, 2}),
      });

      // mat3x4 * mat3 -> mat3x4
      uint32_t tmp_mat = op_multiply(inverse ? inv_inst_mat3x4 : inst_mat3x4, orig_mat3);

      // Now handle w (we assume that the matrix we modify isn't affine)
      pvector<uint32_t> result_rows;
      for (uint32_t i = 0; i < matrix->get_num_rows(); ++i) {
        uint32_t tmp_row = op_composite_extract(tmp_mat, {i});
        //uint32_t new_w = op_add(op_composite_extract(tmp_row, {3}), op_composite_extract(rows[i], {3}));
        uint32_t new_w = op_add(op_composite_extract(tmp_mat, {i, 3}), op_composite_extract(rows[i], {3}));
        //rows[i] = op_composite_insert(new_w, tmp_row, {3});
        rows[i] = op_composite_construct(vec4_type, {
          op_vector_shuffle(tmp_row, tmp_row, {0, 1, 2}),
          new_w,
        });
      }

      op_store(result_mat_ptr, op_composite_construct(matrix, rows));
      continue;
    }
    else if (mat_size == 4) {
      // Construct a 4x4 matrix.
      uint32_t &use_inst_mat4 = inverse ? inv_inst_mat4 : inst_mat4;
      if (use_inst_mat4 == 0) {
        const ShaderType *mat4_type = ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 4, 4));
        uint32_t zero = define_float_constant(0);
        uint32_t one = define_float_constant(1);

        uint32_t input = inverse ? inv_inst_mat3x4 : inst_mat3x4;
        uint32_t row0 = op_composite_extract(input, {0});
        uint32_t row1 = op_composite_extract(input, {1});
        uint32_t row2 = op_composite_extract(input, {2});
        uint32_t row3 = op_composite_construct(vec4_type, {zero, zero, zero, one});
        use_inst_mat4 = op_composite_construct(mat4_type, {row0, row1, row2, row3});
      }
      inst_mat = use_inst_mat4;
    }
    else {
      // Construct a 3x3 matrix without translation.
      uint32_t &use_inst_mat3 = inverse ? inv_inst_mat3 : inst_mat3;
      if (use_inst_mat3 == 0) {
        uint32_t input = inverse ? inv_inst_mat3x4 : inst_mat3x4;

        pvector<uint32_t> rows;
        for (uint32_t i = 0; i < mat_size; ++i) {
          uint32_t row = op_composite_extract(input, {i});
          rows.push_back(op_vector_shuffle(row, row, {0, 1, 2}));
        }
        use_inst_mat3 = op_composite_construct(mat3_type, rows);
      }
      inst_mat = use_inst_mat3;
    }

    uint32_t left, right;
    if (transpose) {
      // The incoming instance matrix is already transposed.
      left = inst_mat;
      right = orig_mat;
    } else {
      left = orig_mat;
      right = op_transpose(inst_mat);
    }

    op_store(result_mat_ptr, op_multiply(left, right));
  }
}

/**
 *
 */
void SpirVInjectVertexTransformPass::
inject_instancing_noop() {
  for (auto &pair : _matrix_vars) {
    uint32_t orig_mat_ptr = pair.second._id;
    uint32_t result_mat_ptr = pair.first;

    op_store(result_mat_ptr, op_load(orig_mat_ptr));
  }
}

/**
 * Makes sure the transform block type is defined.
 */
void SpirVInjectVertexTransformPass::
define_transform_block() {
  const ShaderType *block_type;
  if (_use_ssbo) {
    static const ShaderType *ssbo_block_type = make_transform_block_type(0);
    block_type = ssbo_block_type;
  } else {
    static const ShaderType *ubo_block_type = make_transform_block_type(256);
    block_type = ubo_block_type;
  }
  uint32_t block_type_id = define_type(block_type);
  uint32_t block_var_id = define_variable(block_type, spv::StorageClassUniform);

  decorate(block_type_id, _use_ssbo ? spv::DecorationBufferBlock : spv::DecorationBlock);
  decorate(block_var_id, spv::DecorationBinding, _transform_block_binding);
  decorate(block_var_id, spv::DecorationDescriptorSet, _transform_block_set);
  if (_use_ssbo) {
    decorate(block_var_id, spv::DecorationNonWritable);
  }
  _transform_block_var_id = block_var_id;
}

/**
 *
 */
const ShaderType *SpirVInjectVertexTransformPass::
make_transform_block_type(uint32_t num_elements) {
  // Because of alignment rules making it more efficient, we store the matrix
  // as a transposed 3x4 matrix.
  const ShaderType *mat3x4_type = ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 4));
  const ShaderType *array_type = ShaderType::register_type(ShaderType::Array(mat3x4_type, num_elements));
  ShaderType::Struct struct_type;
  struct_type.add_member(array_type, "m");
  return ShaderType::register_type(std::move(struct_type));
}
