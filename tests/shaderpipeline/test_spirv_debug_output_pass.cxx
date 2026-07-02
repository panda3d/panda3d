/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_debug_output_pass.cxx
 * @author rdb
 * @date 2026-07-02
 */

#include "spirVDebugOutputPass.h"

#include "spirv_test_utils.h"
#include "catch_amalgamated.hpp"

TEST_CASE("SpirVDebugOutputPass converts prints to buffer writes", "[shaderpipeline]") {
  enum : uint32_t {
    id_import = 1, id_main, id_void, id_fnvoid, id_float, id_const_one,
    id_string, id_label, id_result,
    id_bound,
  };

  // A fragment shader containing just: debugPrintfEXT("value: %f", 1.0);
  ModuleBuilder b;
  b.op(spv::OpCapability, {spv::CapabilityShader});
  b.op(spv::OpExtension, {}, "SPV_KHR_non_semantic_info");
  b.op(spv::OpExtInstImport, {id_import}, "NonSemantic.DebugPrintf");
  b.op(spv::OpMemoryModel, {spv::AddressingModelLogical, spv::MemoryModelGLSL450});
  b.op(spv::OpEntryPoint, {spv::ExecutionModelFragment, id_main}, "main");
  b.op(spv::OpExecutionMode, {id_main, spv::ExecutionModeOriginUpperLeft});
  b.op(spv::OpString, {id_string}, "value: %f");
  b.op(spv::OpTypeVoid, {id_void});
  b.op(spv::OpTypeFunction, {id_fnvoid, id_void});
  b.op(spv::OpTypeFloat, {id_float, 32});
  b.op(spv::OpConstant, {id_float, id_const_one, 0x3f800000u});
  b.op(spv::OpFunction, {id_void, id_main, spv::FunctionControlMaskNone, id_fnvoid});
  b.op(spv::OpLabel, {id_label});
  b.op(spv::OpExtInst, {id_void, id_result, id_import, 1 /*DebugPrintf*/, id_string, id_const_one});
  b.op(spv::OpReturn, {});
  b.op(spv::OpFunctionEnd, {});

  InstructionStream stream = b.build(id_bound);
  REQUIRE(stream.validate());

  Shader::DebugInfo debug_info;
  SpirVTransformer transformer(stream);
  SpirVDebugOutputPass pass(Shader::Stage::FRAGMENT, debug_info, 3, 0);
  transformer.run(pass);
  CHECK(transformer.validate_db());

  InstructionStream result = transformer.get_result();
  CHECK(result.validate());

  // The print invocation and the instruction set import are gone.
  CHECK(count_op(result, spv::OpExtInst) == 0);
  CHECK(count_op(result, spv::OpExtInstImport) == 0);

#ifndef NDEBUG
  // The format string was interned...
  REQUIRE(debug_info.get_num_strings() == 1);
  CHECK(debug_info.get_string(0) == "value: %f");
  CHECK(debug_info.get_max_num_asserts() == 0);

  // ...and the invocation replaced with bounds-checked writes into a new
  // buffer block.
  CHECK(count_op(result, spv::OpVariable) == 1);
  CHECK(count_op(result, spv::OpAtomicIAdd) == 1);
  CHECK(count_op(result, spv::OpArrayLength) == 1);
#endif
}

TEST_CASE("SpirVDebugOutputPass converts asserts to buffer writes", "[shaderpipeline]") {
  enum : uint32_t {
    id_import = 1, id_main, id_void, id_fnvoid,
    id_string_fmt, id_string_expr, id_label, id_result,
    id_bound,
  };

  // A fragment shader containing just: assert(1 < 2), which is encoded as a
  // print with the special format string "%!".
  ModuleBuilder b;
  b.op(spv::OpCapability, {spv::CapabilityShader});
  b.op(spv::OpExtension, {}, "SPV_KHR_non_semantic_info");
  b.op(spv::OpExtInstImport, {id_import}, "NonSemantic.DebugPrintf");
  b.op(spv::OpMemoryModel, {spv::AddressingModelLogical, spv::MemoryModelGLSL450});
  b.op(spv::OpEntryPoint, {spv::ExecutionModelFragment, id_main}, "main");
  b.op(spv::OpExecutionMode, {id_main, spv::ExecutionModeOriginUpperLeft});
  b.op(spv::OpString, {id_string_fmt}, "%!");
  b.op(spv::OpString, {id_string_expr}, "1 < 2");
  b.op(spv::OpTypeVoid, {id_void});
  b.op(spv::OpTypeFunction, {id_fnvoid, id_void});
  b.op(spv::OpFunction, {id_void, id_main, spv::FunctionControlMaskNone, id_fnvoid});
  b.op(spv::OpLabel, {id_label});
  b.op(spv::OpExtInst, {id_void, id_result, id_import, 1 /*DebugPrintf*/, id_string_fmt, id_string_expr});
  b.op(spv::OpReturn, {});
  b.op(spv::OpFunctionEnd, {});

  InstructionStream stream = b.build(id_bound);
  REQUIRE(stream.validate());

  Shader::DebugInfo debug_info;
  debug_info.set_max_num_asserts(1);

  SpirVTransformer transformer(stream);
  SpirVDebugOutputPass pass(Shader::Stage::FRAGMENT, debug_info, 3, 0);
  transformer.run(pass);
  CHECK(transformer.validate_db());

  InstructionStream result = transformer.get_result();
  CHECK(result.validate());
  CHECK(count_op(result, spv::OpExtInst) == 0);

#ifndef NDEBUG
  // The assertion was registered...
  CHECK(!debug_info.empty());

  // ...and replaced with an atomic counter increment, plus a write of the
  // fragment coordinate for the first failing invocation.
  CHECK(count_op(result, spv::OpAtomicIIncrement) == 1);

  bool found_frag_coord = false;
  const SpirVResultDatabase &db = transformer.get_db();
  for (uint32_t id = 0; id < transformer.get_id_bound(); ++id) {
    const SpirVResultDatabase::Definition &def = db.get_definition(id);
    if (def.is_variable() && def._builtin == spv::BuiltInFragCoord) {
      found_frag_coord = true;
    }
  }
  CHECK(found_frag_coord);
#endif
}
