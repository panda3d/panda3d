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
bool SpirVDebugOutputPass::
transform_debug_op(Instruction op) {
  if (op.opcode == spv::OpExtension) {
    // Would love to strip this, but there might be other NonSemantic
    // instruction sets in use.
    /*if (strcmp((const char*)&op.args[0], "SPV_KHR_non_semantic_info") == 0) {
      return false;
    }*/
  }
  else if (op.opcode == spv::OpExtInstImport) {
    ExtInstImport imp = EI_other;
    bool keep = true;
    if (strcmp((const char*)&op.args[1], "NonSemantic.DebugPrintf") == 0) {
      imp = EI_nonsemantic_debugprintf;
      keep = false;
    }
    else if (strcmp((const char*)&op.args[1], "NonSemantic.Shader.DebugInfo.100") == 0) {
      imp = EI_nonsemantic_shader_debuginfo_100;
      keep = true;
    }
    _ext_inst_imports[op.args[0]] = imp;
    return keep;
  }
  return true;
}

/**
 *
 */
bool SpirVDebugOutputPass::
transform_definition_op(Instruction op) {
  if (op.opcode == spv::OpConstant && op.nargs == 3) {
    // We'll use a lot of uint constants, so keep a table.
    if (resolve_type(op.args[0]) == ShaderType::UINT) {
      uint32_t value = op.args[2];
      if (value <= 256u) {
        if (value >= _numbers.size()) {
          _numbers.resize(value + 1, 0u);
        }
        _numbers[value] = op.args[1];
      }
    }
  }
  else if (op.opcode == spv::OpExtInst &&
           get_ext_inst_import(op.args[2]) == EI_nonsemantic_shader_debuginfo_100) {
    if (op.args[3] == 35 /*DebugSource*/) {
      _debug_source_files[op.args[1]] = op.args[4];
    }
  }
  return true;
}

/**
 *
 */
bool SpirVDebugOutputPass::
transform_function_op(Instruction op) {
  if (op.opcode == spv::OpExtInst &&
      get_ext_inst_import(op.args[2]) == EI_nonsemantic_debugprintf &&
      op.args[3] == 1 /*DebugPrintf*/) {

#ifndef NDEBUG
    std::string string = resolve_string(op.args[4]);
    if (string.empty()) {
      return false;
    }

    if (string == "%!") {
      // Special internal format string that issues an assertion.  The argument
      // contains the expression.
      emit_assert(op.nargs >= 6 ? resolve_string(op.args[5]) : "");
      return false;
    }

    emit_printf(string, op.args + 5, op.nargs - 5);
#endif
    return false;
  }
  else if (op.opcode == spv::OpExtInst &&
           get_ext_inst_import(op.args[2]) == EI_nonsemantic_shader_debuginfo_100) {
    if (op.args[3] == 35 /*DebugSource*/) {
      _debug_source_files[op.args[0]] = op.args[4];
    }
    else if (op.args[3] == 103 /*DebugLine*/) {
      auto it = _debug_source_files.find(op.args[4]);
      if (it != _debug_source_files.end()) {
        _current_file = it->second;
        _current_line = op.args[5];
      } else {
        _current_file = 0u;
        _current_line = 0u;
      }
    }
    else if (op.args[3] == 104 /*DebugNoLine*/) {
      _current_file = 0u;
      _current_line = 0u;
    }
  }
  else if (op.opcode == spv::OpLine) {
    _current_file = op.args[0];
    _current_line = op.args[1];
  }
  else if (op.opcode == spv::OpNoLine) {
    _current_file = 0u;
    _current_line = 0u;
  }

  return SpirVTransformPass::transform_function_op(op);
}

/**
 *
 */
