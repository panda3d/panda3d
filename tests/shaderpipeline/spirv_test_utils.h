/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirv_test_utils.h
 * @author rdb
 * @date 2026-07-02
 */

#ifndef SPIRV_TEST_UTILS_H
#define SPIRV_TEST_UTILS_H

#include "spirVTransformer.h"
#include "spirVBuilder.h"
#include "shaderType.h"

using Instruction = ShaderModuleSpirV::Instruction;
using InstructionStream = ShaderModuleSpirV::InstructionStream;
using Id = SpirVId;

/**
 * Creates an empty module with the standard capability and memory model.
 */
inline SpirVModule
make_module() {
  SpirVModule module;
  module.add_capability(spv::CapabilityShader);
  module.set_memory_model(spv::AddressingModelLogical, spv::MemoryModelGLSL450);
  return module;
}

/**
 * Starts a void function and adds it as an entry point for the given execution
 * model.  The returned builder is positioned at the beginning of the empty
 * function body, after the initial OpLabel.  The caller is responsible for
 * adding a terminator (such as with op_return).
 * The entry point interface is derived from the body's references at emit.
 */
inline SpirVBuilder
make_entry_point(SpirVModule &module,
                 spv::ExecutionModel model = spv::ExecutionModelFragment,
                 const char *name = "main") {
  SpirVBuilder builder = module.make_function(nullptr);
  Id function_id = builder.get_current_function_id();
  module.add_entry_point(model, function_id, name);
  if (model == spv::ExecutionModelFragment) {
    module.add_execution_mode(function_id, spv::ExecutionModeOriginUpperLeft);
  }
  return builder;
}

/**
 * Counts the instructions with the given opcode.
 */
inline int
count_op(InstructionStream &stream, spv::Op opcode) {
  int count = 0;
  for (Instruction op : stream) {
    if (op.opcode == opcode) {
      ++count;
    }
  }
  return count;
}

/**
 * Returns the index'th instruction with the given opcode.  The returned
 * Instruction points into the stream, which must remain alive and unmodified
 * while the Instruction is used.  Returns an OpNop instruction if no match.
 */
inline Instruction
find_op(InstructionStream &stream, spv::Op opcode, int index = 0) {
  for (Instruction op : stream) {
    if (op.opcode == opcode && index-- == 0) {
      return op;
    }
  }
  return Instruction {spv::OpNop, 0, nullptr};
}

/**
 * Returns true if the module still defines a variable with this id.
 */
inline bool
has_variable(InstructionStream &stream, uint32_t id) {
  for (Instruction op : stream) {
    if (op.opcode == spv::OpVariable && op.args[1] == id) {
      return true;
    }
  }
  return false;
}

/**
 * Returns the pointer operand of the only OpLoad with the given result type.
 */
inline uint32_t
find_load_pointer(InstructionStream &stream, uint32_t type_id) {
  for (Instruction op : stream) {
    if (op.opcode == spv::OpLoad && op.args[0] == type_id) {
      return op.args[2];
    }
  }
  return 0;
}

#endif
