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
    else if (module.get_builtin(id) == spv::BuiltInInstanceIndex) {
      _instance_index_var_id = id;
    }
  }

  // Capture the vertex entry points with their original interfaces, before
  // the variable swap below.
  for (size_t i = 0; i < module.get_num_entry_points(); ++i) {
    const SpirVModule::EntryPoint &ep = module.get_entry_point(i);
    if (ep.model == spv::ExecutionModelVertex) {
      _todo_entry_points.insert({ep.function_id, EntryPoint {ep.name, ep.interface_vars}});
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
      const EntryPoint &entry_point = item.second;

      nassertd(module.find_function(function_id) != nullptr) continue;

      // Inject at the top of the body: after the entry block's label and
      // past the OpVariable declarations that must lead it.
      SpirVBuilder builder(module);
      builder.set_insertion_point_to_body_start(function_id);
      inject_animation(builder, entry_point._vars);
      inject_instancing(builder);

      // Update the entry point interface.
      for (SpirVModule::EntryPoint &ep : module.modify_entry_points()) {
        if (ep.model == spv::ExecutionModelVertex && ep.function_id == function_id) {
          ep.interface_vars = make_interface(entry_point._vars,
                                        _anim_locations != 0u,
                                        !_matrix_vars.empty());
        }
      }
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
    const EntryPoint &entry_point = item.second;

    {
      SpirVBuilder builder = module.make_function(nullptr);
      inject_instancing_noop(builder);
      inject_animation_noop(builder, entry_point._vars);
      builder.op_function_call(orig_func_id);
      builder.op_return();
      Id func_id = builder.get_current_function_id();
      module.add_entry_point(spv::ExecutionModelVertex, func_id, entry_point._name,
                             make_interface(entry_point._vars, false, false));
    }

    if (_anim_locations != 0u) {
      SpirVBuilder builder = module.make_function(nullptr);
      inject_instancing_noop(builder);
      inject_animation(builder, entry_point._vars);
      builder.op_function_call(orig_func_id);
      builder.op_return();
      Id func_id = builder.get_current_function_id();
      module.add_entry_point(spv::ExecutionModelVertex, func_id, entry_point._name + "_anim",
                             make_interface(entry_point._vars, true, false));
    }

    if (!_matrix_vars.empty()) {
      SpirVBuilder builder = module.make_function(nullptr);
      inject_instancing(builder);
      inject_animation_noop(builder, entry_point._vars);
      builder.op_function_call(orig_func_id);
      builder.op_return();
      Id func_id = builder.get_current_function_id();
      module.add_entry_point(spv::ExecutionModelVertex, func_id, entry_point._name + "_inst",
                             make_interface(entry_point._vars, false, true));
    }

    if (!_matrix_vars.empty() && _anim_locations != 0u) {
      SpirVBuilder builder = module.make_function(nullptr);
      inject_instancing(builder);
      inject_animation(builder, entry_point._vars);
      builder.op_function_call(orig_func_id);
      builder.op_return();
      Id func_id = builder.get_current_function_id();
      module.add_entry_point(spv::ExecutionModelVertex, func_id, entry_point._name + "_anim_inst",
                             make_interface(entry_point._vars, true, true));
    }
  }
  _todo_entry_points.clear();
}

/**
 * Builds an entry point interface list from the given original interface,
 * mapping the swapped input variables and appending the extra inputs used by
 * the animation and/or instancing preambles.
 */
