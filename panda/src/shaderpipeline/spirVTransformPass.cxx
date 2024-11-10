/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVTransformPass.cxx
 * @author rdb
 * @date 2024-10-08
 */

#include "spirVTransformPass.h"

/**
 *
 */
SpirVTransformPass::
SpirVTransformPass() {
}

/**
 * Processes the header and all instructions, including the debug instructions,
 * up to the first annotation instruction.
 */
void SpirVTransformPass::
process_preamble(std::vector<uint32_t> &stream) {
  nassertv(stream.size() >= 5);

  InstructionIterator it(stream.data() + 5);
  InstructionIterator end(stream.data() + stream.size());

  while (it != end) {
    Instruction op = *it;

    if (op.opcode == spv::OpEntryPoint) {
      // Skip the string literal by skipping words until we have a zero byte.
      pvector<uint32_t> new_args({op.args[0], op.args[1]});
      uint32_t i = 2;
      while (i < op.nargs
          && (op.args[i] & 0x000000ff) != 0
          && (op.args[i] & 0x0000ff00) != 0
          && (op.args[i] & 0x00ff0000) != 0
          && (op.args[i] & 0xff000000) != 0) {
        new_args.push_back(op.args[i]);
        ++i;
      }
      new_args.push_back(op.args[i]);
      ++i;

      // Remove the deleted IDs from the entry point interface.
      uint32_t interface_begin = i;
      while (i < op.nargs) {
        if (!is_deleted(op.args[i])) {
          new_args.push_back(op.args[i]);
        }
        ++i;
      }

      if (transform_entry_point((spv::ExecutionModel)op.args[0], op.args[1], (const char *)&new_args[2], &new_args[interface_begin], new_args.size() - interface_begin)) {
        add_debug(op.opcode, new_args.data(), new_args.size());
      }
    }
    else if (transform_debug_op(op)) {
      _new_preamble.insert(_new_preamble.end(), it._words, it.next()._words);
    }
    ++it;
  }

  // If this triggered, you called add_instruction() outside the function
  // section.
  nassertv(_new_functions.empty());
}

/**
 * Processes the instructions of the annotations section, which contains the
 * decorations.
 */
void SpirVTransformPass::
process_annotations(std::vector<uint32_t> &stream) {
  InstructionIterator it(stream.data());
  InstructionIterator end(stream.data() + stream.size());

  while (it != end) {
    Instruction op = *it;
    nassertv(op.is_annotation());

    if (transform_annotation_op(op)) {
      _new_annotations.insert(_new_annotations.end(), it._words, it.next()._words);
    }
    ++it;
  }

  // If this triggered, you called add_instruction() outside the function
  // section.
  nassertv(_new_functions.empty());
}

/**
 * Processes the instructions of the definitions section, which starts with the
 * first non-annotation instruction and contains all of the types, constants,
 * and global variables.
 */
void SpirVTransformPass::
process_definitions(std::vector<uint32_t> &stream) {
  InstructionIterator it(stream.data());
  InstructionIterator end(stream.data() + stream.size());

  while (it != end) {
    Instruction op = *it;
    nassertv(op.opcode != spv::OpFunction);

    if (op.opcode != spv::OpLine && op.opcode != spv::OpNoLine) {
      // Skip the instruction if it has already been defined, or deleted.
      bool has_result, has_type;
      HasResultAndType(op.opcode, &has_result, &has_type);
      if (!has_result || (!is_defined(op.args[has_type]) && !is_deleted(op.args[has_type])))  {
        if (transform_definition_op(op)) {
          if (has_result) {
            mark_defined(op.args[has_type]);
          }
          _new_definitions.insert(_new_definitions.end(), it._words, it.next()._words);
        }
      }
    }
    ++it;
  }

  // If this triggered, you called add_instruction() outside the function
  // section.
  nassertv(_new_functions.empty());
}

/**
 * Processes the instructions of the function section, which starts with the
 * first OpFunction and contains the remainder of the module.
 */
void SpirVTransformPass::
process_functions(std::vector<uint32_t> &stream) {
  InstructionIterator it(stream.data());
  InstructionIterator end(stream.data() + stream.size());

  while (it != end) {
    Instruction op = *it;
    if (op.opcode == spv::OpFunction) {
      if (begin_function(op)) {
        uint32_t function_id = op.args[1];
        _current_function_id = function_id;
        _new_functions.insert(_new_functions.end(), it._words, it.next()._words);

        ++it;
        while (it != end) {
          Instruction op = *it;
          if (op.opcode == spv::OpFunctionEnd) {
            break;
          }
          bool has_result, has_type;
          HasResultAndType(op.opcode, &has_result, &has_type);
          if (!has_result || (!is_defined(op.args[has_type]) && !is_deleted(op.args[has_type])))  {
            if (transform_function_op(op)) {
              if (has_result) {
                mark_defined(op.args[has_type]);
              }
              _new_functions.insert(_new_functions.end(), it._words, it.next()._words);
            }
          }
          ++it;
        }

        if (it != end) {
          end_function(function_id);
          _current_function_id = 0;
          _new_functions.insert(_new_functions.end(), {spv::OpFunctionEnd | (1 << spv::WordCountShift)});
        } else {
          shader_cat.error()
            << "Encountered end of stream before function end\n";
          return;
        }
      }
    }
    else if (op.opcode != spv::OpLine && op.opcode != spv::OpNoLine) {
      shader_cat.error()
        << "Expected OpFunction instead of " << op.opcode << "\n";
      return;
    }
    ++it;
  }
}

