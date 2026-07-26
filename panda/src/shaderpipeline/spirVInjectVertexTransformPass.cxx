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
  _matrix_vars[Id(id)] = MatrixVar {Id(), inverse, transpose};
}

/**
 *
 */
void SpirVInjectVertexTransformPass::
run(SpirVModule &module) {
  // Identify the vertex inputs of interest.
  for (uint32_t word = 0; word < module.get_id_bound(); ++word) {
    Id id(word);
    if (module.get_definition_type(id) != SpirVModule::DT_variable ||
        module.get_function_id(id) != 0 ||
        module.get_storage_class(id) != spv::StorageClassInput) {
      continue;
    }
    int location = module.get_location(id);
    if (location >= 0) {
      if (_transform_index_location == location) {
        const ShaderType *type = module.resolve_type(id);
        const ShaderType::Vector *vector = (type != nullptr) ? type->as_vector() : nullptr;
        if (vector != nullptr &&
            vector->get_num_components() >= 4 &&
            (vector->get_scalar_type() == ShaderType::ST_uint || vector->get_scalar_type() == ShaderType::ST_int)) {
          _transform_index_var_id = id;
        }
      }
      else if (_transform_weight_location == location) {
        const ShaderType *type = module.resolve_type(id);
        const ShaderType::Vector *vector = (type != nullptr) ? type->as_vector() : nullptr;
        if (vector != nullptr &&
            vector->get_num_components() >= 4 &&
            vector->get_scalar_type() == ShaderType::ST_float) {
          _transform_weight_var_id = id;
        }
      }
      else if (_instance_mat_location == location) {
        _instance_mat_var_id = id;
      }
      else if (_anim_locations & (1u << location)) {
        // This will be transformed.
        _vertex_input_ids[id] = module.allocate_id();
      }
    }
  }

  // Capture the vertex entry points.
  for (size_t i = 0; i < module.get_num_entry_points(); ++i) {
    const SpirVModule::EntryPoint &ep = module.get_entry_point(i);
    if (ep.model == spv::ExecutionModelVertex) {
      _todo_entry_points.insert({ep.function_id, ep.name});
    }
  }

  // Turn each affected variable into a private variable, and recreate the
  // input variable under a new id.  We swap the ids so that the private
  // variable gets the id that the input used to have, so that all loads from
  // this point on read the private variable.  The decorations move to the new
  // input; the name stays with the private variable.
  pvector<std::pair<Id, Id> > swaps;
  for (const auto &item : _vertex_input_ids) {
    swaps.push_back(std::make_pair(item.first, item.second));
  }
  for (auto &item : _matrix_vars) {
    item.second._id = module.allocate_id();
    swaps.push_back(std::make_pair(item.first, item.second._id));
  }

  for (const auto &swap : swaps) {
    Id var_id = swap.first;
    Id new_id = swap.second;

    const Instruction *decl = module.find_declaration(var_id);
    nassertd(decl != nullptr && decl->opcode == spv::OpVariable) continue;

    if (shader_cat.is_spam()) {
      shader_cat.spam()
        << "Changing storage class for variable " << var_id
        << " to Private and creating new variable " << new_id << "\n";
    }

    const ShaderType *type = module.resolve_type(Id(decl->args[0]));

    // The original declaration becomes the new input variable, along with
    // the decorations (location, builtin); the name stays with the private
    // variable declared under the old id below.
    module.reassign_declaration_id(var_id, new_id);
    module.transfer_annotations(var_id, new_id);

    // Declare the private variable under the old id.  The emit-time
    // topological sort takes care of the pointer type being declared after
    // the variable.
    Id pointer_type_id = module.define_pointer_type(type, spv::StorageClassPrivate);
    module.add_declaration(Instruction(spv::OpVariable, {
      pointer_type_id, var_id, (uint32_t)spv::StorageClassPrivate,
    }));

    _make_private_pointers.insert(var_id);
  }

  // We changed the storage class of these variables, so we must change the
  // storage class of the result type of any access chain formed from them.
  for (Function &function : module.modify_functions()) {
    for (Instruction &op : function.instructions) {
      if ((op.opcode == spv::OpAccessChain ||
           op.opcode == spv::OpInBoundsAccessChain ||
           op.opcode == spv::OpCopyObject) &&
          op.args.size() >= 3 && _make_private_pointers.count(Id(op.args[2]))) {
        Id type_id = module.unwrap_pointer_type(Id(op.args[0]));
        Id type_pointer_id = module.define_pointer_type(type_id, spv::StorageClassPrivate);
        op.args[0] = type_pointer_id;
        module.record_result(op, function.id);

        _make_private_pointers.insert(Id(op.args[1]));
      }
    }
  }

  if (!_make_new_entry_points) {
    // Modify the existing entry points in place: inject the transformations
    // at the top of each entry point function.
    for (const auto &item : _todo_entry_points) {
      Id function_id = item.first;

      nassertd(module.find_function(function_id) != nullptr) continue;

      // Inject at the top of the body: after the entry block's label and
      // past the OpVariable declarations that must lead it.
      SpirVBuilder builder(module);
      builder.set_insertion_point_to_body_start(function_id);
      inject_animation(builder);
      inject_instancing(builder);
    }
    _todo_entry_points.clear();
    return;
  }

  // Create new entry points, in all the required variants, that wrap the
  // original entry point functions.
  {
    // The original entry points are replaced by the wrappers.
    pvector<SpirVModule::EntryPoint> &entry_points = module.modify_entry_points();
    for (size_t i = entry_points.size(); i-- > 0;) {
      if (entry_points[i].model == spv::ExecutionModelVertex) {
        entry_points.erase(entry_points.begin() + i);
      }
    }
  }

  for (const auto &item : _todo_entry_points) {
    Id orig_func_id = item.first;
    const std::string &name = item.second;

    {
      SpirVBuilder builder =
        module.make_entry_point(spv::ExecutionModelVertex, name);
      inject_instancing_noop(builder);
      inject_animation_noop(builder);
      builder.op_function_call(orig_func_id);
      builder.op_return();
    }

    if (_anim_locations != 0u) {
      SpirVBuilder builder =
        module.make_entry_point(spv::ExecutionModelVertex, name + "_anim");
      inject_instancing_noop(builder);
      inject_animation(builder);
      builder.op_function_call(orig_func_id);
      builder.op_return();
    }

    if (!_matrix_vars.empty()) {
      SpirVBuilder builder =
        module.make_entry_point(spv::ExecutionModelVertex, name + "_inst");
      inject_instancing(builder);
      inject_animation_noop(builder);
      builder.op_function_call(orig_func_id);
      builder.op_return();
    }

    if (!_matrix_vars.empty() && _anim_locations != 0u) {
      SpirVBuilder builder =
        module.make_entry_point(spv::ExecutionModelVertex, name + "_anim_inst");
      inject_instancing(builder);
      inject_animation(builder);
      builder.op_function_call(orig_func_id);
      builder.op_return();
    }
  }
  _todo_entry_points.clear();
}

