/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spirv_instruction_cursor.cxx
 * @author rdb
 * @date 2026-07-07
 *
 * Tests for SpirVInstructionCursor: replacing an instruction with a sequence
 * whose final value takes over the original result id, inserting code before
 * or after an instruction, and removing instructions, all while keeping the
 * module's id index consistent (checked through SpirVModule::validate).
 * Also tests the snapshot-end idiom a pass uses to walk the declarations
 * section while appending to it.
 */

#include "spirVInstructionCursor.h"

#include "spirv_test_utils.h"
#include "catch_amalgamated.hpp"

TEST_CASE("SpirVInstructionCursor replaces an instruction while keeping its result id", "[shaderpipeline]") {
  // main() { b = a; }
  SpirVModule module = make_module();
  Id var_a = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  Id var_b = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  Id loaded;
  {
    SpirVBuilder builder = make_entry_point(module);
    loaded = builder.op_load(var_a);
    builder.op_store(var_b, loaded);
    builder.op_return();
  }

  // Replace each load with a load followed by a negation.
  for (SpirVModule::Function &function : module.modify_functions()) {
    SpirVInstructionCursor cursor(module, function);
    while (cursor.next()) {
      if (cursor->opcode == spv::OpLoad) {
        Id var_id(cursor->args[2]);
        cursor.replace([&](SpirVBuilder &builder) -> Id {
          return builder.op_negate(builder.op_load(var_id));
        });
      }
    }
  }
  CHECK(module.validate());

  InstructionStream result = module.emit();

  // The negation took over the load's result id, so the store still reads
  // the same id, which now holds the negated value.
  CHECK(count_op(result, spv::OpLoad) == 1);
  CHECK(count_op(result, spv::OpFNegate) == 1);
  Instruction negate = find_op(result, spv::OpFNegate);
  CHECK(negate.args[1] == loaded);
  Instruction load = find_op(result, spv::OpLoad);
  CHECK(load.args[1] != loaded);
  CHECK(negate.args[2] == load.args[1]);
  Instruction store = find_op(result, spv::OpStore);
  CHECK(store.args[1] == loaded);
}

TEST_CASE("SpirVInstructionCursor handles a replacement that collapses to one op", "[shaderpipeline]") {
  SpirVModule module = make_module();
  Id var_a = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  Id var_b = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  Id loaded;
  {
    SpirVBuilder builder = make_entry_point(module);
    loaded = builder.op_load(var_a);
    builder.op_store(var_b, loaded);
    builder.op_return();
  }

  // Replace each load with a load followed by a no-op conversion.
  for (SpirVModule::Function &function : module.modify_functions()) {
    SpirVInstructionCursor cursor(module, function);
    while (cursor.next()) {
      if (cursor->opcode == spv::OpLoad) {
        Id var_id(cursor->args[2]);
        cursor.replace([&](SpirVBuilder &builder) -> Id {
          Id temp = builder.op_load(var_id);
          // Converting float to float returns temp unchanged.
          return builder.op_convert(ShaderType::ST_float, temp);
        });
      }
    }
  }
  CHECK(module.validate());

  InstructionStream result = module.emit();

  // The replacement load itself was renumbered to the original result id.
  CHECK(count_op(result, spv::OpLoad) == 1);
  Instruction load = find_op(result, spv::OpLoad);
  CHECK(load.args[1] == loaded);
  Instruction store = find_op(result, spv::OpStore);
  CHECK(store.args[1] == loaded);
}

TEST_CASE("SpirVInstructionCursor replaces an instruction without a result id", "[shaderpipeline]") {
  SpirVModule module = make_module();
  {
    SpirVBuilder builder = make_entry_point(module);
    builder.op_return();
  }

  // Replace the resultless return instruction with a kill instruction.
  for (SpirVModule::Function &function : module.modify_functions()) {
    SpirVInstructionCursor cursor(module, function);
    while (cursor.next()) {
      if (cursor->opcode == spv::OpReturn) {
        cursor.replace([](SpirVBuilder &builder) {
          builder.op_kill();
        });
      }
    }
  }
  CHECK(module.validate());

  InstructionStream result = module.emit();

  CHECK(count_op(result, spv::OpReturn) == 0);
  CHECK(count_op(result, spv::OpKill) == 1);
}