/**
 * Called before any of the instructions are read.  Perform any pre-processing
 * based on the result database and the input arguments here.
 */
void SpirVTransformPass::
preprocess() {
}

/**
 * Transforms an OpEntryPoint.
 * Return true to keep the instruction, false to omit it.
 */
bool SpirVTransformPass::
transform_entry_point(spv::ExecutionModel model, uint32_t id, const char *name, const uint32_t *var_ids, uint16_t num_vars) {
  return true;
}

/**
 * Transforms a debug instruction (OpName or OpMemberName).
 * Return true to preserve the instruction, false to omit it (in which case you
 * may replace it using add_debug).
 */
bool SpirVTransformPass::
transform_debug_op(Instruction op) {
  if ((op.opcode == spv::OpName || op.opcode == spv::OpMemberName) && op.nargs >= 1 && is_deleted(op.args[0])) {
    return false;
  }
  return true;
}

/**
 * Transforms an annotation instruction.
 * Return true to preserve the instruction, false to omit it (in which case you
 * may replace it using add_annotation).
 */
bool SpirVTransformPass::
transform_annotation_op(Instruction op) {
  if (is_deleted(op.args[0])) {
    return false;
  }
  return true;
}

/**
 * Transforms a definition instruction (a type, constant or global variable).
 * Return true to preserve the instruction, false to omit it (in which case you
 * may replace it using add_definition).
 */
bool SpirVTransformPass::
transform_definition_op(Instruction op) {
  switch (op.opcode) {
  case spv::OpTypePointer:
    if (op.nargs >= 3) {
      if (is_deleted(op.args[2])) {
        delete_id(op.args[0]);
        return false;
      }
    }
    break;

  case spv::OpTypeFunction:
    if (op.nargs >= 3) {
      auto it = _deleted_function_parameters.find(op.args[0]);
      if (it != _deleted_function_parameters.end()) {
        pvector<uint32_t> new_args({op.args[0], op.args[1]});
        for (size_t i = 2; i < op.nargs; ++i) {
          if (!it->second.count(i - 2)) {
            if (is_deleted(op.args[i])) {
              delete_function_parameter(op.args[0], i);
            } else {
              new_args.push_back(op.args[i]);
            }
          }
        }
        add_definition(spv::OpTypeFunction, new_args.data(), new_args.size());
        return false;
      }
    }
    break;

  default:
    break;
  }
  return true;
}

/**
 * Called when an OpFunction is encountered.  Return true to preserve the
 * function, false to skip all instructions up to the next OpFunctionEnd (in
 * which case end_function() will not be called either).
 * It is permitted to modify the arguments of the given op.
 */
bool SpirVTransformPass::
begin_function(Instruction op) {
  return true;
}

/**
 * Transforms an instruction encountered inside a function.  This will always
 * be called between begin_function() and end_function() and will be passed the
 * result identifier of the previous OpFunction.
 */
bool SpirVTransformPass::
transform_function_op(Instruction op) {
  switch (op.opcode) {
  case spv::OpLoad:
  case spv::OpAtomicLoad:
  case spv::OpAtomicExchange:
  case spv::OpAtomicCompareExchange:
  case spv::OpAtomicCompareExchangeWeak:
  case spv::OpAtomicIIncrement:
  case spv::OpAtomicIDecrement:
  case spv::OpAtomicIAdd:
  case spv::OpAtomicISub:
  case spv::OpAtomicSMin:
  case spv::OpAtomicUMin:
  case spv::OpAtomicSMax:
  case spv::OpAtomicUMax:
  case spv::OpAtomicAnd:
  case spv::OpAtomicOr:
  case spv::OpAtomicXor:
  case spv::OpAtomicFlagTestAndSet:
  case spv::OpAtomicFMinEXT:
  case spv::OpAtomicFMaxEXT:
  case spv::OpAtomicFAddEXT:
    nassertr(!is_deleted(op.args[2]), true);
    break;

  case spv::OpStore:
  case spv::OpAtomicStore:
  case spv::OpAtomicFlagClear:
    nassertr(!is_deleted(op.args[0]), true);
    break;

  case spv::OpCopyMemory:
  case spv::OpCopyMemorySized:
    nassertr(!is_deleted(op.args[0]), true);
    nassertr(!is_deleted(op.args[1]), true);
    break;

  case spv::OpImageTexelPointer:
  case spv::OpAccessChain:
  case spv::OpInBoundsAccessChain:
  case spv::OpPtrAccessChain:
  case spv::OpInBoundsPtrAccessChain:
  case spv::OpCopyObject:
  case spv::OpExpectKHR:
  case spv::OpBitcast:
  case spv::OpCopyLogical:
    // Delete these uses of a deleted variable, presumably the result is also
    // not being used.
    if (is_deleted(op.args[2])) {
      delete_id(op.args[1]);
      return false;
    }
    break;

  case spv::OpPtrEqual:
  case spv::OpPtrNotEqual:
  case spv::OpPtrDiff:
    nassertr(!is_deleted(op.args[2]), true);
    nassertr(!is_deleted(op.args[3]), true);
    break;

  case spv::OpFunctionCall:
    if (op.nargs >= 4) {
      uint32_t func_type_id = get_type_id(op.args[2]);

      auto it = _deleted_function_parameters.find(func_type_id);
      if (it != _deleted_function_parameters.end()) {
        pvector<uint32_t> new_args({op.args[0], op.args[1], op.args[2]});
        for (size_t i = 3; i < op.nargs; ++i) {
          if (!it->second.count(i - 3)) {
            new_args.push_back(op.args[i]);
          }
        }
        add_instruction(spv::OpFunctionCall, new_args.data(), new_args.size());
        return false;
      }
    }
    break;

  case spv::OpReturnValue:
    nassertr(!is_deleted(op.args[0]), true);
    break;

  default:
    break;
  };

  return true;
}

