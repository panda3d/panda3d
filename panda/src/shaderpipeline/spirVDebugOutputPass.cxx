/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVDebugOutputPass.cxx
 * @author rdb
 * @date 2026-02-28
 */

#include "spirVDebugOutputPass.h"
#include "spirVInstructionCursor.h"

/**
 *
 */
SpirVDebugOutputPass::
SpirVDebugOutputPass(Shader::Stage stage, Shader::DebugInfo &debug_info,
                     uint32_t buffer_binding, uint32_t buffer_set) :
  _stage(stage),
  _buffer_binding(buffer_binding),
  _buffer_set(buffer_set),
  _debug_info(debug_info) {
}

/**
 *
 */
void SpirVDebugOutputPass::
run(SpirVModule &module) {
  // Identify the extended instruction sets we care about.  The DebugPrintf
  // import is removed, along with every instruction using it (below).
  pvector<Id> deleted_imports;
  for (size_t i = 0; i < module.get_num_ext_inst_imports(); ++i) {
    const Instruction &import = module.get_ext_inst_import(i);
    Id id(import.args[0]);
    std::string name = import.get_string(1);
    if (name == "NonSemantic.DebugPrintf") {
      _ext_inst_imports[id] = EI_nonsemantic_debugprintf;
      deleted_imports.push_back(id);
    }
    else if (name == "NonSemantic.Shader.DebugInfo.100") {
      _ext_inst_imports[id] = EI_nonsemantic_shader_debuginfo_100;
    }
    else {
      _ext_inst_imports[id] = EI_other;
    }
  }

  // We'll use a lot of uint constants, so keep a table of the existing ones.
  // Also scan for debug source information declared at module scope.
  size_t num_declarations = module.get_num_declarations();
  for (size_t i = 0; i < num_declarations; ++i) {
    Instruction op = module.get_declaration(i);
    if (op.opcode == spv::OpConstant && op.args.size() == 3) {
      if (module.resolve_type(Id(op.args[0])) == ShaderType::UINT) {
        uint32_t value = op.args[2];
        if (value <= 256u) {
          if (value >= _numbers.size()) {
            _numbers.resize(value + 1, Id());
          }
          _numbers[value] = Id(op.args[1]);
        }
      }
    }
    else if (op.opcode == spv::OpExtInst && op.args.size() >= 5 &&
             get_ext_inst_import(op.args[2]) == EI_nonsemantic_shader_debuginfo_100) {
      if (op.args[3] == 35 /*DebugSource*/) {
        _debug_source_files[op.args[1]] = Id(op.args[4]);
      }
    }
  }

  pset<Id> maybe_dead_strings;

  for (Function &function : module.modify_functions()) {
    _current_file = Id();
    _current_line = 0;

    SpirVInstructionCursor cursor(module, function);
    while (cursor.next()) {
      Instruction &op = *cursor;

      if (op.opcode == spv::OpExtInst && op.args.size() >= 4 &&
          get_ext_inst_import(op.args[2]) == EI_nonsemantic_debugprintf &&
          op.args[3] == 1 /*DebugPrintf*/) {

        // The format string and any string arguments may be left without a
        // reference once the print is removed; check below, when all prints
        // have been processed.
        for (size_t ai = 4; ai < op.args.size(); ++ai) {
          if (module.get_definition_type(Id(op.args[ai])) == SpirVModule::DT_string) {
            maybe_dead_strings.insert(Id(op.args[ai]));
          }
        }

#ifndef NDEBUG
        std::string string = op.args.size() >= 5 ? module.resolve_string(Id(op.args[4])) : std::string();
        if (!string.empty()) {
          pvector<uint32_t> fmt_args(op.args.data() + 5, op.args.data() + op.args.size());

          cursor.insert_before([&](SpirVBuilder &builder) {
            if (string == "%!") {
              // Special internal format string that issues an assertion.  The
              // argument contains the expression.
              emit_assert(builder, !fmt_args.empty() ? module.resolve_string(Id(fmt_args[0])) : "");
            } else {
              emit_printf(builder, string, fmt_args.data(), (uint32_t)fmt_args.size());
            }
          });
        }
#endif
        // Remove the print instruction, which tombstones it in place and
        // drops its id from the index.
        cursor.remove();
        continue;
      }
      else if (op.opcode == spv::OpExtInst && op.args.size() >= 4 &&
               get_ext_inst_import(op.args[2]) == EI_nonsemantic_shader_debuginfo_100) {
        if (op.args[3] == 35 /*DebugSource*/ && op.args.size() >= 5) {
          _debug_source_files[op.args[1]] = Id(op.args[4]);
        }
        else if (op.args[3] == 103 /*DebugLine*/ && op.args.size() >= 6) {
          auto it = _debug_source_files.find(op.args[4]);
          if (it != _debug_source_files.end()) {
            _current_file = it->second;
            _current_line = op.args[5];
          } else {
            _current_file = Id();
            _current_line = 0u;
          }
        }
        else if (op.args[3] == 104 /*DebugNoLine*/) {
          _current_file = Id();
          _current_line = 0u;
        }
      }
      else if (op.opcode == spv::OpLine) {
        _current_file = Id(op.args[0]);
        _current_line = op.args[1];
      }
      else if (op.opcode == spv::OpNoLine) {
        _current_file = Id();
        _current_line = 0u;
      }
    }
  }

  for (Id import_id : deleted_imports) {
    module.delete_id(import_id);
  }

  // Delete the strings that the removed prints have orphaned.  A string that
  // is still referenced (eg. by an OpLine or an OpSource) is kept.  This
  // matters beyond tidiness: the NVIDIA driver rejects a SPIR-V module
  // containing an OpString with a "%s" format specifier in it, even when
  // nothing references the string.
  for (Id string_id : maybe_dead_strings) {
    if (module.get_definition_type(string_id) == SpirVModule::DT_string &&
        !module.is_string_referenced(string_id)) {
      module.delete_id(string_id);
    }
  }
}