pvector<SpirVId> SpirVInjectVertexTransformPass::
make_interface(const pvector<Id> &vars,
               bool with_animation, bool with_instancing) const {
  pvector<Id> result;
  bool has_transform_index = false;
  bool has_transform_weight = false;
  bool has_instance_matrix = false;
  bool has_instance_index = false;

  for (Id var_id : vars) {
    auto it = _vertex_input_ids.find(var_id);
    if (it != _vertex_input_ids.end()) {
      result.push_back(it->second);
    } else {
      result.push_back(var_id);
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

  if (with_animation && _anim_locations != 0u) {
    if (!has_transform_index && _transform_index_var_id != 0u) {
      result.push_back(_transform_index_var_id);
    }
    if (!has_transform_weight && _transform_weight_var_id != 0u) {
      result.push_back(_transform_weight_var_id);
    }
  }
  if (with_instancing && !_matrix_vars.empty()) {
    if (_instance_mat_var_id != 0u) {
      if (!has_instance_matrix) {
        result.push_back(_instance_mat_var_id);
      }
    } else {
      if (!has_instance_index && _instance_index_var_id != 0u) {
        result.push_back(_instance_index_var_id);
      }
    }
  }
  return result;
}

/**
 * Injects the function preamble that applies the animation transformations
 * at the given builder's cursor.
 */
void SpirVInjectVertexTransformPass::
inject_animation(SpirVBuilder &builder, const pvector<Id> &vars) {
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

  Id transform_index = builder.op_load(_transform_index_var_id);
  Id transform_weight = builder.op_load(_transform_weight_var_id);

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

    // Must broadcast to a vector before we can add it.
    Id offset = builder.op_load(_transform_offset_var_id);
    Id vec_offset = builder.op_composite_construct(transform_index_type,
      pvector<Id>(transform_index_type->get_num_components(), offset));
    transform_index = builder.op_add(transform_index, vec_offset);
  }

  // We can't add matrices together, we have to add them per vector.
  Id accum_matrix_row0;
  Id accum_matrix_row1;
  Id accum_matrix_row2;

  Id zero = module.define_int_constant(0);

  for (uint32_t i = 0; i < num_components; ++i) {
    // matrix = ubo.m[transform_index[i]]
    Id matrix = builder.op_load(builder.op_access_chain(_transform_block_var_id, {zero, builder.op_composite_extract(transform_index, {i})}));

    // matrix *= transform_weight[i]
    matrix = builder.op_multiply(matrix, builder.op_composite_extract(transform_weight, {i}));

    // Split into rows
    Id matrix_row0 = builder.op_composite_extract(matrix, {0});
    Id matrix_row1 = builder.op_composite_extract(matrix, {1});
    Id matrix_row2 = builder.op_composite_extract(matrix, {2});
    if (i == 0) {
      accum_matrix_row0 = matrix_row0;
      accum_matrix_row1 = matrix_row1;
      accum_matrix_row2 = matrix_row2;
    } else {
      accum_matrix_row0 = builder.op_add(accum_matrix_row0, matrix_row0);
      accum_matrix_row1 = builder.op_add(accum_matrix_row1, matrix_row1);
      accum_matrix_row2 = builder.op_add(accum_matrix_row2, matrix_row2);
    }
  }

  const ShaderType *mat3x4_type = ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 4));
  Id mat3x4 = builder.op_composite_construct(mat3x4_type, {accum_matrix_row0, accum_matrix_row1, accum_matrix_row2});

  for (Id private_vec_ptr : vars) {
    auto it = _vertex_input_ids.find(private_vec_ptr);
    if (it != _vertex_input_ids.end()) {
      Id input_vec_ptr = it->second;

      const ShaderType *input_type = module.resolve_type(input_vec_ptr);
      const ShaderType::Vector *vec_type = (input_type != nullptr) ? input_type->as_vector() : nullptr;
      if (vec_type == nullptr || vec_type->get_num_components() < 3) {
        continue;
      }

      bool is_point = (_anim_point_locations & (1u << module.get_location(input_vec_ptr))) != 0;
      Id w = module.define_float_constant(is_point);

      Id orig_vec = builder.op_load(input_vec_ptr);
      if (vec_type->get_num_components() == 3) {
        // Expand to vec4.
        orig_vec = builder.op_composite_construct(vec4_type, {orig_vec, w});
      }

      Id result = builder.op_multiply(orig_vec, mat3x4);
      if (vec_type->get_num_components() == 4) {
        // It comes out as a vec3, so expand to vec4 again.
        result = builder.op_composite_construct(vec4_type, {result, w});
      }

      builder.op_store(private_vec_ptr, result);
    }
  }
}

/**
 * Injects the function preamble that does not apply the transformations.
 */