/**
 * Called when an OpFunctionEnd instruction is encountered, belonging to an
 * OpFunction with the given identifier.
 */
void SpirVTransformPass::
end_function(uint32_t function_id) {
}

/**
 * Called after all instructions have been read, this does any post-processing
 * needed (such as updating the result database to reflect the transformations,
 * adding names/decorations, etc.)
 */
void SpirVTransformPass::
postprocess() {
}

/**
 * Writes a name for the given id.
 */
void SpirVTransformPass::
set_name(uint32_t id, const std::string &name) {
  Definition &def = _db.modify_definition(id);
  if (!def._name.empty()) {
    // Remove the existing name.
    auto it = _new_preamble.begin() + 5;
    while (it != _new_preamble.end()) {
      spv::Op opcode = (spv::Op)(*it & spv::OpCodeMask);
      uint32_t wcount = *it >> spv::WordCountShift;
      nassertd(wcount > 0) break;

      if (wcount >= 2 && opcode == spv::OpName && *(it + 1) == id) {
        it = _new_preamble.erase(it, it + wcount);
        continue;
      }

      std::advance(it, wcount);
    }
  }
  def._name = name;

  uint32_t nargs = 2 + name.size() / 4;
  uint32_t *args = (uint32_t *)alloca(nargs * 4);
  memset(args, 0, nargs * 4);
  args[0] = id;
  memcpy((char *)(args + 1), name.data(), name.size());
  add_debug(spv::OpName, args, nargs);
}

/**
 * Writes a name for the given struct member.
 */
void SpirVTransformPass::
set_member_name(uint32_t type_id, uint32_t member_index, const std::string &name) {
  MemberDefinition &mdef = _db.modify_definition(type_id).modify_member(member_index);
  if (!mdef._name.empty()) {
    // Remove the existing name.
    auto it = _new_preamble.begin();
    while (it != _new_preamble.end()) {
      spv::Op opcode = (spv::Op)(*it & spv::OpCodeMask);
      uint32_t wcount = *it >> spv::WordCountShift;
      nassertd(wcount > 0) break;

      if (wcount >= 3 && opcode == spv::OpMemberName && *(it + 1) == type_id && *(it + 2) == member_index) {
        it = _new_preamble.erase(it, it + wcount);
        continue;
      }

      std::advance(it, wcount);
    }
  }
  mdef._name = name;

  uint32_t nargs = 3 + name.size() / 4;
  uint32_t *args = (uint32_t *)alloca(nargs * 4);
  memset(args, 0, nargs * 4);
  args[0] = type_id;
  args[1] = member_index;
  memcpy((char *)(args + 2), name.data(), name.size());
  add_debug(spv::OpMemberName, args, nargs);
}

/**
 * Deletes the given identifier, and any annotations for it.
 */
void SpirVTransformPass::
delete_id(uint32_t id) {
  _deleted_ids.insert(id);

  // Since the annotations and debug names are defined before the actual
  // definition, we go back here and remove these.
  if (_new_preamble.size() > 5) {
    auto it = _new_preamble.begin() + 5;
    while (it != _new_preamble.end()) {
      spv::Op opcode = (spv::Op)(*it & spv::OpCodeMask);
      uint32_t wcount = *it >> spv::WordCountShift;
      nassertd(wcount > 0) break;

      if ((opcode == spv::OpName || opcode == spv::OpMemberName) && wcount >= 2 && *(it + 1) == id) {
        it = _new_preamble.erase(it, it + wcount);
        continue;
      }
      else if (opcode == spv::OpEntryPoint) {
        // Skip the string literal by skipping words until we have a zero byte.
        uint32_t i = 3;
        while (i < wcount
            && (*(it + i) & 0x000000ff) != 0
            && (*(it + i) & 0x0000ff00) != 0
            && (*(it + i) & 0x00ff0000) != 0
            && (*(it + i) & 0xff000000) != 0) {
          ++i;
        }
        ++i;

        // Remove the deleted IDs from the entry point interface.
        while (i < wcount) {
          if (is_deleted(*(it + i))) {
            it = _new_preamble.erase(it + i, it + i + 1);
            --wcount;
          }
          ++i;
        }

        *it = opcode | (wcount << spv::WordCountShift);
      }

      std::advance(it, wcount);
    }
  }

  auto it = _new_annotations.begin();
  while (it != _new_annotations.end()) {
    //spv::Op opcode = (spv::Op)(*it & spv::OpCodeMask);
    uint32_t wcount = *it >> spv::WordCountShift;
    nassertd(wcount > 0) break;

    if (wcount >= 2 && *(it + 1) == id) {
      it = _new_annotations.erase(it, it + wcount);
      continue;
    }

    std::advance(it, wcount);
  }
}

