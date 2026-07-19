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

#include "spirVModule.h"

class SpirVTransformPass;

/**
 * Materializes a SPIR-V instruction stream into a SpirVModule, runs
 * transformation passes and common small transformations on it, and
 * serializes the result back out.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVTransformer {
public:
  using Id = SpirVId;
  using InstructionStream = ShaderModuleSpirV::InstructionStream;

  SpirVTransformer(const InstructionStream &stream);

  void run(SpirVTransformPass &pass);
  INLINE void run(SpirVTransformPass &&pass);

  InstructionStream get_result() const;

  INLINE uint32_t get_id_bound() const;
  INLINE SpirVModule &get_module();
  INLINE const SpirVModule &get_module() const;

  void assign_interface_locations(ShaderModule::Stage stage);
  void assign_locations(pmap<uint32_t, int> locations);
  void assign_flat_decorations();
  void assign_procedural_names(const pmap<uint32_t, int> &uniform_locations,
                               unsigned int stage_index);
  void strip_uniform_locations();
  void strip_bindings();
  void bind_descriptor_set(uint32_t set, const pvector<uint32_t> &ids);

private:
  SpirVModule _module;
};

#include "spirVTransformer.I"

#endif