void SpirVInjectVertexTransformPass::
inject_animation_noop(SpirVBuilder &builder, const pvector<Id> &vars) {
  for (Id private_vec_ptr : vars) {
    auto it = _vertex_input_ids.find(private_vec_ptr);
    if (it != _vertex_input_ids.end()) {
      Id input_vec_ptr = it->second;
      builder.op_store(private_vec_ptr, builder.op_load(input_vec_ptr));
    }
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

  Id inst_mat3x4;

  if (_instance_mat_location >= 0) {
    // Make sure the instance matrix vertex input is defined.
    if (_instance_mat_var_id == 0u) {
      _instance_mat_var_id = module.define_variable(ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 4)), spv::StorageClassInput);
      module.decorate(_instance_mat_var_id, spv::DecorationLocation, (uint32_t)_instance_mat_location);
    }

    inst_mat3x4 = builder.op_load(_instance_mat_var_id);
  } else {
    // Make sure the transform SSBO is defined.
    if (_transform_block_var_id == 0) {
      define_transform_block(module);
    }
    if (_instance_index_var_id == 0) {
      _instance_index_var_id = module.define_variable(ShaderType::INT, spv::StorageClassInput);
      module.decorate(_instance_index_var_id, spv::DecorationBuiltIn, spv::BuiltInInstanceIndex);
    }

    Id inst_index = builder.op_load(_instance_index_var_id);
    Id zero = module.define_int_constant(0);
    inst_mat3x4 = builder.op_load(builder.op_access_chain(_transform_block_var_id, {zero, inst_index}));
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

      const ShaderType::Matrix *matrix = module.resolve_type(pair.first)->as_matrix();
      if (pair.second._transpose ? matrix->get_num_columns() == 4 : matrix->get_num_rows() == 4) {
        need_inverse_translation = true;
      }
    }
  }
  Id inv_inst_mat3;
  Id inv_inst_mat4;
  Id inv_inst_mat3x4;
  if (need_inverse) {
    const ShaderType *mat3x4_type = ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 4));
    const ShaderType *vec3_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 3));

    Id r0 = builder.op_composite_construct(vec3_type, {
      builder.op_sub(builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {1, 1}), builder.op_composite_extract(inst_mat3x4, {2, 2})),
                     builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {2, 1}), builder.op_composite_extract(inst_mat3x4, {1, 2}))),
      builder.op_sub(builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {2, 1}), builder.op_composite_extract(inst_mat3x4, {0, 2})),
                     builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {0, 1}), builder.op_composite_extract(inst_mat3x4, {2, 2}))),
      builder.op_sub(builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {0, 1}), builder.op_composite_extract(inst_mat3x4, {1, 2})),
                     builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {1, 1}), builder.op_composite_extract(inst_mat3x4, {0, 2}))),
    });
    Id r1 = builder.op_composite_construct(vec3_type, {
      builder.op_sub(builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {1, 2}), builder.op_composite_extract(inst_mat3x4, {2, 0})),
                     builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {2, 2}), builder.op_composite_extract(inst_mat3x4, {1, 0}))),
      builder.op_sub(builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {2, 2}), builder.op_composite_extract(inst_mat3x4, {0, 0})),
                     builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {0, 2}), builder.op_composite_extract(inst_mat3x4, {2, 0}))),
      builder.op_sub(builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {0, 2}), builder.op_composite_extract(inst_mat3x4, {1, 0})),
                     builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {1, 2}), builder.op_composite_extract(inst_mat3x4, {0, 0}))),
    });
    Id r2 = builder.op_composite_construct(vec3_type, {
      builder.op_sub(builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {1, 0}), builder.op_composite_extract(inst_mat3x4, {2, 1})),
                     builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {2, 0}), builder.op_composite_extract(inst_mat3x4, {1, 1}))),
      builder.op_sub(builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {2, 0}), builder.op_composite_extract(inst_mat3x4, {0, 1})),
                     builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {0, 0}), builder.op_composite_extract(inst_mat3x4, {2, 1}))),
      builder.op_sub(builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {0, 0}), builder.op_composite_extract(inst_mat3x4, {1, 1})),
                     builder.op_multiply(builder.op_composite_extract(inst_mat3x4, {1, 0}), builder.op_composite_extract(inst_mat3x4, {0, 1}))),
    });
    Id one = module.define_float_constant(1);
    Id inv_det = builder.op_div(one, builder.op_dot(r0, builder.op_composite_construct(vec3_type, {
      builder.op_composite_extract(inst_mat3x4, {0, 0}),
      builder.op_composite_extract(inst_mat3x4, {1, 0}),
      builder.op_composite_extract(inst_mat3x4, {2, 0}),
    })));

    if (need_inverse_translation) {
      // Handle the translation.
      Id t = builder.op_composite_construct(vec3_type, {
        builder.op_composite_extract(inst_mat3x4, {0, 3}),
        builder.op_composite_extract(inst_mat3x4, {1, 3}),
        builder.op_composite_extract(inst_mat3x4, {2, 3}),
      });

      inv_inst_mat3x4 = builder.op_multiply(builder.op_composite_construct(mat3x4_type, {
        builder.op_composite_construct(vec4_type, {r0, builder.op_negate(builder.op_dot(r0, t))}),
        builder.op_composite_construct(vec4_type, {r1, builder.op_negate(builder.op_dot(r1, t))}),
        builder.op_composite_construct(vec4_type, {r2, builder.op_negate(builder.op_dot(r2, t))}),
      }), inv_det);
    }
    else {
      // Don't bother constructing the last column.
      inv_inst_mat3 = builder.op_multiply(builder.op_composite_construct(mat3_type, {r0, r1, r2}), inv_det);
    }
  }
  Id inst_mat3;
  Id inst_mat4;

  for (auto &pair : _matrix_vars) {
    bool inverse = pair.second._inverse;
    bool transpose = pair.second._transpose;
    Id orig_mat_ptr = pair.second._id;
    Id result_mat_ptr = pair.first;

    Id orig_mat = builder.op_load(orig_mat_ptr);
    Id inst_mat;

    // Compose the right matrix for multiplying into the target matrix.
    const ShaderType::Matrix *matrix = module.resolve_type(orig_mat_ptr)->as_matrix();
    uint32_t mat_size = transpose ? matrix->get_num_columns() : matrix->get_num_rows();

    if (matrix->get_num_rows() == 3 && matrix->get_num_columns() == 4 && transpose) {
      // Special case of affine mat * affine mat.
      pvector<Id> rows {
        builder.op_composite_extract(orig_mat, {0}),
        builder.op_composite_extract(orig_mat, {1}),
        builder.op_composite_extract(orig_mat, {2}),
      };

      Id orig_mat3 = builder.op_composite_construct(mat3_type, {
        builder.op_vector_shuffle(rows[0], rows[0], {0, 1, 2}),
        builder.op_vector_shuffle(rows[1], rows[1], {0, 1, 2}),
        builder.op_vector_shuffle(rows[2], rows[2], {0, 1, 2}),
      });

      // mat3x4 * mat3 -> mat3x4
      Id tmp_mat = builder.op_multiply(inverse ? inv_inst_mat3x4 : inst_mat3x4, orig_mat3);

      // Now handle w (we assume that the matrix we modify isn't affine)
      for (uint32_t i = 0; i < matrix->get_num_rows(); ++i) {
        Id tmp_row = builder.op_composite_extract(tmp_mat, {i});
        Id new_w = builder.op_add(builder.op_composite_extract(tmp_mat, {i, 3}), builder.op_composite_extract(rows[i], {3}));
        rows[i] = builder.op_composite_construct(vec4_type, {
          builder.op_vector_shuffle(tmp_row, tmp_row, {0, 1, 2}),
          new_w,
        });
      }

      builder.op_store(result_mat_ptr, builder.op_composite_construct(matrix, rows));
      continue;
    }
    else if (mat_size == 4) {
      // Construct a 4x4 matrix.
      Id &use_inst_mat4 = inverse ? inv_inst_mat4 : inst_mat4;
      if (use_inst_mat4 == 0) {
        const ShaderType *mat4_type = ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 4, 4));
        Id zero = module.define_float_constant(0);
        Id one = module.define_float_constant(1);

        Id input = inverse ? inv_inst_mat3x4 : inst_mat3x4;
        Id row0 = builder.op_composite_extract(input, {0});
        Id row1 = builder.op_composite_extract(input, {1});
        Id row2 = builder.op_composite_extract(input, {2});
        Id row3 = builder.op_composite_construct(vec4_type, {zero, zero, zero, one});
        use_inst_mat4 = builder.op_composite_construct(mat4_type, {row0, row1, row2, row3});
      }
      inst_mat = use_inst_mat4;
    }
    else {
      // Construct a 3x3 matrix without translation.
      Id &use_inst_mat3 = inverse ? inv_inst_mat3 : inst_mat3;
      if (use_inst_mat3 == 0) {
        Id input = inverse ? inv_inst_mat3x4 : inst_mat3x4;

        pvector<Id> rows;
        for (uint32_t i = 0; i < mat_size; ++i) {
          Id row = builder.op_composite_extract(input, {i});
          rows.push_back(builder.op_vector_shuffle(row, row, {0, 1, 2}));
        }
        use_inst_mat3 = builder.op_composite_construct(mat3_type, rows);
      }
      inst_mat = use_inst_mat3;
    }

    Id left, right;
    if (transpose) {
      // The incoming instance matrix is already transposed.
      left = inst_mat;
      right = orig_mat;
    } else {
      left = orig_mat;
      right = builder.op_transpose(inst_mat);
    }

    builder.op_store(result_mat_ptr, builder.op_multiply(left, right));
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

    builder.op_store(result_mat_ptr, builder.op_load(orig_mat_ptr));
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