/**
 * Deletes the annotations for the given struct member (using the pre-transform
 * struct index numbering).  Does not update the actual OpTypeStruct args, any
 * access chains, etc.
 */
void SpirVTransformPass::
delete_struct_member(uint32_t id, uint32_t member_index) {
  Definition &struct_def = _db.modify_definition(id);
  nassertv(member_index < struct_def._members.size());

  if (!_deleted_members[id].insert(member_index).second) {
    // Was already deleted.
    return;
  }

  MemberDefinition &member_def = struct_def._members[member_index];

  uint32_t current_index = member_def._new_index;

  for (size_t i = member_index + 1; i < struct_def._members.size(); ++i) {
    struct_def._members[i]._new_index--;
  }

  {
    auto it = _new_preamble.begin() + 5;
    while (it != _new_preamble.end()) {
      spv::Op opcode = (spv::Op)(*it & spv::OpCodeMask);
      uint32_t wcount = *it >> spv::WordCountShift;
      nassertd(wcount > 0) break;

      if (opcode == spv::OpMemberName && wcount >= 3 && *(it + 1) == id) {
        if (*(it + 2) == current_index) {
          it = _new_preamble.erase(it, it + wcount);
          continue;
        }
        if (*(it + 2) > current_index) {
          --(*(it + 2));
        }
      }

      std::advance(it, wcount);
    }
  }
  {
    auto it = _new_annotations.begin();
    while (it != _new_annotations.end()) {
      spv::Op opcode = (spv::Op)(*it & spv::OpCodeMask);
      uint32_t wcount = *it >> spv::WordCountShift;
      nassertd(wcount > 0) break;

      if (wcount >= 3 && (opcode == spv::OpMemberDecorate || opcode == spv::OpMemberDecorateString) && *(it + 1) == id) {
        if (*(it + 2) == current_index) {
          it = _new_annotations.erase(it, it + wcount);
          continue;
        }
        if (*(it + 2) > current_index) {
          --(*(it + 2));
        }
      }

      std::advance(it, wcount);
    }
  }
}

/**
 * Deletes the given parameter of the given function type.  Should be called
 * before the OpTypeFunction is encountered, or the OpTypeFunction should have
 * already been modified to remove this parameter.
 */
void SpirVTransformPass::
delete_function_parameter(uint32_t type_id, uint32_t param_index) {
  if (!_deleted_function_parameters[type_id].insert(param_index).second) {
    // Was already deleted.
    return;
  }

  for (size_t id = 0; id < get_id_bound(); ++id) {
    const Definition &def = _db.get_definition(id);
    if (def._type_id == type_id) {
      uint32_t param_id = def._parameters[param_index];
      delete_id(param_id);
    }
  }
}

/**
 * Adds a new variable definition to the definitions section.  Inserts any type
 * declarations and annotations that may be necessary.
 */
uint32_t SpirVTransformPass::
define_variable(const ShaderType *type, spv::StorageClass storage_class) {
  uint32_t pointer_type_id = define_pointer_type(type, storage_class);

  uint32_t variable_id = allocate_id();
  add_definition(spv::OpVariable, {
    pointer_type_id,
    variable_id,
    (uint32_t)storage_class,
  });

  _db.record_variable(variable_id, pointer_type_id, storage_class);

  // Depending on the storage class, we may need to make sure it is laid out.
  if (storage_class == spv::StorageClassStorageBuffer ||
      storage_class == spv::StorageClassPhysicalStorageBuffer ||
      storage_class == spv::StorageClassUniform ||
      storage_class == spv::StorageClassPushConstant) {
    r_annotate_struct_layout(unwrap_pointer_type(pointer_type_id));
  }

  return variable_id;
}

/**
 *
 */
uint32_t SpirVTransformPass::
define_pointer_type(const ShaderType *type, spv::StorageClass storage_class) {
  uint32_t pointer_type_id = _db.find_pointer_type(type, storage_class);
  if (pointer_type_id != 0 && is_defined(pointer_type_id)) {
    return pointer_type_id;
  }

  uint32_t type_id = define_type(type);
  if (pointer_type_id == 0) {
    pointer_type_id = allocate_id();
    _db.record_pointer_type(pointer_type_id, storage_class, type_id);
  }

  add_definition(spv::OpTypePointer,
    {pointer_type_id, (uint32_t)storage_class, type_id});

  return pointer_type_id;
}

/**
 * Ensures that the given type is defined by adding instructions to the
 * definitions section as necessary.
 */
