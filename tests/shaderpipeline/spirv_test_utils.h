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
#include "shaderType.h"

#include <cstring>

using Instruction = ShaderModuleSpirV::Instruction;
using InstructionStream = ShaderModuleSpirV::InstructionStream;

/**
 * Assembles a SPIR-V module word-by-word, prepending the module header.
 */
class ModuleBuilder {
public:
  void op(spv::Op opcode, std::initializer_list<uint32_t> args) {
    _words.push_back(((uint32_t)args.size() + 1u) << spv::WordCountShift | (uint32_t)opcode);
    _words.insert(_words.end(), args.begin(), args.end());
  }

  // Same, but packs a string literal after the given args (as used by OpName,
  // OpEntryPoint, etc.), optionally followed by further arguments.
  void op(spv::Op opcode, std::initializer_list<uint32_t> args, const char *str,
           std::initializer_list<uint32_t> more = {}) {
    size_t str_words = std::strlen(str) / 4 + 1;
    _words.push_back((uint32_t)(args.size() + str_words + more.size() + 1u) << spv::WordCountShift | (uint32_t)opcode);
    _words.insert(_words.end(), args.begin(), args.end());
    size_t offset = _words.size();
    _words.resize(offset + str_words, 0u);
    std::memcpy(&_words[offset], str, std::strlen(str));
    _words.insert(_words.end(), more.begin(), more.end());
  }

  InstructionStream build(uint32_t id_bound) const {
    std::vector<uint32_t> words({spv::MagicNumber, 0x10000u, 0u, id_bound, 0u});
    words.insert(words.end(), _words.begin(), _words.end());
    return InstructionStream(std::move(words));
  }

private:
  std::vector<uint32_t> _words;
};

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
