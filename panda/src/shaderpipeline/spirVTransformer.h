/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVTransformer.h
 * @author rdb
 * @date 2024-10-08
 */

#ifndef SPIRVTRANSFORMER_H
#define SPIRVTRANSFORMER_H

#include "shaderModuleSpirV.h"
#include "spirVResultDatabase.h"

class SpirVTransformPass;

/**
 * A SpirVTransformer can be used for more advanced transformations on a SPIR-V
 * instruction stream.  It sets up temporary support structures that help make
 * changes more efficiently.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVTransformer {
public:
  using Definition = SpirVResultDatabase::Definition;
  using MemberDefinition = SpirVResultDatabase::MemberDefinition;
  using Instruction = ShaderModuleSpirV::Instruction;
  using InstructionStream = ShaderModuleSpirV::InstructionStream;
  using InstructionIterator = ShaderModuleSpirV::InstructionIterator;

  SpirVTransformer(const InstructionStream &stream);

  void run(SpirVTransformPass &pass);
  INLINE void run(SpirVTransformPass &&pass);

  InstructionStream get_result() const;

  INLINE uint32_t get_id_bound() const;
  INLINE const SpirVResultDatabase &get_db() const;

  void assign_interface_locations(ShaderModule::Stage stage);
  void assign_locations(pmap<uint32_t, int> locations);
  void assign_procedural_names(const char *prefix, const pmap<uint32_t, int> &suffixes);
  void strip_uniform_locations();
  void strip_bindings();
  void bind_descriptor_set(uint32_t set, const pvector<uint32_t> &ids);

private:
  // Stores the module split into the different sections for easier
  // concurrent modification of the various sections.
  std::vector<uint32_t> _preamble;
  std::vector<uint32_t> _annotations;
  std::vector<uint32_t> _definitions;
  std::vector<uint32_t> _functions;

  // Keeps track of the different definitions.
  SpirVResultDatabase _db;
};

#include "spirVTransformer.I"

#endif
