/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_emulate_texture_queries_pass.cxx
 * @author rdb
 * @date 2026-07-02
 */

#include "spirVEmulateTextureQueriesPass.h"

#include "spirv_test_utils.h"
#include "catch_amalgamated.hpp"

TEST_CASE("SpirVEmulateTextureQueriesPass replaces textureSize", "[shaderpipeline]") {
  enum : uint32_t {
    id_main = 1, id_void, id_fnvoid, id_float,
    id_image, id_sampled_image, id_ptr_uc_image, id_tex,
    id_int, id_ivec2, id_const_0,
    id_label, id_load, id_img, id_size,
    id_bound,
  };

  // uniform sampler2D tex;  main() calls textureSize(tex, 0).
  ModuleBuilder b;
  b.op(spv::OpCapability, {spv::CapabilityShader});
  b.op(spv::OpCapability, {spv::CapabilityImageQuery});
  b.op(spv::OpMemoryModel, {spv::AddressingModelLogical, spv::MemoryModelGLSL450});
  b.op(spv::OpEntryPoint, {spv::ExecutionModelFragment, id_main}, "main");
  b.op(spv::OpExecutionMode, {id_main, spv::ExecutionModeOriginUpperLeft});
  b.op(spv::OpName, {id_tex}, "tex");
  b.op(spv::OpTypeVoid, {id_void});
  b.op(spv::OpTypeFunction, {id_fnvoid, id_void});
  b.op(spv::OpTypeFloat, {id_float, 32});
  b.op(spv::OpTypeImage, {id_image, id_float, spv::Dim2D, 0, 0, 0, 1, spv::ImageFormatUnknown});
  b.op(spv::OpTypeSampledImage, {id_sampled_image, id_image});
  b.op(spv::OpTypePointer, {id_ptr_uc_image, spv::StorageClassUniformConstant, id_sampled_image});
  b.op(spv::OpTypeInt, {id_int, 32, 1});
  b.op(spv::OpTypeVector, {id_ivec2, id_int, 2});
  b.op(spv::OpConstant, {id_int, id_const_0, 0});
  b.op(spv::OpVariable, {id_ptr_uc_image, id_tex, spv::StorageClassUniformConstant});
  b.op(spv::OpFunction, {id_void, id_main, spv::FunctionControlMaskNone, id_fnvoid});
  b.op(spv::OpLabel, {id_label});
  b.op(spv::OpLoad, {id_sampled_image, id_load, id_tex});
  b.op(spv::OpImage, {id_image, id_img, id_load});
  b.op(spv::OpImageQuerySizeLod, {id_ivec2, id_size, id_img, id_const_0});
  b.op(spv::OpReturn, {});
  b.op(spv::OpFunctionEnd, {});

  InstructionStream stream = b.build(id_bound);
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  SpirVEmulateTextureQueriesPass pass(Shader::C_texture_query_size);
  transformer.run(pass);
  CHECK(transformer.validate_db());

  InstructionStream result = transformer.get_result();
  CHECK(result.validate());

  // The query is replaced with a read from a new vec4 uniform that holds the
  // texture size, converted to the int vector the query returned.
  CHECK(count_op(result, spv::OpImageQuerySizeLod) == 0);
  CHECK(count_op(result, spv::OpConvertFToS) == 1);

  REQUIRE(pass._size_var_ids.size() == 1);
  const auto &item = *pass._size_var_ids.begin();
  CHECK(item.first._var_id == id_tex);
  CHECK(item.first.size() == 0);

  const SpirVResultDatabase::Definition &size_def =
    transformer.get_db().get_definition(item.second);
  CHECK(size_def._type ==
        ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4)));
  CHECK(size_def._storage_class == spv::StorageClassUniformConstant);
}
