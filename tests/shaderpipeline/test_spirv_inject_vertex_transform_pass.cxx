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
  enum : uint32_t {
    id_main = 1, id_void, id_fnvoid, id_float, id_vec4, id_mat4,
    id_ptr_in_vec4, id_ptr_out_vec4, id_ptr_uc_mat4,
    id_vertex, id_out, id_mv,
    id_label, id_load_mat, id_load_vtx, id_xformed,
    id_bound,
  };

  // in vec4 vertex; uniform mat4 mv;  main() { out_pos = mv * vertex; }
  ModuleBuilder b;
  b.op(spv::OpCapability, {spv::CapabilityShader});
  b.op(spv::OpMemoryModel, {spv::AddressingModelLogical, spv::MemoryModelGLSL450});
  b.op(spv::OpEntryPoint, {spv::ExecutionModelVertex, id_main}, "main", {id_vertex, id_out});
  b.op(spv::OpName, {id_vertex}, "vertex");
  b.op(spv::OpName, {id_mv}, "p3d_ModelViewMatrix");
  b.op(spv::OpDecorate, {id_vertex, spv::DecorationLocation, 0});
  b.op(spv::OpDecorate, {id_out, spv::DecorationLocation, 0});
  b.op(spv::OpTypeVoid, {id_void});
  b.op(spv::OpTypeFunction, {id_fnvoid, id_void});
  b.op(spv::OpTypeFloat, {id_float, 32});
  b.op(spv::OpTypeVector, {id_vec4, id_float, 4});
  b.op(spv::OpTypeMatrix, {id_mat4, id_vec4, 4});
  b.op(spv::OpTypePointer, {id_ptr_in_vec4, spv::StorageClassInput, id_vec4});
  b.op(spv::OpTypePointer, {id_ptr_out_vec4, spv::StorageClassOutput, id_vec4});
  b.op(spv::OpTypePointer, {id_ptr_uc_mat4, spv::StorageClassUniformConstant, id_mat4});
  b.op(spv::OpVariable, {id_ptr_in_vec4, id_vertex, spv::StorageClassInput});
  b.op(spv::OpVariable, {id_ptr_out_vec4, id_out, spv::StorageClassOutput});
  b.op(spv::OpVariable, {id_ptr_uc_mat4, id_mv, spv::StorageClassUniformConstant});
  b.op(spv::OpFunction, {id_void, id_main, spv::FunctionControlMaskNone, id_fnvoid});
  b.op(spv::OpLabel, {id_label});
  b.op(spv::OpLoad, {id_mat4, id_load_mat, id_mv});
  b.op(spv::OpLoad, {id_vec4, id_load_vtx, id_vertex});
  b.op(spv::OpMatrixTimesVector, {id_vec4, id_xformed, id_load_mat, id_load_vtx});
  b.op(spv::OpStore, {id_out, id_xformed});
  b.op(spv::OpReturn, {});
  b.op(spv::OpFunctionEnd, {});

  InstructionStream stream = b.build(id_bound);
  REQUIRE(stream.validate());

  SECTION("animation transforms the vertex inputs") {
    SpirVTransformer transformer(stream);
    SpirVInjectVertexTransformPass pass(false, false, 0, 0);
    pass.setup_animation(0x1, 0x1, 5, 6);
    transformer.run(pass);
    CHECK(transformer.validate_db());

    InstructionStream result = transformer.get_result();
    CHECK(result.validate());

    const SpirVResultDatabase &db = transformer.get_db();

    // A uniform block was added to hold the joint matrices, and vertex
    // columns were added for the transform indices and weights.
    REQUIRE(pass._transform_block_var_id != 0);
    CHECK(db.get_definition(pass._transform_block_var_id)._storage_class == spv::StorageClassUniform);

    REQUIRE(pass._transform_index_var_id != 0);
    CHECK(db.get_definition(pass._transform_index_var_id)._storage_class == spv::StorageClassInput);
    CHECK(db.get_definition(pass._transform_index_var_id)._location == 5);

    REQUIRE(pass._transform_weight_var_id != 0);
    CHECK(db.get_definition(pass._transform_weight_var_id)._storage_class == spv::StorageClassInput);
    CHECK(db.get_definition(pass._transform_weight_var_id)._location == 6);

    // The original vertex input variable now refers to a private copy that is
    // written at the top of the entry point.
    CHECK(db.get_definition(id_vertex)._storage_class == spv::StorageClassPrivate);
  }

  SECTION("instancing multiplies in a matrix vertex attribute") {
    SpirVTransformer transformer(stream);
    SpirVInjectVertexTransformPass pass(false, false, 0, 0);
    pass.setup_instancing_attrib(4);
    pass.mark_model_matrix(id_mv, false, false);
    transformer.run(pass);
    CHECK(transformer.validate_db());

    InstructionStream result = transformer.get_result();
    CHECK(result.validate());

    const SpirVResultDatabase &db = transformer.get_db();

    // A vertex column was added for the instance matrix.
    REQUIRE(pass._instance_mat_var_id != 0);
    const SpirVResultDatabase::Definition &mat_def =
      db.get_definition(pass._instance_mat_var_id);
    CHECK(mat_def._storage_class == spv::StorageClassInput);
    CHECK(mat_def._location == 4);
    CHECK(mat_def._type ==
          ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 4)));

    // The model-view matrix now refers to a private copy with the instance
    // matrix multiplied in.
    CHECK(db.get_definition(id_mv)._storage_class == spv::StorageClassPrivate);
  }
}
