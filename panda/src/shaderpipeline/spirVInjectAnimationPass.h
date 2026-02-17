/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVInjectAnimationPass.h
 * @author rdb
 * @date 2026-02-17
 */

#ifndef SPIRVINJECTANIMATIONPASS_H
#define SPIRVINJECTANIMATIONPASS_H

#include "spirVTransformPass.h"
#include "pmap.h"

/**
 * Injects hardware animation support into an existing vertex shader.
 *
 * This takes the vertex inputs and pre-multiplies them by the skinning matrix,
 * which is composed based on the transform_index and transform_weight vertex
 * columns and a new UBO that is created to hold the joint matrices.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVInjectAnimationPass final : public SpirVTransformPass {
public:
  SpirVInjectAnimationPass(uint32_t point_locations, uint32_t vector_locations,
                           int index_location, int weight_location,
                           uint32_t ubo_binding, uint32_t ubo_set = 0);

  virtual bool transform_entry_point(spv::ExecutionModel model, uint32_t id, const char *name, pvector<uint32_t> &vars);
  virtual bool transform_annotation_op(Instruction op);
  virtual bool transform_definition_op(Instruction op);
  virtual bool transform_function_op(Instruction op);

private:
  static const ShaderType *make_uniform_block_type(uint32_t num_elements);

public:
  uint32_t _point_locations = 0;
  uint32_t _vector_locations = 0;
  int _transform_index_location = -1;
  int _transform_weight_location = -1;
  uint32_t _transform_index_var_id = 0;
  uint32_t _transform_weight_var_id = 0;
  bool _declared_transform_index = false;
  bool _declared_transform_weight = false;
  pmap<uint32_t, uint32_t> vertex_input_ids;
  uint32_t _uniform_block_var_id = 0;
  uint32_t _uniform_block_binding = 0;
  uint32_t _uniform_block_set = 0;
  pset<uint32_t> _todo_functions;
};

#endif