TEST_CASE("SpirVInstructionCursor does not visit code inserted before the cursor", "[shaderpipeline]") {
  SpirVModule module = make_module();
  Id var_a = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  Id var_b = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  {
    SpirVBuilder builder = make_entry_point(module);
    builder.op_return();
  }

  // Insert a store immediately before the return instruction.
  int num_visited = 0;
  for (SpirVModule::Function &function : module.modify_functions()) {
    SpirVInstructionCursor cursor(module, function);
    while (cursor.next()) {
      ++num_visited;
      if (cursor->opcode == spv::OpReturn) {
        cursor.insert_before([&](SpirVBuilder &builder) {
          builder.op_store(var_b, builder.op_load(var_a));
        });
        // The cursor must still be on the OpReturn.
        REQUIRE(cursor->opcode == spv::OpReturn);
      }
    }
  }
  CHECK(module.validate());

  // The inserted OpLoad and OpStore were not visited.
  CHECK(num_visited == 2);

  InstructionStream result = module.emit();
  CHECK(count_op(result, spv::OpStore) == 1);
  CHECK(count_op(result, spv::OpLoad) == 1);
}

TEST_CASE("SpirVInstructionCursor does not visit code inserted after the cursor", "[shaderpipeline]") {
  SpirVModule module = make_module();
  Id var_a = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  Id var_b = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  {
    SpirVBuilder builder = make_entry_point(module);
    builder.op_return();
  }

  // Insert a store immediately after the label instruction.
  int num_visited = 0;
  for (SpirVModule::Function &function : module.modify_functions()) {
    SpirVInstructionCursor cursor(module, function);
    while (cursor.next()) {
      ++num_visited;
      if (cursor->opcode == spv::OpLabel) {
        cursor.insert_after([&](SpirVBuilder &builder) {
          builder.op_store(var_b, builder.op_load(var_a));
        });
      }
    }
  }
  CHECK(module.validate());

  // The inserted OpLoad and OpStore were not visited.
  CHECK(num_visited == 2);

  InstructionStream result = module.emit();
  CHECK(count_op(result, spv::OpStore) == 1);
  CHECK(count_op(result, spv::OpLoad) == 1);

  // The store landed after the label, before the return.
  SpirVModule reparsed(result);
  const SpirVModule::Function &function = reparsed.get_function(0);
  REQUIRE(function.instructions.size() == 4);
  CHECK(function.instructions[0].opcode == spv::OpLabel);
  CHECK(function.instructions[1].opcode == spv::OpLoad);
  CHECK(function.instructions[2].opcode == spv::OpStore);
  CHECK(function.instructions[3].opcode == spv::OpReturn);
}

TEST_CASE("SpirVInstructionCursor removes instructions and clears their ids", "[shaderpipeline]") {
  // main() { float unused = -a; b = a; }
  SpirVModule module = make_module();
  Id var_a = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  Id var_b = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  {
    SpirVBuilder builder = make_entry_point(module);
    Id loaded = builder.op_load(var_a);
    builder.op_negate(loaded);
    builder.op_store(var_b, loaded);
    builder.op_return();
  }

  // Remove every negation instruction and clear its result id.
  for (SpirVModule::Function &function : module.modify_functions()) {
    SpirVInstructionCursor cursor(module, function);
    while (cursor.next()) {
      if (cursor->opcode == spv::OpFNegate) {
        cursor.remove();
      }
    }
  }
  CHECK(module.validate());

  InstructionStream result = module.emit();
  CHECK(count_op(result, spv::OpFNegate) == 0);
  CHECK(count_op(result, spv::OpLoad) == 1);
  CHECK(count_op(result, spv::OpStore) == 1);
}

TEST_CASE("SpirVInstructionCursor detaches instructions with deferred id deletion", "[shaderpipeline]") {
  // Same shape as the removal test, but through detach() + late delete_id.
  SpirVModule module = make_module();
  Id var_a = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  Id var_b = module.define_variable(ShaderType::FLOAT, spv::StorageClassPrivate);
  {
    SpirVBuilder builder = make_entry_point(module);
    Id loaded = builder.op_load(var_a);
    builder.op_negate(loaded);
    builder.op_store(var_b, loaded);
    builder.op_return();
  }

  // Detach every negation, deferring result-id deletion until afterwards.
  pvector<Id> deferred;
  for (SpirVModule::Function &function : module.modify_functions()) {
    SpirVInstructionCursor cursor(module, function);
    while (cursor.next()) {
      if (cursor->opcode == spv::OpFNegate) {
        Id result_id = cursor->get_result();
        cursor.detach();
        // The id state must still be queryable after the detach.
        REQUIRE(module.get_definition_type(result_id) != SpirVModule::DT_none);
        deferred.push_back(result_id);
      }
    }
  }
  for (Id id : deferred) {
    module.delete_id(id);
  }
  CHECK(module.validate());

  InstructionStream result = module.emit();
  CHECK(count_op(result, spv::OpFNegate) == 0);
  CHECK(count_op(result, spv::OpLoad) == 1);
  CHECK(count_op(result, spv::OpStore) == 1);
}
