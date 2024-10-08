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
 *
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
      while (i < op.nargs) {
        if (!is_deleted(op.args[i])) {
          new_args.push_back(op.args[i]);
        }
        ++i;
      }
      add_debug(op.opcode, new_args.data(), new_args.size());
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
 *
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
 *
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
 *
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
            if (transform_function_op(op, function_id)) {
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
 *
 */
void SpirVTransformPass::
preprocess() {
}

/**
 *
 */
ShaderModuleSpirV::InstructionStream SpirVTransformPass::
get_result() const {
  InstructionStream stream(_new_preamble);
  stream._words.insert(stream._words.end(), _new_annotations.begin(), _new_annotations.end());
  stream._words.insert(stream._words.end(), _new_definitions.begin(), _new_definitions.end());
  stream._words.insert(stream._words.end(), _new_functions.begin(), _new_functions.end());
  return stream;
}

/**
 *
 */
bool SpirVTransformPass::
transform_debug_op(Instruction op) {
  if ((op.opcode == spv::OpName || op.opcode == spv::OpMemberName) && op.nargs >= 1 && is_deleted(op.args[0])) {
    return false;
  }
  return true;
}

/**
 *
 */
bool SpirVTransformPass::
transform_annotation_op(Instruction op) {
  if (is_deleted(op.args[0])) {
    return false;
  }
  return true;
}

/**
 *
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
        mark_defined(op.args[0]);
        return false;
      }
    }
    break;
  }
  return true;
}

/**
 *
 */
bool SpirVTransformPass::
begin_function(Instruction op) {
  return true;
}

/**
 *
 */
bool SpirVTransformPass::
transform_function_op(Instruction op, uint32_t function_id) {
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
        mark_defined(new_args[1]);
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
 *
 */
void SpirVTransformPass::
end_function(uint32_t function_id) {
}

/**
 *
 */
void SpirVTransformPass::
postprocess() {
}

/**
 * Writes a name for the given id.
 */
void SpirVTransformPass::
add_name(uint32_t id, const std::string &name) {
  uint32_t nargs = 2 + name.size() / 4;
  uint32_t *args = (uint32_t *)alloca(nargs * 4);
  memset(args, 0, nargs * 4);
  args[0] = id;
  memcpy((char *)(args + 1), name.data(), name.size());
  add_debug(spv::OpName, args, nargs);
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
    spv::Op opcode = (spv::Op)(*it & spv::OpCodeMask);
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
 * struct index numbering).
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
 * Deletes the given parameter of the given function type.
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
 *
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
 * Helper for define_type.  Inserts the given type (after any requisite
 * dependent types, as found through the given type map) at the given iterator,
 * and advances the iterator.
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
    case ShaderType::Access::none:
    case ShaderType::Access::read_only:
      args[8] = spv::AccessQualifierReadOnly;
      ++nargs;
      break;
    case ShaderType::Access::write_only:
      args[8] = spv::AccessQualifierWriteOnly;
      ++nargs;
      break;
    case ShaderType::Access::read_write:
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
 * existing one one is already defined.
 */
uint32_t SpirVTransformPass::
define_int_constant(int32_t constant) {
  uint32_t constant_id = 0;
  uint32_t type_id = 0;

  for (uint32_t id = 0; id < get_id_bound(); ++id) {
    const Definition &def = _db.get_definition(id);
    if (def.is_constant() &&
        def._constant == constant &&
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
 * Defines a new constant.
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

      if (def._array_stride == 0) {
        def._array_stride = array_type->get_stride_bytes();
        add_annotation(spv::OpDecorate,
          {array_type_id, spv::DecorationArrayStride, def._array_stride});
      }
    }

    if (const ShaderType::Matrix *matrix_type = base_type->as_matrix()) {
      // Matrix types need to be explicitly laid out.
      add_annotation(spv::OpMemberDecorate,
        {type_id, i, spv::DecorationMatrixStride, matrix_type->get_num_columns() * 4});
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
 * Adds an instruction to the current function.
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