uint32_t SpirVTransformPass::
define_type(const ShaderType *type) {
  uint32_t id = _db.find_type(type);
  if (id != 0 && is_defined(id)) {
    return id;
  }

  if (id == 0) {
    id = allocate_id();
    _db.record_type(id, type);
  }

  if (const ShaderType::Scalar *scalar_type = type->as_scalar()) {
    switch (scalar_type->get_scalar_type()) {
    case ShaderType::ST_float:
      add_definition(spv::OpTypeFloat, {id, 32});
      break;
    case ShaderType::ST_double:
      add_definition(spv::OpTypeFloat, {id, 64});
      break;
    case ShaderType::ST_int:
      add_definition(spv::OpTypeInt, {id, 32, 1});
      break;
    case ShaderType::ST_uint:
      add_definition(spv::OpTypeInt, {id, 32, 0});
      break;
    case ShaderType::ST_bool:
      add_definition(spv::OpTypeBool, {id});
      break;
    default:
      add_definition(spv::OpTypeVoid, {id});
      break;
    }
  }
  else if (const ShaderType::Vector *vector_type = type->as_vector()) {
    uint32_t component_type = define_type(
      ShaderType::register_type(ShaderType::Scalar(vector_type->get_scalar_type())));

    add_definition(spv::OpTypeVector,
      {id, component_type, vector_type->get_num_components()});
  }
  else if (const ShaderType::Matrix *matrix_type = type->as_matrix()) {
    uint32_t row_type = define_type(
      ShaderType::register_type(ShaderType::Vector(matrix_type->get_scalar_type(), matrix_type->get_num_columns())));

    add_definition(spv::OpTypeMatrix,
      {id, row_type, matrix_type->get_num_rows()});
  }
  else if (const ShaderType::Struct *struct_type = type->as_struct()) {
    size_t num_members = struct_type->get_num_members();
    uint32_t *args = (uint32_t *)alloca((1 + num_members) * sizeof(uint32_t));
    args[0] = id;
    uint32_t *member_types = args + 1;

    Definition &def = _db.modify_definition(id);

    for (size_t i = 0; i < num_members; ++i) {
      const ShaderType::Struct::Member &member = struct_type->get_member(i);

      member_types[i] = define_type(member.type);
      MemberDefinition &member_def = def.modify_member(i);
      member_def._new_index = i;
      member_def._type_id = member_types[i];
    }

    add_definition(spv::OpTypeStruct, args, num_members + 1);
  }
  else if (const ShaderType::Array *array_type = type->as_array()) {
    uint32_t element_type = define_type(array_type->get_element_type());

    auto size = array_type->get_num_elements();
    if (size != 0) {
      uint32_t constant_id = define_int_constant(size);

      add_definition(spv::OpTypeArray,
        {id, element_type, constant_id});
      _db.modify_definition(id)._type_id = element_type;
    } else {
      add_definition(spv::OpTypeRuntimeArray,
        {id, element_type});
    }
  }
  else if (const ShaderType::Image *image_type = type->as_image()) {
    uint32_t args[9] = {
      id,
      define_type(ShaderType::register_type(ShaderType::Scalar(image_type->get_sampled_type()))),
      0, // Dimensionality, see below
      2, // Unspecified depthness
      0, // Arrayness, see below
      0, // Multisample not supported
      0, // Sampled (unknown)
      spv::ImageFormatUnknown,
      0, // Access qualifier
    };

    switch (image_type->get_texture_type()) {
    case Texture::TT_1d_texture:
      args[2] = spv::Dim1D;
      args[4] = 0;
      break;
    case Texture::TT_2d_texture:
      args[2] = spv::Dim2D;
      args[4] = 0;
      break;
    case Texture::TT_3d_texture:
      args[2] = spv::Dim3D;
      args[4] = 0;
      break;
    case Texture::TT_2d_texture_array:
      args[2] = spv::Dim2D;
      args[4] = 1;
      break;
    case Texture::TT_cube_map:
      args[2] = spv::DimCube;
      args[4] = 0;
      break;
    case Texture::TT_buffer_texture:
      args[2] = spv::DimBuffer;
      args[4] = 0;
      break;
    case Texture::TT_cube_map_array:
      args[2] = spv::DimCube;
      args[4] = 1;
      break;
    case Texture::TT_1d_texture_array:
      args[2] = spv::Dim1D;
      args[4] = 1;
      break;
    }

    uint32_t nargs = 8;
    switch (image_type->get_access()) {
    case ShaderType::Access::NONE:
    case ShaderType::Access::READ_ONLY:
      args[8] = spv::AccessQualifierReadOnly;
      ++nargs;
      break;
    case ShaderType::Access::WRITE_ONLY:
      args[8] = spv::AccessQualifierWriteOnly;
      ++nargs;
      break;
    case ShaderType::Access::READ_WRITE:
      args[8] = spv::AccessQualifierReadWrite;
      ++nargs;
      break;
    }

    add_definition(spv::OpTypeImage, args, nargs);
  }
  else if (type->as_sampler() != nullptr) {
    add_definition(spv::OpTypeSampler, {id});
  }
  else if (const ShaderType::SampledImage *sampled_image_type = type->as_sampled_image()) {
    // We insert the image type here as well, because there are some specifics
    // about the image definition that we need to get right.
    uint32_t image_id = allocate_id();
    uint32_t args[8] = {
      image_id,
      define_type(ShaderType::register_type(ShaderType::Scalar(sampled_image_type->get_sampled_type()))),
      0, // Dimensionality, see below
      sampled_image_type->is_shadow() ? (uint32_t)1 : (uint32_t)0, // Depthness
      0, // Arrayness, see below
      0, // Multisample not supported
      1, // Sampled
      spv::ImageFormatUnknown,
    };

    switch (sampled_image_type->get_texture_type()) {
    case Texture::TT_1d_texture:
      args[2] = spv::Dim1D;
      args[4] = 0;
      break;
    case Texture::TT_2d_texture:
      args[2] = spv::Dim2D;
      args[4] = 0;
      break;
    case Texture::TT_3d_texture:
      args[2] = spv::Dim3D;
      args[4] = 0;
      break;
    case Texture::TT_2d_texture_array:
      args[2] = spv::Dim2D;
      args[4] = 1;
      break;
    case Texture::TT_cube_map:
      args[2] = spv::DimCube;
      args[4] = 0;
      break;
    case Texture::TT_buffer_texture:
      args[2] = spv::DimBuffer;
      args[4] = 0;
      break;
    case Texture::TT_cube_map_array:
      args[2] = spv::DimCube;
      args[4] = 1;
      break;
    case Texture::TT_1d_texture_array:
      args[2] = spv::Dim1D;
      args[4] = 1;
      break;
    }

    add_definition(spv::OpTypeImage, args, 8);
    add_definition(spv::OpTypeSampledImage, {id, image_id});
  }
  else {
    add_definition(spv::OpTypeVoid, {id});
  }

  return id;
}

