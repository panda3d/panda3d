/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVDebugOutputPass.h
 * @author rdb
 * @date 2026-02-28
 */

#ifndef SPIRVDEBUGOUTPUTPASS_H
#define SPIRVDEBUGOUTPUTPASS_H

#include "spirVTransformPass.h"
#include "pset.h"
#include "stl_compares.h"
#include "vector_string.h"
#include "shader.h"

/**
 * Adds support for debug printing from shaders.  This works by replacing
 * debugOutputfEXT invocations with writes to an SSBO.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVDebugOutputPass final : public SpirVTransformPass {
public:
  SpirVDebugOutputPass(Shader::Stage stage, Shader::DebugInfo &debug_info,
                       uint32_t buffer_binding, uint32_t buffer_set);

  virtual bool transform_debug_op(Instruction op);
  virtual bool transform_definition_op(Instruction op);
  virtual bool transform_function_op(Instruction op);

private:
  void emit_printf(const std::string &fmt_string, const uint32_t *args, uint32_t nargs);
  void emit_assert(const std::string &expression);

  void define_buffer_block();
  static const ShaderType *make_buffer_block_type();

  enum ExtInstImport {
    EI_other,
    EI_nonsemantic_debugprintf,
    EI_nonsemantic_shader_debuginfo_100,
  };

  INLINE ExtInstImport get_ext_inst_import(uint32_t id) const;
  INLINE uint32_t get_uint_constant(uint32_t v);

private:
  Shader::Stage _stage;
  uint32_t _buffer_binding = 0u;
  uint32_t _buffer_set = 0u;
  uint32_t _buffer_block_var_id = 0u;

  Shader::DebugInfo &_debug_info;

  pvector<uint32_t> _numbers;
  pmap<uint32_t, ExtInstImport> _ext_inst_imports;
  pmap<uint32_t, uint32_t> _debug_source_files;

  uint32_t _current_file = 0;
  uint32_t _current_line = 0;
};

#include "spirVDebugOutputPass.I"

#endif