/**
 * Injects the function preamble that applies the animation transformations
 * at the given builder's cursor.
 */
void SpirVInjectVertexTransformPass::
inject_animation(SpirVBuilder &builder) {
  if (_anim_locations == 0u) {
    return;
  }

  SpirVModule &module = builder.get_module();

  // Make sure the skinning matrix buffer is defined.
  if (_transform_block_var_id == 0) {
    define_transform_block(module);
  }

  if (_transform_index_var_id == 0u) {
    const ShaderType *uvec4_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_uint, 4));
    _transform_index_var_id = module.define_variable(uvec4_type, spv::StorageClassInput);
    module.decorate(_transform_index_var_id, spv::DecorationLocation, _transform_index_location);
    module.set_name(_transform_index_var_id, "transform_index");
  }

  const ShaderType *vec4_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));
  if (_transform_weight_var_id == 0u) {
    _transform_weight_var_id = module.define_variable(vec4_type, spv::StorageClassInput);
    module.decorate(_transform_weight_var_id, spv::DecorationLocation, _transform_weight_location);
    module.set_name(_transform_weight_var_id, "transform_weight");
  }

  Value transform_index = Pointer(builder, _transform_index_var_id).load();
  Value transform_weight = Pointer(builder, _transform_weight_var_id).load();

  // How many components do we have?
  const ShaderType::Vector *transform_index_type = module.resolve_type(_transform_index_var_id)->as_vector();
  const ShaderType::Vector *transform_weight_type = module.resolve_type(_transform_weight_var_id)->as_vector();
  nassertv(transform_index_type != nullptr && transform_weight_type != nullptr);
  uint32_t num_components = std::min(transform_index_type->get_num_components(), transform_weight_type->get_num_components());

  if (_use_ssbo) {
    // Create an offset variable, add it to the indices.
    if (_transform_offset_var_id == 0u) {
      _transform_offset_var_id = module.define_variable(ShaderType::UINT, spv::StorageClassUniformConstant);
    }

    // The index vector may be signed or unsigned, so convert the offset to
    // its scalar type; the scalar then broadcasts to the component count.
    Value offset = Pointer(builder, _transform_offset_var_id).load();
    transform_index += offset.convert(transform_index_type->get_scalar_type());
  }

  // We can't add matrices together, we have to add them per vector.
  Value anim_mat_row0;
  Value anim_mat_row1;
  Value anim_mat_row2;

  Pointer transform_block(builder, _transform_block_var_id);

  for (uint32_t i = 0; i < num_components; ++i) {
    // matrix = ubo.m[transform_index[i]]
    Value matrix = transform_block[0][transform_index[i]].load();

    matrix *= transform_weight[i];

    // Split into rows
    if (i == 0) {
      anim_mat_row0 = matrix[0];
      anim_mat_row1 = matrix[1];
      anim_mat_row2 = matrix[2];
    } else {
      anim_mat_row0 += matrix[0];
      anim_mat_row1 += matrix[1];
      anim_mat_row2 += matrix[2];
    }
  }

  Value anim_mat = mat3x4(anim_mat_row0, anim_mat_row1, anim_mat_row2);

  for (const auto &item : _vertex_input_ids) {
    Id private_vec_ptr = item.first;
    Id input_vec_ptr = item.second;

    const ShaderType *input_type = module.resolve_type(input_vec_ptr);
    const ShaderType::Vector *vec_type = (input_type != nullptr) ? input_type->as_vector() : nullptr;
    if (vec_type == nullptr || vec_type->get_num_components() < 3) {
      continue;
    }

    bool is_point = (_anim_point_locations & (1u << module.get_location(input_vec_ptr))) != 0;
    double w = is_point ? 1.0 : 0.0;

    Value result = Pointer(builder, input_vec_ptr).load();
    if (vec_type->get_num_components() == 3) {
      // Expand to vec4.
      result = vec4(result, w);
    }

    result = result * anim_mat;
    if (vec_type->get_num_components() == 4) {
      // It comes out as a vec3, so expand to vec4 again.
      result = vec4(result, w);
    }

    Pointer(builder, private_vec_ptr).store(result);
  }
}