/**
 * Inserts the code implementing a debug print at the given builder's cursor.
 */
void SpirVDebugOutputPass::
emit_printf(SpirVBuilder &builder, const std::string &fmt_string, const uint32_t *args, uint32_t nargs) {
  SpirVModule &module = builder.get_module();

  if (_buffer_block_var_id == 0) {
    define_buffer_block(module);
  }

  // Write a new string with vectors expanded.
  std::string new_fmt;
  pvector<Id> fmt_args;
  uint32_t arg_i = 0;
  const char *fmt = fmt_string.c_str();
  while (*fmt != 0) {
    char c = *fmt++;
    if (c != '%') {
      new_fmt += c;
      continue;
    }

    c = *fmt++;
    if (c == '%') {
      new_fmt += "%%";
      continue;
    }

    if (c == '?') {
      // Special code indicating current location.
      if (_current_file == 0) {
        new_fmt += "<unknown>";
      } else {
        Filename fn = module.resolve_string(_current_file);
        std::ostringstream str;
        str << new_fmt;
        str << fn.get_basename();
        str << ":";
        str << _current_line;
        new_fmt = std::move(str).str();
      }
      continue;
    }

    if (arg_i >= nargs) {
      // We don't actually have any more arguments, so the rest gets
      // passed through verbatim.
      new_fmt += "%%";
      new_fmt += c;
      continue;
    }

    std::string this_fmt = "%";
    while (c != 0 && !isalpha((unsigned char)c)) {
      this_fmt += c;
      c = *fmt++;
    }
    if (c == 0) {
      break;
    }

    Id arg(args[arg_i++]);
    if (module.get_definition_type(arg) == SpirVModule::DT_string) {
      //TODO: warn if fmt isn't %s
      new_fmt += module.resolve_string(arg);
      continue;
    }

    const ShaderType *arg_type = module.resolve_type(arg);
    nassertd(arg_type != nullptr) continue;

    ShaderType::ScalarType scalar_type;
    uint32_t num_array_elements, num_rows, num_columns;
    if (!arg_type->as_scalar_type(scalar_type, num_array_elements, num_rows, num_columns) || num_rows > 1) {
      //TODO: warn
      continue;
    }

    uint32_t req_components = 1;
    if (c == 'v') {
      c = *fmt++;
      if (c == 0) {
        break;
      }
      if (c >= '0' && c <= '9') {
        req_components = (uint32_t)(c - '0');
        c = *fmt++;
      }
    }
    this_fmt += c;

    ShaderType::ScalarType desired_type = scalar_type;
    if (c == 'u' || c == 'x' || c == 'X' || c == 'o' || c == 'c') {
      desired_type = ShaderType::ST_uint;
    }
    else if (c == 'i' || c == 'd') {
      desired_type = ShaderType::ST_int;
    }
    else if (c == 'a' || c == 'A' || c == 'e' || c == 'E' || c == 'f' || c == 'F' || c == 'g' || c == 'G') {
      desired_type = ShaderType::ST_float;
    }
    else if (c == 's') {
      if (req_components == 1) {
        req_components = num_columns;
      }
      switch (scalar_type) {
      case ShaderType::ST_float:
      case ShaderType::ST_double:
        this_fmt = "%f";
        break;

      case ShaderType::ST_int:
        this_fmt = "%d";
        break;

      case ShaderType::ST_uint:
        this_fmt = "%u";
        break;

      case ShaderType::ST_unknown:
      case ShaderType::ST_bool:
        this_fmt = "%s";
        desired_type = ShaderType::ST_uint;
        break;
      }
    }
    else {
      // Invalid or unsupported, just pass it through verbatim.
      new_fmt += "%";
      new_fmt += this_fmt;
      continue;
    }

    if (desired_type != scalar_type) {
      // Silently convert (eg. int to float).
      arg = builder.op_convert(desired_type, arg);
    }

    if (arg_type->as_vector() != nullptr) {
      for (uint32_t i = 0; i < req_components; ++i) {
        if (i < num_columns) {
          fmt_args.push_back(builder.op_composite_extract(arg, {i}));
        } else {
          fmt_args.push_back(get_uint_constant(module, 0));
        }
        if (i > 0) {
          new_fmt += ", ";
        }
        new_fmt += this_fmt;
      }
    } else {
      // Just broadcast if user requested a vector but it's not
      for (uint32_t i = 0; i < req_components; ++i) {
        fmt_args.push_back(arg);
        if (i > 0) {
          new_fmt += ", ";
        }
        new_fmt += this_fmt;
      }
    }
  }

  // Store the number of arguments for this string
  uint32_t num_args = (uint32_t)fmt_args.size();
  uint32_t string_index = (uint32_t)_debug_info.intern_string(new_fmt);

  Id zero = get_uint_constant(module, 0);
  Id one = get_uint_constant(module, 1);
  Id reservation = get_uint_constant(module, num_args + 1);

  Id counter_ptr = builder.op_access_chain(_buffer_block_var_id, {zero});

  Id uint_type = module.define_type(ShaderType::UINT);
  Id offset = builder.allocate_id();

  builder.insert(spv::OpAtomicIAdd, {uint_type, offset, counter_ptr, one, zero, reservation});

  // We have to skip the assert section, which is a fixed-size 4-word-per-assert
  // section that lives at the beginning of the data array.
  uint32_t assert_count = (uint32_t)_debug_info.get_max_num_asserts();
  if (assert_count > 0) {
    offset = builder.op_add(offset, get_uint_constant(module, assert_count * 4));
  }

  // Get length of the data array in the SSBO
  Id length = builder.allocate_id();
  builder.insert(spv::OpArrayLength, {uint_type, length, _buffer_block_var_id, 1});

  // Bounds check to make sure we don't write outside the SSBO
  Id branch = builder.branch_if(builder.op_compare(spv::OpULessThanEqual, builder.op_add(offset, reservation), length));
    // Store the string identifier (1-based)...
    builder.op_store(builder.op_access_chain(_buffer_block_var_id, {one, offset}), get_uint_constant(module, string_index + 1));

    // ...followed by the format arguments.
    for (uint32_t i = 0; i < (uint32_t)fmt_args.size(); ++i) {
      Id arg = fmt_args[i];
      if (module.get_type_id(arg) != uint_type) {
        // Bitcast to uint if it isn't already.
        arg = builder.op_bitcast(ShaderType::UINT, fmt_args[i]);
      }
      builder.op_store(builder.op_access_chain(_buffer_block_var_id, {one, builder.op_add(offset, get_uint_constant(module, 1 + i))}), arg);
    }
    builder.op_branch(branch);
  builder.branch_endif(branch);
}

