/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVInjectInstancingPass.h
 * @author rdb
 * @date 2026-02-09
 */

#ifndef SPIRVINJECTINSTANCINGPASS_H
#define SPIRVINJECTINSTANCINGPASS_H

#include "spirVTransformPass.h"
#include "pmap.h"

/**
 * Injects hardware instancing support into an existing vertex shader.
 *
 * This takes in a set of matrices that involve the modelview matrix and
 * modifies them (by making a private copy) by multiplying in an affine
 * instance matrix (mat3x4, transposed) fetched from a vertex attribute
 * (which is given a supplied location).
 *
 * This doesn't generate optimal code in all circumstances (in some cases
 * computing the whole matrix may be unnecessary) but it is likely that the
 * driver will optimize this out anyway.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVInjectInstancingPass final : public SpirVTransformPass {
public:
  struct MatrixVar {
    uint32_t _id;
    bool _inverse;
    bool _transpose;
  };

  SpirVInjectInstancingPass(const pvector<MatrixVar> &matrix_vars,
                            int instance_mat_location=-1);

  virtual bool transform_entry_point(spv::ExecutionModel model, uint32_t id, const char *name, pvector<uint32_t> &vars);
  virtual bool transform_annotation_op(Instruction op);
  virtual bool transform_definition_op(Instruction op);
  virtual bool transform_function_op(Instruction op);

public:
  pmap<uint32_t, MatrixVar> _matrix_vars;
  pset<uint32_t> _todo_functions;
  int _instance_mat_location = -1;
  uint32_t _instance_mat_id = 0;
  bool _instance_mat_id_declared = false;
};

#endif