/**
 * Defines a new integral constant, either of type uint or int, reusing an
 * existing one if one is already defined (except for OpConstantNull, which
 * can't be used to index structure members).
 */
uint32_t SpirVTransformPass::
define_int_constant(int32_t constant) {
  uint32_t constant_id = 0;
  uint32_t type_id = 0;

  for (uint32_t id = 0; id < get_id_bound(); ++id) {
    const Definition &def = _db.get_definition(id);
    if (def.is_constant() && !def.is_null_constant() &&
        def._constant == (uint32_t)constant &&
        (def._type == ShaderType::int_type || (constant >= 0 && def._type == ShaderType::uint_type))) {
      if (is_defined(id)) {
        return id;
      }
      constant_id = id;
      type_id = def._type_id;
    }
  }

  if (constant_id == 0) {
    type_id = define_type(ShaderType::int_type);
    constant_id = allocate_id();
  }

  add_definition(spv::OpConstant, {type_id, constant_id, (uint32_t)constant});

  _db.record_constant(constant_id, type_id, (uint32_t *)&constant, 1);
  return constant_id;
}

/**
 * Defines a new null constant of the given type, reusing an existing one if
 * one is already defined.
 */
uint32_t SpirVTransformPass::
define_null_constant(const ShaderType *type) {
  uint32_t constant_id = 0;
  uint32_t type_id = define_type(type);

  for (uint32_t id = 0; id < get_id_bound(); ++id) {
    const Definition &def = _db.get_definition(id);
    if (def.is_null_constant() && def._type_id == type_id) {
      if (is_defined(id)) {
        return id;
      }
      constant_id = id;
    }
  }

  if (constant_id == 0) {
    constant_id = allocate_id();
  }

  add_definition(spv::OpConstantNull, {type_id, constant_id});

  _db.record_constant(constant_id, type_id, nullptr, 0);
  return constant_id;
}

/**
 * Defines a new constant.  Does not attempt to reuse constants.
 */
uint32_t SpirVTransformPass::
define_constant(const ShaderType *type, uint32_t constant) {
  uint32_t type_id = define_type(type);

  uint32_t constant_id = allocate_id();
  add_definition(spv::OpConstant, {type_id, constant_id, constant});

  _db.record_constant(constant_id, type_id, &constant, 1);
  return constant_id;
}

/**
 * Makes sure that the given type has all its structure members correctly laid
 * out using offsets and strides.
 */
void SpirVTransformPass::
r_annotate_struct_layout(uint32_t type_id) {
  Definition &def = _db.modify_definition(type_id);

  const ShaderType *type = def._type;
  nassertv(type != nullptr);

  const ShaderType::Struct *struct_type = type->as_struct();
  if (struct_type == nullptr) {
    // If this is an array of structs, recurse.
    if (const ShaderType::Array *array_type = type->as_array()) {
      // Also make sure there's an ArrayStride decoration for this array.
      if (def._array_stride == 0) {
        def._array_stride = array_type->get_stride_bytes();
        add_annotation(spv::OpDecorate,
          {type_id, spv::DecorationArrayStride, def._array_stride});
      }

      uint32_t element_type_id = _db.find_type(array_type->get_element_type());
      r_annotate_struct_layout(element_type_id);
    }
    return;
  }

  uint32_t num_members = struct_type->get_num_members();

  for (uint32_t i = 0; i < num_members; ++i) {
    const ShaderType::Struct::Member &member = struct_type->get_member(i);

    MemberDefinition &member_def = def.modify_member(i);
    if (member_def._offset < 0) {
      member_def._offset = member.offset;

      add_annotation(spv::OpMemberDecorate,
        {type_id, i, spv::DecorationOffset, member.offset});
    }

    // Unwrap array to see if there's a matrix here.
    const ShaderType *base_type = member.type;
    while (const ShaderType::Array *array_type = base_type->as_array()) {
      base_type = array_type->get_element_type();

      // Also make sure there's an ArrayStride decoration for this array.
      uint32_t array_type_id = _db.find_type(array_type);

      Definition &array_def = _db.modify_definition(array_type_id);
      if (array_def._array_stride == 0) {
        array_def._array_stride = array_type->get_stride_bytes();
        add_annotation(spv::OpDecorate,
          {array_type_id, spv::DecorationArrayStride, array_def._array_stride});
      }
    }

    if (const ShaderType::Matrix *matrix_type = base_type->as_matrix()) {
      // Matrix types need to be explicitly laid out.
      uint32_t stride = ShaderType::get_scalar_size_bytes(matrix_type->get_scalar_type()) * 4;
      add_annotation(spv::OpMemberDecorate,
        {type_id, i, spv::DecorationMatrixStride, stride});
      add_annotation(spv::OpMemberDecorate,
        {type_id, i, spv::DecorationColMajor});
    } else {
      r_annotate_struct_layout(member_def._type_id);
    }
  }
}

