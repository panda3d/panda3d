/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVInjectAnimationPass.cxx
 * @author rdb
 * @date 2026-02-17
 */

#include "spirVInjectAnimationPass.h"

/**
 *
 */
SpirVInjectAnimationPass::
SpirVInjectAnimationPass(uint32_t point_locations, uint32_t vector_locations,
                         int index_location, int weight_location,
                         uint32_t ubo_binding, uint32_t ubo_set) :
  _point_locations(point_locations),
  _vector_locations(vector_locations),
  _transform_index_location(index_location),
  _transform_weight_location(weight_location),
  _uniform_block_binding(ubo_binding),
  _uniform_block_set(ubo_set) {
}

/**
 * Transforms an OpEntryPoint.
 * Return true to keep the instruction, false to omit it.
 */
bool SpirVInjectAnimationPass::
transform_entry_point(spv::ExecutionModel model, uint32_t id, const char *name, pvector<uint32_t> &vars) {
  if (model == spv::ExecutionModelVertex) {
    bool got_any = false;
    uint32_t transform_index_var_id = 0;
    uint32_t transform_weight_var_id = 0;

    for (uint32_t &var_id : vars) {
      const Definition &var_def = _db.get_definition(var_id);
      if (var_def._storage_class == spv::StorageClassInput && var_def._location >= 0) {
        uint32_t loc_mask = (1u << var_def._location);
        if ((_vector_locations & loc_mask) != 0 || (_point_locations & loc_mask) != 0) {
          // This will be transformed.
          uint32_t new_id = allocate_id();
          vertex_input_ids[var_id] = new_id;
          var_id = new_id;
          got_any = true;
        }

        if (_transform_index_location == var_def._location) {
          transform_index_var_id = var_id;
        }
        if (_transform_weight_location == var_def._location) {
          transform_weight_var_id = var_id;
        }
      }
    }
    if (got_any) {
      _todo_functions.insert(id);

      // Add transform_index and transform_weight to the interface, if they
      // are not already there.
      if (transform_index_var_id != 0 && _transform_index_var_id == 0) {
        _transform_index_var_id = transform_index_var_id;
        _declared_transform_index = true;
      } else {
        if (_transform_index_var_id == 0) {
          _transform_index_var_id = allocate_id();
        }
        vars.push_back(_transform_index_var_id);
      }

      if (transform_weight_var_id != 0 && _transform_weight_var_id == 0) {
        _transform_weight_var_id = transform_weight_var_id;
        _declared_transform_weight = true;
      } else {
        if (_transform_weight_var_id == 0) {
          _transform_weight_var_id = allocate_id();
        }
        vars.push_back(_transform_weight_var_id);
      }
    }
  }
  return true;
}

/**
 *
 */
bool SpirVInjectAnimationPass::
transform_annotation_op(Instruction op) {
  // Move the decoration to point to a yet-to-be-constructed copy.
  if (op.opcode == spv::OpDecorate) {
    uint32_t var_id = op.args[0];

    auto it = vertex_input_ids.find(var_id);
    if (it != vertex_input_ids.end()) {
      op.args[0] = it->second;
    }
  }
  return SpirVTransformPass::transform_annotation_op(op);
}

/**
 *
 */
bool SpirVInjectAnimationPass::
transform_definition_op(Instruction op) {
  if (op.opcode == spv::OpVariable) {
    uint32_t var_id = op.args[1];
    spv::StorageClass storage_class = (spv::StorageClass)op.args[2];
    if (storage_class == spv::StorageClassInput) {
      // We swap the ids so that the new private variable definition gets the
      // id that the input used to have, so that all loads from this point on
      // now point to the private variable.
      auto it = vertex_input_ids.find(var_id);
      if (it != vertex_input_ids.end()) {
        const ShaderType *type = resolve_pointer_type(op.args[0]);

        Definition &old_def = _db.modify_definition(var_id);
        Definition def = std::move(old_def);
        old_def.clear();

        push_id(var_id);
        define_variable(type, spv::StorageClassPrivate);

        op.args[1] = it->second;
        _db.modify_definition(op.args[1]) = std::move(def);
      }
    }
  }
  return SpirVTransformPass::transform_definition_op(op);
}

/**
 *
 */
