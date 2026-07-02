/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_flatten_struct_pass.cxx
 * @author rdb
 * @date 2026-07-02
 */

#include "spirVFlattenStructPass.h"

#include "spirv_test_utils.h"
#include "catch_amalgamated.hpp"

TEST_CASE("SpirVFlattenStructPass turns struct members into variables", "[shaderpipeline]") {
  enum : uint32_t {
    id_main = 1, id_void, id_fnvoid, id_float, id_vec2,
    id_struct, id_ptr_uc_struct, id_var,
    id_int, id_const_0, id_ptr_uc_float,
    id_label, id_chain, id_value,
    id_bound,
  };

  // struct S { float a; vec2 b; }; uniform S s;  main() loads s.a.
  ModuleBuilder b;
  b.op(spv::OpCapability, {spv::CapabilityShader});
  b.op(spv::OpMemoryModel, {spv::AddressingModelLogical, spv::MemoryModelGLSL450});
  b.op(spv::OpEntryPoint, {spv::ExecutionModelFragment, id_main}, "main");
  b.op(spv::OpExecutionMode, {id_main, spv::ExecutionModeOriginUpperLeft});
  b.op(spv::OpName, {id_struct}, "S");
  b.op(spv::OpMemberName, {id_struct, 0}, "a");
  b.op(spv::OpMemberName, {id_struct, 1}, "b");
  b.op(spv::OpName, {id_var}, "s");
  b.op(spv::OpTypeVoid, {id_void});
  b.op(spv::OpTypeFunction, {id_fnvoid, id_void});
  b.op(spv::OpTypeFloat, {id_float, 32});
  b.op(spv::OpTypeVector, {id_vec2, id_float, 2});
  b.op(spv::OpTypeStruct, {id_struct, id_float, id_vec2});
  b.op(spv::OpTypePointer, {id_ptr_uc_struct, spv::StorageClassUniformConstant, id_struct});
  b.op(spv::OpTypeInt, {id_int, 32, 1});
  b.op(spv::OpConstant, {id_int, id_const_0, 0});
  b.op(spv::OpTypePointer, {id_ptr_uc_float, spv::StorageClassUniformConstant, id_float});
  b.op(spv::OpVariable, {id_ptr_uc_struct, id_var, spv::StorageClassUniformConstant});
  b.op(spv::OpFunction, {id_void, id_main, spv::FunctionControlMaskNone, id_fnvoid});
  b.op(spv::OpLabel, {id_label});
  b.op(spv::OpAccessChain, {id_ptr_uc_float, id_chain, id_var, id_const_0});
  b.op(spv::OpLoad, {id_float, id_value, id_chain});
  b.op(spv::OpReturn, {});
  b.op(spv::OpFunctionEnd, {});

  InstructionStream stream = b.build(id_bound);
  REQUIRE(stream.validate());

  SpirVTransformer transformer(stream);
  transformer.run(SpirVFlattenStructPass(id_struct));
  CHECK(transformer.validate_db());

  InstructionStream result = transformer.get_result();
  CHECK(result.validate());
  CHECK(count_op(result, spv::OpTypeStruct) == 0);
  CHECK(!has_variable(result, id_var));

  // The members have become individual variables, named after the struct.
  const SpirVResultDatabase &db = transformer.get_db();
  uint32_t var_a = db.find_definition("s.a");
  uint32_t var_b = db.find_definition("s.b");
  REQUIRE(var_a != 0);
  REQUIRE(var_b != 0);
  CHECK(db.get_definition(var_a)._type == ShaderType::FLOAT);
  CHECK(db.get_definition(var_b)._type ==
        ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 2)));

  // The access chain is gone; the load reads the new variable directly.
  CHECK(count_op(result, spv::OpAccessChain) == 0);
  CHECK(find_load_pointer(result, id_float) == var_a);
}