/**
 * Injects the function preamble that does not apply the transformations.
 */
void SpirVInjectVertexTransformPass::
inject_animation_noop(SpirVBuilder &builder) {
  for (const auto &item : _vertex_input_ids) {
    Pointer(builder, item.first).store(Pointer(builder, item.second).load());
  }
}

/**
 *
 */
void SpirVInjectVertexTransformPass::
inject_instancing(SpirVBuilder &builder) {
  if (_matrix_vars.empty()) {
    return;
  }

  SpirVModule &module = builder.get_module();

  Value inst_mat3x4;

  if (_instance_mat_location >= 0) {
    // Make sure the instance matrix vertex input is defined.
    if (_instance_mat_var_id == 0u) {
      _instance_mat_var_id = module.define_variable(ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 4)), spv::StorageClassInput);
      module.decorate(_instance_mat_var_id, spv::DecorationLocation, (uint32_t)_instance_mat_location);
    }

    inst_mat3x4 = Pointer(builder, _instance_mat_var_id).load();
  } else {
    // Make sure the transform SSBO is defined.
    if (_transform_block_var_id == 0) {
      define_transform_block(module);
    }

    Value inst_index = builder.ensure_builtin_input(spv::BuiltInInstanceIndex).load();
    Pointer transform_block(builder, _transform_block_var_id);
    inst_mat3x4 = transform_block[0][inst_index].load();
  }

  const ShaderType *vec4_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));

  // Do we need to invert the matrix?  We know that it is affine, so we
  // only have to invert the upper 3x3, which is more efficient.  We write
  // out the math for transpose(inverse(transpose(inst_mat))).
  bool need_inverse = false;
  bool need_inverse_translation = false;
  for (auto &pair : _matrix_vars) {
    if (pair.second._inverse) {
      need_inverse = true;

      const ShaderType::Matrix *matrix = module.resolve_type(pair.first)->as_matrix();
      if (pair.second._transpose ? matrix->get_num_columns() == 4 : matrix->get_num_rows() == 4) {
        need_inverse_translation = true;
      }
    }
  }
  Value inv_inst_mat3;
  Value inv_inst_mat4;
  Value inv_inst_mat3x4;
  if (need_inverse) {
    auto row0 = inst_mat3x4[0];
    auto row1 = inst_mat3x4[1];
    auto row2 = inst_mat3x4[2];

    auto r0 = vec3(row1[1] * row2[2] - row2[1] * row1[2],
                   row2[1] * row0[2] - row0[1] * row2[2],
                   row0[1] * row1[2] - row1[1] * row0[2]);
    auto r1 = vec3(row1[2] * row2[0] - row2[2] * row1[0],
                   row2[2] * row0[0] - row0[2] * row2[0],
                   row0[2] * row1[0] - row1[2] * row0[0]);
    auto r2 = vec3(row1[0] * row2[1] - row2[0] * row1[1],
                   row2[0] * row0[1] - row0[0] * row2[1],
                   row0[0] * row1[1] - row1[0] * row0[1]);
    auto inv_det = 1.0 / dot(r0, vec3(row0[0], row1[0], row2[0]));

    if (need_inverse_translation) {
      // Handle the translation.
      auto t = vec3(row0[3], row1[3], row2[3]);

      inv_inst_mat3x4 = mat3x4(vec4(r0, -dot(r0, t)),
                               vec4(r1, -dot(r1, t)),
                               vec4(r2, -dot(r2, t))) * inv_det;
    }
    else {
      // Don't bother constructing the last column.
      inv_inst_mat3 = mat3(r0, r1, r2) * inv_det;
    }
  }
  Value inst_mat3;
  Value inst_mat4;

  for (auto &pair : _matrix_vars) {
    bool inverse = pair.second._inverse;
    bool transpose = pair.second._transpose;
    Id orig_mat_ptr = pair.second._id;
    Id result_mat_ptr = pair.first;

    Value orig_mat = Pointer(builder, orig_mat_ptr).load();
    Value inst_mat;

    // Compose the right matrix for multiplying into the target matrix.
    const ShaderType::Matrix *matrix = module.resolve_type(orig_mat_ptr)->as_matrix();
    uint32_t mat_size = transpose ? matrix->get_num_columns() : matrix->get_num_rows();

    if (matrix->get_num_rows() == 3 && matrix->get_num_columns() == 4 && transpose) {
      // Special case of affine mat * affine mat.
      pvector<Value> rows {orig_mat[0], orig_mat[1], orig_mat[2]};

      Value orig_mat3 = mat3(rows[0].get_xyz(), rows[1].get_xyz(), rows[2].get_xyz());

      // mat3x4 * mat3 -> mat3x4
      Value tmp_mat = (inverse ? inv_inst_mat3x4 : inst_mat3x4) * orig_mat3;

      // Now handle w (we assume that the matrix we modify isn't affine)
      for (uint32_t i = 0; i < matrix->get_num_rows(); ++i) {
        Value tmp_row = tmp_mat[i];
        rows[i] = vec4(tmp_row.get_xyz(), tmp_row[3] + rows[i][3]);
      }

      Pointer(builder, result_mat_ptr).store(builder.construct(matrix, rows));
      continue;
    }
    else if (mat_size == 4) {
      // Construct a 4x4 matrix.
      Value &use_inst_mat4 = inverse ? inv_inst_mat4 : inst_mat4;
      if (!SpirVId(use_inst_mat4)) {
        Id zero = module.define_float_constant(0);
        Id one = module.define_float_constant(1);
        Id row3 = module.define_composite_constant(vec4_type, {zero, zero, zero, one});

        Value input = inverse ? inv_inst_mat3x4 : inst_mat3x4;
        use_inst_mat4 = mat4(input[0], input[1], input[2], Value(builder, row3));
      }
      inst_mat = use_inst_mat4;
    }
    else {
      // Construct a 3x3 matrix without translation.
      Value &use_inst_mat3 = inverse ? inv_inst_mat3 : inst_mat3;
      if (!SpirVId(use_inst_mat3)) {
        // Take the upper-left 3x3 part, dropping the translation column.
        use_inst_mat3 = mat3(inverse ? inv_inst_mat3x4 : inst_mat3x4);
      }
      inst_mat = use_inst_mat3;
    }

    Value left, right;
    if (transpose) {
      // The incoming instance matrix is already transposed.
      left = inst_mat;
      right = orig_mat;
    } else {
      left = orig_mat;
      right = glsl::transpose(inst_mat);
    }

    Pointer(builder, result_mat_ptr).store(left * right);
  }
}