bool SpirVInjectAnimationPass::
transform_function_op(Instruction op) {
  if (op.opcode == spv::OpLabel && _todo_functions.erase(_current_function_id)) {
    // First label in a function.  Write out the label and then insert the
    // writes to the private vars.
    _new_functions.push_back((2 << spv::WordCountShift) | spv::OpLabel);
    _new_functions.push_back(op.args[0]);

    // Make sure the skinning matrix UBO is defined.
    if (_uniform_block_var_id == 0) {
      static const ShaderType *block_type = make_uniform_block_type(256);
      uint32_t block_type_id = define_type(block_type);
      uint32_t block_var_id = define_variable(block_type, spv::StorageClassUniform);

      decorate(block_type_id, spv::DecorationBlock);
      decorate(block_var_id, spv::DecorationBinding, _uniform_block_binding);
      decorate(block_var_id, spv::DecorationDescriptorSet, _uniform_block_set);
      _uniform_block_var_id = block_var_id;
    }

    if (!_declared_transform_index) {
      const ShaderType *uvec4_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_uint, 4));
      push_id(_transform_index_var_id);
      _transform_index_var_id = define_variable(uvec4_type, spv::StorageClassInput);
      decorate(_transform_index_var_id, spv::DecorationLocation, _transform_index_location);
      _declared_transform_index = true;
    }

    const ShaderType *vec4_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));
    if (!_declared_transform_weight) {
      push_id(_transform_weight_var_id);
      _transform_weight_var_id = define_variable(vec4_type, spv::StorageClassInput);
      decorate(_transform_weight_var_id, spv::DecorationLocation, _transform_weight_location);
      _declared_transform_weight = true;
    }

    uint32_t transform_index = op_load(_transform_index_var_id);
    uint32_t transform_weight = op_load(_transform_weight_var_id);

    // How many components do we have?
    const ShaderType::Vector *transform_index_type = _db.get_definition(_transform_index_var_id)._type->as_vector();
    const ShaderType::Vector *transform_weight_type = _db.get_definition(_transform_weight_var_id)._type->as_vector();
    nassertr(transform_index_type != nullptr && transform_weight_type != nullptr, true);
    uint32_t num_components = std::min(transform_index_type->get_num_components(), transform_weight_type->get_num_components());

    // We can't add matrices together, we have to add them per vector.
    uint32_t accum_matrix_row0 = 0;
    uint32_t accum_matrix_row1 = 0;
    uint32_t accum_matrix_row2 = 0;

    uint32_t zero = define_int_constant(0);

    for (uint32_t i = 0; i < num_components; ++i) {
      // matrix = ubo.m[transform_index[i]]
      uint32_t matrix = op_load(op_access_chain(_uniform_block_var_id, {zero, op_composite_extract(transform_index, {i})}));

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

    for (auto &pair : vertex_input_ids) {
      uint32_t orig_vec_ptr = pair.second;
      uint32_t result_vec_ptr = pair.first;

      const Definition &vec_def = _db.get_definition(orig_vec_ptr);
      const ShaderType::Vector *vec_type = vec_def._type->as_vector();
      if (vec_type == nullptr || vec_type->get_num_components() < 3) {
        continue;
      }

      bool is_point = (_point_locations & (1u << vec_def._location)) != 0;
      uint32_t w = define_float_constant(is_point);

      uint32_t orig_vec = op_load(orig_vec_ptr);
      if (vec_type->get_num_components() == 3) {
        // Expand to vec4.
        orig_vec = op_composite_construct(vec4_type, {orig_vec, w});
      }

      uint32_t result = op_multiply(orig_vec, mat3x4);
      if (vec_type->get_num_components() == 4) {
        // It comes out as a vec3, so expand to vec4 again.
        result = op_composite_construct(vec4_type, {result, w});
      }

      op_store(result_vec_ptr, result);
    }

    return false;
  }

  return SpirVTransformPass::transform_function_op(op);
}

/**
 *
 */
const ShaderType *SpirVInjectAnimationPass::
make_uniform_block_type(uint32_t num_elements) {
  // Because of alignment rules making it more efficient, we store the matrix
  // as a transposed 3x4 matrix.
  const ShaderType *mat3x4_type = ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 4));
  const ShaderType *array_type = ShaderType::register_type(ShaderType::Array(mat3x4_type, num_elements));
  ShaderType::Struct struct_type;
  struct_type.add_member(array_type, "m");
  return ShaderType::register_type(std::move(struct_type));
}
