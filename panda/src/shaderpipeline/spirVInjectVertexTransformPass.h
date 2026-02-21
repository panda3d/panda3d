/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVInjectVertexTransformPass.h
 * @author rdb
 * @date 2026-02-17
 */

#ifndef SPIRVINJECTVERTEXTRANSFORMPASS_H
#define SPIRVINJECTVERTEXTRANSFORMPASS_H

#include "spirVTransformPass.h"
#include "pmap.h"

/**
 * Injects transformations into an existing vertex shader for implementing
 * animation, instancing, and/or both.  This is designed as a single pass with
 * two options because both are kind of similar, may share the same SSBO, and
 * combining them in a single pass results in less function call indirection
 * when duplicating out entry points.
 *
 * The animation transform takes the vertex inputs and pre-multiplies them by
 * the skinning matrix, which is composed based on the transform_index and
 * transform_weight vertex columns and a new buffer that is created to hold the
 * joint matrices.  The buffer may be either an UBO or an SSBO; if the latter,
 * a new uniform constant is created to offset the indices on top of the
 * transform_index values, stored in _transform_offset_var_id, to allow the use
 * of a large global SSBO without needing to rebind it.
 *
 * The instancing transform takes in a set of matrices that involve the
 * modelview matrix and modifies them (by making a private copy) by multiplying
 * in an affine instance matrix (mat3x4, transposed) fetched from a vertex
 * attribute (which is given a supplied location) or from the same SSBO as above
 * (in which case gl_InstanceIndex is used for indexing, with no other offsets).
 *
 * This can operate in two modes: in one mode it will modify all existing entry
 * points (with vertex model), in the other mode it will create new entry points
 * in addition to the existing ones but with suffixes added to their name.
 * The latter mode is useful for Vulkan, where we only use one compiled
 * VkShaderModule but choose at pipeline creation time which entry point to use.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVInjectVertexTransformPass final : public SpirVTransformPass {
public:
  SpirVInjectVertexTransformPass(bool make_new_entry_points, bool use_ssbo,
                                 uint32_t buffer_binding = 0,
                                 uint32_t buffer_set = 0);

  void setup_animation(uint32_t locations, uint32_t point_locations,
                       int index_location, int weight_location);

  void setup_instancing_attrib(int instance_mat_location);
  void mark_model_matrix(uint32_t id, bool inverse, bool transpose);

  virtual void preprocess();
  virtual bool transform_entry_point(spv::ExecutionModel model, uint32_t id, const char *name, pvector<uint32_t> &vars);
  virtual bool transform_annotation_op(Instruction op);
  virtual bool transform_definition_op(Instruction op);
  virtual bool transform_function_op(Instruction op);
  virtual void postprocess();

private:
  void inject_animation(const pvector<uint32_t> &vars);
  void inject_animation_noop(const pvector<uint32_t> &vars);
  void inject_instancing();
  void inject_instancing_noop();

  void define_transform_block();
  static const ShaderType *make_transform_block_type(uint32_t num_elements);

public:
  // Settings that could be related to both instancing and animation.
  struct EntryPoint {
    std::string _name;
    pvector<uint32_t> _vars;
  };
  pmap<uint32_t, EntryPoint> _todo_entry_points;
  bool _make_new_entry_points = false;
  uint32_t _transform_block_var_id = 0;
  uint32_t _transform_block_binding = 0;
  uint32_t _transform_block_set = 0;
  bool _use_ssbo = false;
  pset<uint32_t> _make_private_pointers;

  // Settings related to animation.
  uint32_t _anim_locations = 0;
  uint32_t _anim_point_locations = 0;
  int _transform_index_location = -1;
  int _transform_weight_location = -1;
  uint32_t _transform_index_var_id = 0;
  uint32_t _transform_weight_var_id = 0;
  pmap<uint32_t, uint32_t> _vertex_input_ids;
  uint32_t _transform_offset_var_id = 0;

  // Settings related to instancing.
  struct MatrixVar {
    uint32_t _id;
    bool _inverse;
    bool _transpose;
  };
  pmap<uint32_t, MatrixVar> _matrix_vars;
  int _instance_mat_location = -1;
  uint32_t _instance_mat_var_id = 0;
  uint32_t _instance_index_var_id = 0;
};

#endif