void SpirVDebugOutputPass::
emit_printf(const std::string &fmt_string, const uint32_t *args, uint32_t nargs) {
  if (_buffer_block_var_id == 0) {
    define_buffer_block();
  }

  // Write a new string with vectors expanded.
  std::string new_fmt;
  pvector<uint32_t> fmt_args;
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
        Filename fn = resolve_string(_current_file);
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

    uint32_t arg = args[arg_i++];
    const Definition &arg_def = _db.get_definition(arg);
    if (arg_def.is_string()) {
      //TODO: warn if fmt isn't %s
      new_fmt += arg_def._name;
      continue;
    }

    const ShaderType *arg_type = resolve_type(arg_def._type_id);

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
      arg = op_convert(desired_type, arg);
    }

    if (arg_type->as_vector() != nullptr) {
      for (uint32_t i = 0; i < req_components; ++i) {
        if (i < num_columns) {
          fmt_args.push_back(op_composite_extract(arg, {i}));
        } else {
          fmt_args.push_back(get_uint_constant(0));
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

  uint32_t zero = get_uint_constant(0);
  uint32_t one = get_uint_constant(1);
  uint32_t reservation = get_uint_constant(num_args + 1);

  uint32_t counter_ptr = op_access_chain(_buffer_block_var_id, {zero});

  uint32_t uint_type = define_type(ShaderType::UINT);
  uint32_t offset = allocate_id();

  add_instruction(spv::OpAtomicIAdd, {uint_type, offset, counter_ptr, one, zero, reservation});
  {
    Definition &def = _db.modify_definition(offset);
    def._type_id = uint_type;
    def._type = ShaderType::UINT;
  }

  // We have to skip the assert section, which is a fixed-size 4-word-per-assert
  // section that lives at the beginning of the data array.
  uint32_t assert_count = (uint32_t)_debug_info.get_max_num_asserts();
  if (assert_count > 0) {
    offset = op_add(offset, get_uint_constant(assert_count * 4));
  }

  // Get length of the data array in the SSBO
  uint32_t length = allocate_id();
  add_instruction(spv::OpArrayLength, {uint_type, length, _buffer_block_var_id, 1});
  {
    Definition &def = _db.modify_definition(length);
    def._type_id = uint_type;
    def._type = ShaderType::UINT;
  }

  // Bounds check to make sure we don't write outside the SSBO
  uint32_t branch = branch_if(op_compare(spv::OpULessThanEqual, op_add(offset, reservation), length));
    // Store the string identifier (1-based)...
    op_store(op_access_chain(_buffer_block_var_id, {one, offset}), get_uint_constant(string_index + 1));

    // ...followed by the format arguments.
    for (uint32_t i = 0; i < (uint32_t)fmt_args.size(); ++i) {
      uint32_t arg = fmt_args[i];
      if (get_type_id(arg) != uint_type) {
        // Bitcast to uint if it isn't already.
        arg = op_bitcast(ShaderType::UINT, fmt_args[i]);
      }
      op_store(op_access_chain(_buffer_block_var_id, {one, op_add(offset, get_uint_constant(1 + i))}), arg);
    }
    op_branch(branch);
  branch_endif(branch);
}

/**
 *
 */
void SpirVDebugOutputPass::
emit_assert(const std::string &expression) {
  if (_buffer_block_var_id == 0) {
    define_buffer_block();
  }

  // Allocate a unique index for this assert.
  uint32_t assert_id;
  {
    Filename fn;
    if (_current_file == 0) {
      fn = "<unknown>";
    } else {
      fn = resolve_string(_current_file);
    }
    assert_id = (uint32_t)_debug_info.add_assert(expression, std::move(fn), (int)_current_line, _stage);
  }

  // Write asserts to a special 4-word-sized section in the buffer: one per
  // unique assert, first one being a counter.  The remaining 3 fields are
  // used to store the invocation ID of the first occurrence.
  uint32_t zero = get_uint_constant(0);
  uint32_t one = get_uint_constant(1);

  uint32_t index = assert_id == 0 ? zero : get_uint_constant(assert_id * 4);
  uint32_t counter_ptr = op_access_chain(_buffer_block_var_id, {one, index});

  uint32_t uint_type = define_type(ShaderType::UINT);
  uint32_t result = allocate_id();
  add_instruction(spv::OpAtomicIIncrement, {uint_type, result, counter_ptr, one, zero});
  {
    Definition &def = _db.modify_definition(result);
    def._type_id = uint_type;
    def._type = ShaderType::UINT;
  }

  // If we were the first occurrence, write the invocation ID as additional
  // helpful context information.
  uint32_t branch = branch_if(op_compare(spv::OpIEqual, result, zero));
    uint32_t invoc[3] = {0, 0, 0};

    switch (_stage) {
    case Shader::Stage::VERTEX:
      // Vertex ID, instance ID
      invoc[0] = op_bitcast(ShaderType::UINT, op_load(ensure_builtin_input(spv::ExecutionModelVertex, spv::BuiltInVertexId)));
      invoc[1] = op_bitcast(ShaderType::UINT, op_load(ensure_builtin_input(spv::ExecutionModelVertex, spv::BuiltInInstanceId)));
      break;

    case Shader::Stage::GEOMETRY:
      // Primitive ID, invocation ID
      {
        invoc[0] = op_bitcast(ShaderType::UINT, op_load(ensure_builtin_input(spv::ExecutionModelGeometry, spv::BuiltInPrimitiveId)));
        invoc[1] = op_bitcast(ShaderType::UINT, op_load(ensure_builtin_input(spv::ExecutionModelGeometry, spv::BuiltInInvocationId)));
      }
      break;

    case Shader::Stage::TESS_CONTROL:
      // Primitive ID, invocation ID
      {
        invoc[0] = op_bitcast(ShaderType::UINT, op_load(ensure_builtin_input(spv::ExecutionModelTessellationControl, spv::BuiltInPrimitiveId)));
        invoc[1] = op_bitcast(ShaderType::UINT, op_load(ensure_builtin_input(spv::ExecutionModelTessellationControl, spv::BuiltInInvocationId)));
      }
      break;

    case Shader::Stage::TESS_EVALUATION:
      // Primitive ID, Tess coordinate x and y
      {
        uint32_t primitive_id = ensure_builtin_input(spv::ExecutionModelTessellationEvaluation, spv::BuiltInPrimitiveId);
        uint32_t tess_coord_id = ensure_builtin_input(spv::ExecutionModelTessellationEvaluation, spv::BuiltInTessCoord);
        invoc[0] = op_bitcast(ShaderType::UINT, op_load(primitive_id));
        invoc[1] = op_bitcast(ShaderType::UINT, op_load(op_access_chain(tess_coord_id, {zero})));
        invoc[2] = op_bitcast(ShaderType::UINT, op_load(op_access_chain(tess_coord_id, {one})));
      }
      break;

    case Shader::Stage::FRAGMENT:
      // Fragment coordinate
      {
        uint32_t frag_coord_id = ensure_builtin_input(spv::ExecutionModelFragment, spv::BuiltInFragCoord);
        uint32_t two = get_uint_constant(2);
        invoc[0] = op_bitcast(ShaderType::UINT, op_load(op_access_chain(frag_coord_id, {zero})));
        invoc[1] = op_bitcast(ShaderType::UINT, op_load(op_access_chain(frag_coord_id, {one})));
        invoc[2] = op_bitcast(ShaderType::UINT, op_load(op_access_chain(frag_coord_id, {two})));
      }
      break;

    case Shader::Stage::COMPUTE:
      // Global invocation ID
      {
        uint32_t value = op_load(ensure_builtin_input(spv::ExecutionModelGLCompute, spv::BuiltInGlobalInvocationId));
        invoc[0] = op_composite_extract(value, {0});
        invoc[1] = op_composite_extract(value, {1});
        invoc[2] = op_composite_extract(value, {2});
      }
      break;

    default:
      break;
    }

    for (size_t i = 0; i < 3; ++i) {
      if (invoc[i] != 0) {
        op_store(op_access_chain(_buffer_block_var_id, {one, get_uint_constant(assert_id * 4 + i + 1)}), invoc[i]);
      }
    }
    op_branch(branch);
  branch_endif(branch);
}

/**
 * Makes sure the buffer block type is defined.
 */
void SpirVDebugOutputPass::
define_buffer_block() {
  static const ShaderType *block_type = make_buffer_block_type();
  uint32_t block_type_id = define_type(block_type);
  uint32_t block_var_id = define_variable(block_type, spv::StorageClassUniform);

  decorate(block_type_id, spv::DecorationBufferBlock);
  decorate(block_var_id, spv::DecorationBinding, _buffer_binding);
  decorate(block_var_id, spv::DecorationDescriptorSet, _buffer_set);
  decorate(block_var_id, spv::DecorationCoherent);
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