/**
 * Adds an instruction to the end of the new definitions section.
 */
void SpirVTransformPass::
add_definition(spv::Op opcode, const uint32_t *args, uint16_t nargs) {
  bool has_result, has_type;
  HasResultAndType(opcode, &has_result, &has_type);
  nassertv(nargs >= has_result + has_type)
  if (has_result) {
    mark_defined(args[has_type]);
  }
  _new_definitions.push_back(((nargs + 1) << spv::WordCountShift) | opcode);
  _new_definitions.insert(_new_definitions.end(), args, args + nargs);
}

/**
 * Adds an instruction to the current function.  May only be called from
 * transform_function_op.
 */
void SpirVTransformPass::
add_instruction(spv::Op opcode, const uint32_t *args, uint16_t nargs) {
  bool has_result, has_type;
  HasResultAndType(opcode, &has_result, &has_type);
  nassertv(nargs >= has_result + has_type)
  if (has_result) {
    mark_defined(args[has_type]);
  }
  _new_functions.push_back(((nargs + 1) << spv::WordCountShift) | opcode);
  _new_functions.insert(_new_functions.end(), args, args + nargs);
}

/**
 * Inserts an OpLoad from the given pointer id.
 */
uint32_t SpirVTransformPass::
op_load(uint32_t var_id, spv::MemoryAccessMask access) {
  const Definition &var_def = _db.get_definition(var_id);
  uint32_t type_id = unwrap_pointer_type(var_def._type_id);

  uint32_t id = allocate_id();
  if (access != spv::MemoryAccessMaskNone) {
    _new_functions.insert(_new_functions.end(),
      {(5 << spv::WordCountShift) | spv::OpLoad, type_id, id, var_id, (uint32_t)access});
  } else {
    _new_functions.insert(_new_functions.end(),
      {(4 << spv::WordCountShift) | spv::OpLoad, type_id, id, var_id});
  }

  _db.record_temporary(id, type_id, var_id, _current_function_id);

  // A load from the pointer is enough for us to consider it "used", for now.
  mark_used(id);
  mark_defined(id);
  return id;
}

/**
 * Inserts an OpSelect.
 */
uint32_t SpirVTransformPass::
op_select(uint32_t cond, uint32_t obj1, uint32_t obj2) {
  const Definition &obj1_def = _db.get_definition(obj1);
  const Definition &obj2_def = _db.get_definition(obj2);
  nassertr(obj1_def._type_id == obj2_def._type_id, 0);

  uint32_t id = allocate_id();
  _new_functions.insert(_new_functions.end(), {(6 << spv::WordCountShift) | spv::OpSelect, obj1_def._type_id, id, cond, obj1, obj2});

  mark_used(obj1);
  mark_used(obj2);
  mark_defined(id);
  return id;
}

/**
 * Inserts an OpAccessChain with the given base id (which must be a pointer)
 * and constant ids containing the various member/array indices.
 */
uint32_t SpirVTransformPass::
op_access_chain(uint32_t var_id, std::initializer_list<uint32_t> chain) {
  const Definition &var_def = _db.get_definition(var_id);
  const Definition &var_type_def = _db.get_definition(var_def._type_id);
  nassertr(var_type_def.is_pointer_type(), 0);
  spv::StorageClass storage_class = var_type_def._storage_class;

  uint32_t type_id = var_type_def._type_id;
  for (auto index_id : chain) {
    const Definition &type_def = _db.get_definition(type_id);
    nassertr(type_def.is_type(), 0);

    if (!type_def._members.empty()) {
      uint32_t member_index = resolve_constant(index_id);
      nassertr((size_t)member_index < type_def._members.size(), 0);
      type_id = type_def._members[member_index]._type_id;
    } else {
      // Array, matrix, or vector
      type_id = type_def._type_id;
    }
    nassertr(type_id != 0, 0);
  }

  uint32_t pointer_type_id = _db.find_pointer_type(type_id, storage_class);
  if (pointer_type_id == 0) {
    pointer_type_id = allocate_id();
    _db.record_pointer_type(pointer_type_id, storage_class, type_id);

    add_definition(spv::OpTypePointer,
      {pointer_type_id, (uint32_t)storage_class, type_id});
  }

  uint32_t id = allocate_id();
  _new_functions.insert(_new_functions.end(), {((4 + (uint32_t)chain.size()) << spv::WordCountShift) | spv::OpAccessChain, pointer_type_id, id, var_id});
  _new_functions.insert(_new_functions.end(), chain);

  _db.record_temporary(id, pointer_type_id, var_id, _current_function_id);
  return id;
}

