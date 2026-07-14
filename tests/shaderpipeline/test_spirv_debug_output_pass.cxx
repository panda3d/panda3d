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
  // A fragment shader containing just: debugPrintfEXT("value: %f", 1.0);
  SpirVModule module = make_module();
  Id id_import = module.add_ext_inst_import("NonSemantic.DebugPrintf");
  Id id_string = module.add_string("value: %f");
  Id id_void = module.define_type(nullptr);
  Id id_const_one = module.define_float_constant(1.0f);

  {
    SpirVBuilder b = make_entry_point(module);
    b.insert(spv::OpExtInst, {id_void, b.allocate_id(), id_import,
                              1 /*DebugPrintf*/, id_string, id_const_one});
    b.op_return();
  }

  module.add_extension("SPV_KHR_non_semantic_info");
  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  Shader::DebugInfo debug_info;
  SpirVTransformer transformer(stream);
  SpirVDebugOutputPass pass(Shader::Stage::FRAGMENT, debug_info, 3, 0);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();

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
  // A fragment shader containing just: assert(1 < 2), which is encoded as a
  // print with the special format string "%!".
  SpirVModule module = make_module();
  Id id_import = module.add_ext_inst_import("NonSemantic.DebugPrintf");
  Id id_string_fmt = module.add_string("%!");
  Id id_string_expr = module.add_string("1 < 2");
  Id id_void = module.define_type(nullptr);

  {
    SpirVBuilder b = make_entry_point(module);
    b.insert(spv::OpExtInst, {id_void, b.allocate_id(), id_import,
                              1 /*DebugPrintf*/, id_string_fmt, id_string_expr});
    b.op_return();
  }

  module.add_extension("SPV_KHR_non_semantic_info");
  InstructionStream stream = module.emit();
  REQUIRE(stream.validate());

  Shader::DebugInfo debug_info;
  debug_info.set_max_num_asserts(1);

  SpirVTransformer transformer(stream);
  SpirVDebugOutputPass pass(Shader::Stage::FRAGMENT, debug_info, 3, 0);
  transformer.run(pass);
  CHECK(transformer.get_module().validate());

  InstructionStream result = transformer.get_result();
  CHECK(count_op(result, spv::OpExtInst) == 0);
  CHECK(count_op(result, spv::OpExtInstImport) == 0);

#ifndef NDEBUG
  // The assertion was registered...
  CHECK(!debug_info.empty());

  // ...and replaced with an atomic counter increment, plus a write of the
  // fragment coordinate for the first failing invocation.
  CHECK(count_op(result, spv::OpAtomicIIncrement) == 1);

  bool found_frag_coord = false;
  const SpirVModule &out_module = transformer.get_module();
  for (uint32_t word = 0; word < transformer.get_id_bound(); ++word) {
    Id id(word);
    if (out_module.get_definition_type(id) == SpirVModule::DT_variable &&
        out_module.get_builtin(id) == spv::BuiltInFragCoord) {
      found_frag_coord = true;
    }
  }
  CHECK(found_frag_coord);
#endif
}