/**
 * Inserts the code implementing an assertion at the given builder's cursor.
 */
void SpirVDebugOutputPass::
emit_assert(SpirVBuilder &builder, const std::string &expression) {
  SpirVModule &module = builder.get_module();

  if (_buffer_block_var_id == 0) {
    define_buffer_block(module);
  }

  // Allocate a unique index for this assert.
  uint32_t assert_id;
  {
    Filename fn;
    if (_current_file == 0) {
      fn = "<unknown>";
    } else {
      fn = module.resolve_string(_current_file);
    }
    assert_id = (uint32_t)_debug_info.add_assert(expression, std::move(fn), (int)_current_line, _stage);
  }

  // Write asserts to a special 4-word-sized section in the buffer: one per
  // unique assert, first one being a counter.  The remaining 3 fields are
  // used to store the invocation ID of the first occurrence.
  Id zero = get_uint_constant(module, 0);
  Id one = get_uint_constant(module, 1);

  Id index = assert_id == 0 ? zero : get_uint_constant(module, assert_id * 4);
  Id counter_ptr = builder.op_access_chain(_buffer_block_var_id, {one, index});

  Id uint_type = module.define_type(ShaderType::UINT);
  Id result = builder.allocate_id();
  builder.insert(spv::OpAtomicIIncrement, {uint_type, result, counter_ptr, one, zero});

  // If we were the first occurrence, write the invocation ID as additional
  // helpful context information.
  Id branch = builder.branch_if(builder.op_compare(spv::OpIEqual, result, zero));
    Id invoc[3];

    switch (_stage) {
    case Shader::Stage::VERTEX:
      // Vertex ID, instance ID
      invoc[0] = builder.op_bitcast(ShaderType::UINT, builder.op_load(module.ensure_builtin_input(spv::ExecutionModelVertex, spv::BuiltInVertexId)));
      invoc[1] = builder.op_bitcast(ShaderType::UINT, builder.op_load(module.ensure_builtin_input(spv::ExecutionModelVertex, spv::BuiltInInstanceId)));
      break;

    case Shader::Stage::GEOMETRY:
      // Primitive ID, invocation ID
      {
        invoc[0] = builder.op_bitcast(ShaderType::UINT, builder.op_load(module.ensure_builtin_input(spv::ExecutionModelGeometry, spv::BuiltInPrimitiveId)));
        invoc[1] = builder.op_bitcast(ShaderType::UINT, builder.op_load(module.ensure_builtin_input(spv::ExecutionModelGeometry, spv::BuiltInInvocationId)));
      }
      break;

    case Shader::Stage::TESS_CONTROL:
      // Primitive ID, invocation ID
      {
        invoc[0] = builder.op_bitcast(ShaderType::UINT, builder.op_load(module.ensure_builtin_input(spv::ExecutionModelTessellationControl, spv::BuiltInPrimitiveId)));
        invoc[1] = builder.op_bitcast(ShaderType::UINT, builder.op_load(module.ensure_builtin_input(spv::ExecutionModelTessellationControl, spv::BuiltInInvocationId)));
      }
      break;

    case Shader::Stage::TESS_EVALUATION:
      // Primitive ID, Tess coordinate x and y
      {
        Id primitive_id = module.ensure_builtin_input(spv::ExecutionModelTessellationEvaluation, spv::BuiltInPrimitiveId);
        Id tess_coord_id = module.ensure_builtin_input(spv::ExecutionModelTessellationEvaluation, spv::BuiltInTessCoord);
        invoc[0] = builder.op_bitcast(ShaderType::UINT, builder.op_load(primitive_id));
        invoc[1] = builder.op_bitcast(ShaderType::UINT, builder.op_load(builder.op_access_chain(tess_coord_id, {zero})));
        invoc[2] = builder.op_bitcast(ShaderType::UINT, builder.op_load(builder.op_access_chain(tess_coord_id, {one})));
      }
      break;

    case Shader::Stage::FRAGMENT:
      // Fragment coordinate
      {
        Id frag_coord_id = module.ensure_builtin_input(spv::ExecutionModelFragment, spv::BuiltInFragCoord);
        Id two = get_uint_constant(module, 2);
        invoc[0] = builder.op_bitcast(ShaderType::UINT, builder.op_load(builder.op_access_chain(frag_coord_id, {zero})));
        invoc[1] = builder.op_bitcast(ShaderType::UINT, builder.op_load(builder.op_access_chain(frag_coord_id, {one})));
        invoc[2] = builder.op_bitcast(ShaderType::UINT, builder.op_load(builder.op_access_chain(frag_coord_id, {two})));
      }
      break;

    case Shader::Stage::COMPUTE:
      // Global invocation ID
      {
        Id value = builder.op_load(module.ensure_builtin_input(spv::ExecutionModelGLCompute, spv::BuiltInGlobalInvocationId));
        invoc[0] = builder.op_composite_extract(value, {0});
        invoc[1] = builder.op_composite_extract(value, {1});
        invoc[2] = builder.op_composite_extract(value, {2});
      }
      break;

    default:
      break;
    }

    for (size_t i = 0; i < 3; ++i) {
      if (invoc[i] != 0) {
        Id offset_id = get_uint_constant(module, assert_id * 4 + i + 1);
        builder.op_store(builder.op_access_chain(_buffer_block_var_id, {one, offset_id}), invoc[i]);
      }
    }
    builder.op_branch(branch);
  builder.branch_endif(branch);
}

/**
 * Makes sure the buffer block type is defined.
 */
void SpirVDebugOutputPass::
define_buffer_block(SpirVModule &module) {
  static const ShaderType *block_type = make_buffer_block_type();
  Id block_type_id = module.define_type(block_type);
  Id block_var_id = module.define_variable(block_type, spv::StorageClassUniform);

  module.decorate(block_type_id, spv::DecorationBufferBlock);
  module.decorate(block_var_id, spv::DecorationBinding, _buffer_binding);
  module.decorate(block_var_id, spv::DecorationDescriptorSet, _buffer_set);
  module.decorate(block_var_id, spv::DecorationCoherent);
  _buffer_block_var_id = block_var_id;
}

/**
 *
 */
const ShaderType *SpirVDebugOutputPass::
make_buffer_block_type() {
  const ShaderType *array_type = ShaderType::register_type(ShaderType::Array(ShaderType::UINT, 0, 4));
  ShaderType::Struct struct_type;
  struct_type.add_member(ShaderType::UINT, "count", 0);
  struct_type.add_member(array_type, "data", 4);
  return ShaderType::register_type(std::move(struct_type));
}
