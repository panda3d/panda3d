/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_inject_vertex_transform_pass.cxx
 * @author rdb
 * @date 2026-07-02
 */

#include "spirVInjectVertexTransformPass.h"

#include "spirv_test_utils.h"
#include "catch_amalgamated.hpp"

TEST_CASE("SpirVInjectVertexTransformPass injects transforms", "[shaderpipeline]") {
  // in vec4 vertex; uniform mat4 mv;  main() { out_pos = mv * vertex; }
  SpirVModule module = make_module();
  const ShaderType *vec4_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));
  const ShaderType *mat4_type = ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 4, 4));

  Id id_vertex = module.define_variable(vec4_type, spv::StorageClassInput);
  module.decorate(id_vertex, spv::DecorationLocation, 0u);
  module.set_name(id_vertex, "vertex");

  Id id_out = module.define_variable(vec4_type, spv::StorageClassOutput);
  module.decorate(id_out, spv::DecorationLocation, 0u);

  Id id_mv = module.define_variable(mat4_type, spv::StorageClassUniformConstant);
  module.set_name(id_mv, "p3d_ModelViewMatrix");

  {
    SpirVBuilder builder = make_entry_point(module, spv::ExecutionModelVertex, {id_vertex, id_out});
    Id load_mat = builder.op_load(id_mv);
    Id load_vtx = builder.op_load(id_vertex);
    Id xformed = builder.op_multiply(load_mat, load_vtx);
    builder.op_store(id_out, xformed);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  SECTION("animation transforms the vertex inputs") {
    SpirVTransformer transformer(stream);
    SpirVInjectVertexTransformPass pass(false, false, 0, 0);
    pass.setup_animation(0x1, 0x1, 5, 6);
    transformer.run(pass);
    CHECK(transformer.get_module().validate());

    const SpirVModule &out_module = transformer.get_module();

    // A uniform block was added to hold the joint matrices, and vertex
    // columns were added for the transform indices and weights.
    REQUIRE(pass._transform_block_var_id != 0);
    CHECK(out_module.get_storage_class(pass._transform_block_var_id) == spv::StorageClassUniform);

    REQUIRE(pass._transform_index_var_id != 0);
    CHECK(out_module.get_storage_class(pass._transform_index_var_id) == spv::StorageClassInput);
    CHECK(out_module.get_location(pass._transform_index_var_id) == 5);

    REQUIRE(pass._transform_weight_var_id != 0);
    CHECK(out_module.get_storage_class(pass._transform_weight_var_id) == spv::StorageClassInput);
    CHECK(out_module.get_location(pass._transform_weight_var_id) == 6);

    // The original vertex input variable now refers to a private copy that is
    // written at the top of the entry point.
    CHECK(out_module.get_storage_class(id_vertex) == spv::StorageClassPrivate);
  }

  SECTION("instancing multiplies in a matrix vertex attribute") {
    SpirVTransformer transformer(stream);
    SpirVInjectVertexTransformPass pass(false, false, 0, 0);
    pass.setup_instancing_attrib(4);
    pass.mark_model_matrix(id_mv, false, false);
    transformer.run(pass);
    CHECK(transformer.get_module().validate());

    const SpirVModule &out_module = transformer.get_module();

    // A vertex column was added for the instance matrix.
    REQUIRE(pass._instance_mat_var_id != 0);
    CHECK(out_module.get_storage_class(pass._instance_mat_var_id) == spv::StorageClassInput);
    CHECK(out_module.get_location(pass._instance_mat_var_id) == 4);
    CHECK(out_module.resolve_type(pass._instance_mat_var_id) ==
          ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 4)));

    // The model-view matrix now refers to a private copy with the instance
    // matrix multiplied in.
    CHECK(out_module.get_storage_class(id_mv) == spv::StorageClassPrivate);
  }
}