/**
 * Inserts an OpVectorShuffle, like a swizzle but may source from two vectors
 * at once, with the indices continuing to number into the second vector.
 * For a regular swizzle, pass the same vector twice.
 */
uint32_t SpirVTransformPass::
op_vector_shuffle(uint32_t vec1, uint32_t vec2, const pvector<uint32_t> &components) {
  const ShaderType::Vector *vec1_type = resolve_type(get_type_id(vec1))->as_vector();
  const ShaderType::Vector *vec2_type = resolve_type(get_type_id(vec2))->as_vector();
  nassertr(vec1_type != nullptr && vec2_type != nullptr, 0);
  nassertr(vec1_type->get_scalar_type() == vec2_type->get_scalar_type(), 0);

  const ShaderType *result_type = ShaderType::register_type(ShaderType::Vector(vec1_type->get_scalar_type(), components.size()));
  uint32_t type_id = define_type(result_type);

  uint32_t id = allocate_id();
  _new_functions.insert(_new_functions.end(), {((5 + (uint32_t)components.size()) << spv::WordCountShift) | spv::OpVectorShuffle, type_id, id, vec1, vec2});
  _new_functions.insert(_new_functions.end(), components.begin(), components.end());

  Definition &def = _db.modify_definition(id);
  def._type_id = type_id;
  def._type = result_type;

  mark_defined(id);
  return id;
}

/**
 * Constructs a composite with the given type from the given constituents.
 */
uint32_t SpirVTransformPass::
op_composite_construct(const ShaderType *type, const pvector<uint32_t> &constituents) {
  uint32_t type_id = define_type(type);

  uint32_t id = allocate_id();
  _new_functions.insert(_new_functions.end(), {((3 + (uint32_t)constituents.size()) << spv::WordCountShift) | spv::OpCompositeConstruct, type_id, id});
  _new_functions.insert(_new_functions.end(), constituents.begin(), constituents.end());

  Definition &def = _db.modify_definition(id);
  def._type_id = type_id;
  def._type = type;

  mark_defined(id);
  return id;
}

/**
 * Inserts an OpCompositeExtract.
 */
uint32_t SpirVTransformPass::
op_composite_extract(uint32_t obj_id, std::initializer_list<uint32_t> chain) {
  const Definition &obj_def = _db.get_definition(obj_id);

  uint32_t type_id = obj_def._type_id;
  for (auto index : chain) {
    const Definition &type_def = _db.get_definition(type_id);
    nassertr(type_def.is_type() && !type_def.is_pointer_type(), 0);

    if (!type_def._members.empty()) {
      nassertr((size_t)index < type_def._members.size(), 0);
      type_id = type_def._members[index]._type_id;
    } else {
      // Array, matrix, or vector
      type_id = type_def._type_id;
    }
    nassertr(type_id != 0, 0);
  }

  uint32_t id = allocate_id();
  _new_functions.insert(_new_functions.end(), {((4 + (uint32_t)chain.size()) << spv::WordCountShift) | spv::OpCompositeExtract, type_id, id, obj_id});
  _new_functions.insert(_new_functions.end(), chain);

  Definition &def = _db.modify_definition(id);
  def._type_id = type_id;
  def._type = resolve_type(type_id);

  mark_defined(id);
  return id;
}

/**
 * Inserts a comparison op, taking two operands and returning a bool.
 * At the moment, only works on scalars.
 */
uint32_t SpirVTransformPass::
op_compare(spv::Op opcode, uint32_t obj1, uint32_t obj2) {
  uint32_t type_id = define_type(ShaderType::bool_type);

  uint32_t id = allocate_id();
  _new_functions.insert(_new_functions.end(), {(5u << spv::WordCountShift) | opcode, type_id, id, obj1, obj2});

  Definition &def = _db.modify_definition(id);
  def._type_id = type_id;
  def._type = ShaderType::bool_type;

  mark_defined(id);
  return id;
}

/**
 * Inserts an OpKill.
 */
void SpirVTransformPass::
op_kill() {
  _new_functions.insert(_new_functions.end(), {(1u << spv::WordCountShift) | spv::OpKill});
}

/**
 * Begins an "if" branch.
 * The return value should be passed to branch_endif().
 */
uint32_t SpirVTransformPass::
branch_if(uint32_t cond) {
  uint32_t true_label = allocate_id();
  uint32_t false_label = allocate_id();

  _new_functions.insert(_new_functions.end(), {
    (3 << spv::WordCountShift) | spv::OpSelectionMerge, false_label, (uint32_t)spv::SelectionControlMaskNone,
    (4 << spv::WordCountShift) | spv::OpBranchConditional, cond, true_label, false_label,
    (2 << spv::WordCountShift) | spv::OpLabel, true_label});
  return false_label;
}

/**
 * Ends an "if" branch.
 */
void SpirVTransformPass::
branch_endif(uint32_t false_label) {
  _new_functions.insert(_new_functions.end(), {(2u << spv::WordCountShift) | spv::OpLabel, false_label});
}
