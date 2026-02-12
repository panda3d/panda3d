/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVInjectInstancingPass.cxx
 * @author rdb
 * @date 2026-02-09
 */

#include "spirVInjectInstancingPass.h"

/**
 *
 */
SpirVInjectInstancingPass::
SpirVInjectInstancingPass(const pvector<MatrixVar> &matrix_vars, int instance_mat_location) :
  _instance_mat_location(instance_mat_location) {

  for (const MatrixVar &var : matrix_vars) {
    _matrix_vars[var._id] = MatrixVar {0u, var._inverse, var._transpose};
  }
}

/**
 * Transforms an OpEntryPoint.
 * Return true to keep the instruction, false to omit it.
 */
bool SpirVInjectInstancingPass::
transform_entry_point(spv::ExecutionModel model, uint32_t id, const char *name, pvector<uint32_t> &vars) {
  if (model == spv::ExecutionModelVertex) {
    // Add the instance matrix to this entry point.
    if (_instance_mat_id == 0) {
      _instance_mat_id = allocate_id();
    }
    vars.push_back(_instance_mat_id);
    _todo_functions.insert(id);
  }
  return true;
}

/**
 *
 */
bool SpirVInjectInstancingPass::
transform_annotation_op(Instruction op) {
  // Move the decoration to point to a yet-to-be-constructed copy.
  if (op.opcode == spv::OpDecorate) {
    uint32_t var_id = op.args[0];

    auto it = _matrix_vars.find(var_id);
    if (it != _matrix_vars.end()) {
      if (it->second._id == 0) {
        it->second._id = allocate_id();
      }
      op.args[0] = it->second._id;
    }
  }
  return SpirVTransformPass::transform_definition_op(op);
}

/**
 *
 */
bool SpirVInjectInstancingPass::
transform_definition_op(Instruction op) {
  if (op.opcode == spv::OpVariable) {
    uint32_t var_id = op.args[1];
    spv::StorageClass storage_class = (spv::StorageClass)op.args[2];
    if (storage_class == spv::StorageClassUniformConstant) {
      // We swap the ids so that the new private variable definition gets the
      // id that the uniform constant used to have, so that all loads from this
      // point on now point to the private variable.
      auto it = _matrix_vars.find(var_id);
      if (it != _matrix_vars.end()) {
        const ShaderType *type = resolve_pointer_type(op.args[0]);
        if (it->second._id == 0) {
          it->second._id = allocate_id();
        }

        Definition &old_def = _db.modify_definition(var_id);
        Definition def = std::move(_db.modify_definition(var_id));
        old_def.clear();

        push_id(var_id);
        define_variable(type, spv::StorageClassPrivate);

        op.args[1] = it->second._id;
        _db.modify_definition(op.args[1]) = std::move(def);
      }
    }
  }
  return SpirVTransformPass::transform_definition_op(op);
}

/**
 *
 */
bool SpirVInjectInstancingPass::
transform_function_op(Instruction op) {
  if (op.opcode == spv::OpLabel && _todo_functions.erase(_current_function_id)) {
    // First label in a function.  Write out the label and then insert the
    // writes to the private matrix vars.
    _new_functions.push_back((2 << spv::WordCountShift) | spv::OpLabel);
    _new_functions.push_back(op.args[0]);

    // Make sure the instance matrix vertex input is defined.
    if (!_instance_mat_id_declared) {
      push_id(_instance_mat_id);
      uint32_t var_id = define_variable(ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 4)), spv::StorageClassInput);
      assert(var_id == _instance_mat_id);
      if (_instance_mat_location >= 0) {
        decorate(var_id, spv::DecorationLocation, (uint32_t)_instance_mat_location);
      }
      _instance_mat_id_declared = true;
    }

    uint32_t inst_mat3x4 = op_load(_instance_mat_id);

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

    return false;
  }

  return SpirVTransformPass::transform_function_op(op);
}
