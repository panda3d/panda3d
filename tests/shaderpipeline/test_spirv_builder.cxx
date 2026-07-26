/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_builder.cxx
 * @author rdb
 * @date 2026-07-14
 */

#include "catch_amalgamated.hpp"

#include "spirv_test_utils.h"

TEST_CASE("SpirVBuilder positions at the start of a function body", "[shaderpipeline]") {
  // A function whose entry block declares a local variable; code inserted at
  // the body start must land after the OpVariable, not before it.
  SpirVModule module = make_module();
  Id var_in = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  SpirVBuilder main_builder = make_entry_point(module);
  Id main_id = main_builder.get_current_function_id();
  main_builder.op_return();
  {
    // Add a function-local variable right after the entry label.
    SpirVModule::Function *function = module.find_function(main_id);
    REQUIRE(function != nullptr);
    size_t label_index = 0;
    while (function->instructions[label_index].opcode != spv::OpLabel) {
      ++label_index;
    }
    SpirVBuilder builder(module, main_id, label_index + 1);
    Id local_ptr_tid = module.define_pointer_type(ShaderType::FLOAT, spv::StorageClassFunction);
    Id local_id = module.allocate_id();
    builder.insert(spv::OpVariable, {local_ptr_tid, local_id, (uint32_t)spv::StorageClassFunction});

    // Now insert a store "at the body start"; it must go after the variable.
    SpirVBuilder body_builder(module);
    body_builder.set_insertion_point_to_body_start(main_id);
    body_builder.op_store(local_id, body_builder.op_load(var_in));
  }

  InstructionStream stream = module.emit();
  CHECK(stream.validate());
}

TEST_CASE("SpirVBuilder construct broadcasts one value into a runtime type", "[shaderpipeline]") {
  SpirVModule module = make_module();

  const ShaderType *uvec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_uint, 3));

  Id id_result;
  {
    SpirVBuilder builder = make_entry_point(module);
    id_result = builder.construct(uvec3_type, builder.define_private(ShaderType::UINT).load());
    builder.op_return();
  }

  CHECK(module.resolve_type(id_result) == uvec3_type);

  InstructionStream stream = module.emit();
  Instruction op = find_op(stream, spv::OpCompositeConstruct);
  REQUIRE(op.nargs == 5);
  CHECK(op.args[2] == op.args[3]);
  CHECK(op.args[3] == op.args[4]);
  CHECK(stream.validate());
}

TEST_CASE("SpirVBuilder construct accepts numbers in any position", "[shaderpipeline]") {
  // The builder supplies the module a constant needs, so unlike the vecN
  // constructors this needs no expression to anchor it.
  SpirVModule module = make_module();

  const ShaderType *vec3_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_float, 3));
  const ShaderType *ivec2_type = ShaderType::register_type(
    ShaderType::Vector(ShaderType::ST_int, 2));

  Id id_mixed, id_all_numbers, id_broadcast;
  {
    SpirVBuilder builder = make_entry_point(module);
    id_mixed = builder.construct(vec3_type,
      {0.0f, builder.define_private(ShaderType::FLOAT).load(), 1.0f});
    id_all_numbers = builder.construct(ivec2_type, {-1, 2});
    id_broadcast = builder.construct(vec3_type, 0.5f);
    builder.op_return();
  }

  CHECK(module.resolve_type(id_mixed) == vec3_type);
  CHECK(module.resolve_type(id_all_numbers) == ivec2_type);
  CHECK(module.resolve_type(id_broadcast) == vec3_type);

  InstructionStream stream = module.emit();
  // An all-number construct() still emits an instruction; it is not quietly
  // folded into the module-scope constant that const_vec3() would define.
  CHECK(count_op(stream, spv::OpCompositeConstruct) == 3);
  CHECK(count_op(stream, spv::OpConstantComposite) == 0);
  CHECK(stream.validate());
}

TEST_CASE("SpirVBuilder branch_endif terminates a fall-through block", "[shaderpipeline]") {
  // main() { if (true) { b = a; } }
  SpirVModule module = make_module();
  Id merge;
  {
    SpirVBuilder builder = make_entry_point(module);
    Pointer var_a = builder.define_private(ShaderType::FLOAT);
    Pointer var_b = builder.define_private(ShaderType::FLOAT);
    Id cond = module.define_constant(ShaderType::BOOL, 1);
    merge = builder.branch_if(cond);
    var_b.store(var_a.load());
    builder.branch_endif(merge);
    builder.op_return();
  }

  InstructionStream stream = module.emit();
  CHECK(stream.validate());

  // The store does not terminate the true block, so branch_endif must have
  // inserted the branch to the merge block itself.
  REQUIRE(count_op(stream, spv::OpBranch) == 1);
  Instruction branch = find_op(stream, spv::OpBranch);
  CHECK(branch.args[0] == merge);
}