/**
 *
 */
void SpirVInjectVertexTransformPass::
inject_instancing_noop(SpirVBuilder &builder) {
  for (auto &pair : _matrix_vars) {
    Id orig_mat_ptr = pair.second._id;
    Id result_mat_ptr = pair.first;

    Pointer(builder, result_mat_ptr).store(Pointer(builder, orig_mat_ptr).load());
  }
}

/**
 * Makes sure the transform block type is defined.
 */
void SpirVInjectVertexTransformPass::
define_transform_block(SpirVModule &module) {
  const ShaderType *block_type;
  if (_use_ssbo) {
    static const ShaderType *ssbo_block_type = make_transform_block_type(0);
    block_type = ssbo_block_type;
  } else {
    static const ShaderType *ubo_block_type = make_transform_block_type(256);
    block_type = ubo_block_type;
  }
  Id block_type_id = module.define_type(block_type);
  Id block_var_id = module.define_variable(block_type, spv::StorageClassUniform);

  module.decorate(block_type_id, _use_ssbo ? spv::DecorationBufferBlock : spv::DecorationBlock);
  module.decorate(block_var_id, spv::DecorationBinding, _transform_block_binding);
  module.decorate(block_var_id, spv::DecorationDescriptorSet, _transform_block_set);
  if (_use_ssbo) {
    module.decorate(block_var_id, spv::DecorationNonWritable);
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
