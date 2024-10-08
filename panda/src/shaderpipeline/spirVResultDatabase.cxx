/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVResultDatabase.cxx
 * @author rdb
 * @date 2024-10-08
 */

#include "spirVResultDatabase.h"

#include "GLSL.std.450.h"

/**
 * Returns true if this type contains anything decorated with BuiltIn.
 */
bool SpirVResultDatabase::Definition::
has_builtin() const {
  if (_builtin != spv::BuiltInMax) {
    return true;
  }
  for (const MemberDefinition &def : _members) {
    if (def._builtin != spv::BuiltInMax) {
      return true;
    }
  }
  return false;
}

/**
 * Returns a MemberDefinition for the given member.
 */
const SpirVResultDatabase::MemberDefinition &SpirVResultDatabase::Definition::
get_member(uint32_t i) const {
  static MemberDefinition default_def;
  if (i >= _members.size()) {
    return default_def;
  }
  return _members[i];
}

/**
 * Returns a modifiable MemberDefinition for the given member.
 */
SpirVResultDatabase::MemberDefinition &SpirVResultDatabase::Definition::
modify_member(uint32_t i) {
  size_t old_size = _members.size();
  if (i >= old_size) {
    _members.resize(i + 1);
    for (size_t j = old_size; j < _members.size(); ++j) {
      _members[j]._new_index = j;
    }
  }
  return _members[i];
}

/**
 * Clears this definition, in case it has just been removed.
 */
void SpirVResultDatabase::Definition::
clear() {
  _dtype = DT_none;
  _name.clear();
  _type = nullptr;
  _location = -1;
  _builtin = spv::BuiltInMax;
  _constant = 0;
  _type_id = 0;
  _array_stride = 0;
  _origin_id = 0;
  _function_id = 0;
  _members.clear();
  _flags = 0;
}

/**
 * Finds the definition with the given name.
 */
uint32_t SpirVResultDatabase::
find_definition(const std::string &name) const {
  for (uint32_t id = 0; id < _defs.size(); ++id) {
    if (_defs[id]._name == name) {
      return id;
    }
  }

  return 0;
}

/**
 * Returns the definition by its identifier.
 */
const SpirVResultDatabase::Definition &SpirVResultDatabase::
get_definition(uint32_t id) const {
  if (id >= _defs.size()) {
    static Definition default_def;
    return default_def;
  }
  return _defs[id];
}

/**
 * Returns a mutable definition by its identifier.  May invalidate existing
 * definition references.
 */
SpirVResultDatabase::Definition &SpirVResultDatabase::
modify_definition(uint32_t id) {
  if (id >= _defs.size()) {
    _defs.resize(id + 1);
  }
  return _defs[id];
}

/**
 * Parses the instruction with the given SPIR-V opcode and arguments.  Any
 * encountered definitions are recorded in the definitions vector.
 */
void SpirVResultDatabase::
parse_instruction(const Instruction &op, uint32_t &current_function_id) {
  switch (op.opcode) {
  case spv::OpExtInstImport:
    record_ext_inst_import(op.args[0], (const char*)&op.args[1]);
    break;

  case spv::OpExtInst:
    {
      const Definition &def = get_definition(op.args[2]);
      nassertv(def.is_ext_inst());
      if (def._name == "GLSL.std.450") {
        // These standard functions take pointers as arguments.
        switch (op.args[3]) {
        case GLSLstd450Modf:
        case GLSLstd450Frexp:
          mark_used(op.args[5]);
          break;

        case GLSLstd450InterpolateAtCentroid:
          mark_used(op.args[4]);
          break;

        case GLSLstd450InterpolateAtSample:
        case GLSLstd450InterpolateAtOffset:
          mark_used(op.args[4]);
          mark_used(op.args[5]);
          break;
        }
      }
    }
    break;

  case spv::OpName:
    _defs[op.args[0]]._name.assign((const char *)&op.args[1]);
    break;

  case spv::OpMemberName:
    _defs[op.args[0]].modify_member(op.args[1])._name.assign((const char *)&op.args[2]);
    break;

  case spv::OpTypeVoid:
    record_type(op.args[0], nullptr);
    break;

  case spv::OpTypeBool:
    record_type(op.args[0], ShaderType::bool_type);
    break;

  case spv::OpTypeInt:
    {
      if (op.args[2]) {
        record_type(op.args[0], ShaderType::int_type);
      } else {
        record_type(op.args[0], ShaderType::uint_type);
      }
    }
    break;

  case spv::OpTypeFloat:
    {
      if (op.nargs >= 2 && op.args[1] >= 64) {
        record_type(op.args[0], ShaderType::double_type);
      } else {
        record_type(op.args[0], ShaderType::float_type);
      }
    }
    break;

  case spv::OpTypeVector:
    {
      const ShaderType::Scalar *element_type;
      DCAST_INTO_V(element_type, _defs[op.args[1]]._type);
      uint32_t component_count = op.args[2];
      record_type(op.args[0], ShaderType::register_type(
        ShaderType::Vector(element_type->get_scalar_type(), component_count)));
    }
    break;

  case spv::OpTypeMatrix:
    {
      const ShaderType::Vector *column_type;
      DCAST_INTO_V(column_type, _defs[op.args[1]]._type);
      uint32_t num_rows = op.args[2];
      record_type(op.args[0], ShaderType::register_type(
        ShaderType::Matrix(column_type->get_scalar_type(), num_rows, column_type->get_num_components())));
    }
    break;

  case spv::OpTypePointer:
    if (current_function_id != 0) {
      shader_cat.error()
        << "OpTypePointer" << " may not occur within a function!\n";
      return;
    }
    record_pointer_type(op.args[0], (spv::StorageClass)op.args[1], op.args[2]);
    break;

  case spv::OpTypeImage:
    {
      const ShaderType::Scalar *sampled_type;
      DCAST_INTO_V(sampled_type, _defs[op.args[1]]._type);

      Texture::TextureType texture_type;
      switch ((spv::Dim)op.args[2]) {
      case spv::Dim1D:
        if (op.args[4]) {
          texture_type = Texture::TT_1d_texture_array;
        } else {
          texture_type = Texture::TT_1d_texture;
        }
        break;

      case spv::Dim2D:
        if (op.args[4]) {
          texture_type = Texture::TT_2d_texture_array;
        } else {
          texture_type = Texture::TT_2d_texture;
        }
        break;

      case spv::Dim3D:
        texture_type = Texture::TT_3d_texture;
        break;

      case spv::DimCube:
        if (op.args[4]) {
          texture_type = Texture::TT_cube_map_array;
        } else {
          texture_type = Texture::TT_cube_map;
        }
        break;

      case spv::DimRect:
        shader_cat.error()
          << "imageRect shader inputs are not supported.\n";
        return;

      case spv::DimBuffer:
        texture_type = Texture::TT_buffer_texture;
        break;

      case spv::DimSubpassData:
        shader_cat.error()
          << "subpassInput shader inputs are not supported.\n";
        return;

      default:
        shader_cat.error()
          << "Unknown image dimensionality in OpTypeImage instruction.\n";
        return;
      }

      ShaderType::Access access = ShaderType::Access::read_write;
      if (op.nargs > 8) {
        switch ((spv::AccessQualifier)op.args[8]) {
        case spv::AccessQualifierReadOnly:
          access = ShaderType::Access::read_only;
          break;
        case spv::AccessQualifierWriteOnly:
          access = ShaderType::Access::write_only;
          break;
        case spv::AccessQualifierReadWrite:
          access = ShaderType::Access::read_write;
          break;
        default:
          shader_cat.error()
            << "Invalid access qualifier in OpTypeImage instruction.\n";
          break;
        }
      }
      if (_defs[op.args[0]]._flags & DF_non_writable) {
        access = (access & ShaderType::Access::read_only);
      }
      if (_defs[op.args[0]]._flags & DF_non_readable) {
        access = (access & ShaderType::Access::write_only);
      }

      record_type(op.args[0], ShaderType::register_type(
        ShaderType::Image(texture_type, sampled_type->get_scalar_type(), access)));

      // We don't record the "depth" flag on the image type (because no shader
      // language actually does that), so we have to store it somewhere else.
      if (op.args[3] == 1) {
        _defs[op.args[0]]._flags |= DF_depth_image;
      }
    }
    break;

  case spv::OpTypeSampler:
    // A sampler that's not bound to a particular image.
    record_type(op.args[0], ShaderType::sampler_type);
    break;

  case spv::OpTypeSampledImage:
    if (const ShaderType::Image *image = _defs[op.args[1]]._type->as_image()) {
      bool shadow = (_defs[op.args[1]]._flags & DF_depth_image) != 0;
      record_type(op.args[0], ShaderType::register_type(
        ShaderType::SampledImage(image->get_texture_type(), image->get_sampled_type(), shadow)));
    } else {
      shader_cat.error()
        << "OpTypeSampledImage must refer to an image type!\n";
      return;
    }
    break;

  case spv::OpTypeArray:
    if (_defs[op.args[1]]._type != nullptr) {
      record_type(op.args[0], ShaderType::register_type(
        ShaderType::Array(_defs[op.args[1]]._type, _defs[op.args[2]]._constant)));
    }
    _defs[op.args[0]]._type_id = op.args[1];
    break;

  case spv::OpTypeRuntimeArray:
    if (_defs[op.args[1]]._type != nullptr) {
      record_type(op.args[0], ShaderType::register_type(
        ShaderType::Array(_defs[op.args[1]]._type, 0)));
    }
    break;

  case spv::OpTypeStruct:
    {
      Definition &struct_def = _defs[op.args[0]];
      int access_flags = DF_non_writable | DF_non_readable;
      ShaderType::Struct type;
      for (size_t i = 0; i < op.nargs - 1; ++i) {
        uint32_t member_type_id = op.args[i + 1];
        if (member_type_id >= _defs.size() || !_defs[member_type_id].is_type()) {
          shader_cat.error()
            << "Struct type with id " << op.args[0]
            << " contains invalid member type " << member_type_id << "\n";
          return;
        }

        MemberDefinition &member_def = struct_def.modify_member(i);
        member_def._type_id = member_type_id;
        if (member_def._builtin != spv::BuiltInMax) {
          // Ignore built-in member.
          continue;
        }

        const ShaderType *member_type = _defs[member_type_id]._type;
        if (member_def._flags & (DF_non_writable | DF_non_readable)) {
          // If an image member has the readonly/writeonly qualifiers,
          // then we'll inject those back into the type.
          if (const ShaderType::Image *image = member_type->as_image()) {
            ShaderType::Access access = image->get_access();
            if (member_def._flags & DF_non_writable) {
              access = (access & ShaderType::Access::read_only);
            }
            if (member_def._flags & DF_non_readable) {
              access = (access & ShaderType::Access::write_only);
            }
            if (access != image->get_access()) {
              member_type = ShaderType::register_type(ShaderType::Image(
                image->get_texture_type(),
                image->get_sampled_type(),
                access));
            }
          }
        }
        if (member_def._offset >= 0) {
          type.add_member(member_type, member_def._name, (uint32_t)member_def._offset);
        } else {
          type.add_member(member_type, member_def._name);
        }

        // If any member is writable, the struct shan't be marked readonly.
        access_flags &= member_def._flags;
      }
      record_type(op.args[0], ShaderType::register_type(std::move(type)));

      // If all struct members are flagged readonly/writeonly, we tag the type
      // so as well, since glslang doesn't decorate an SSBO in its entirety as
      // readonly/writeonly properly (it applies it to all members instead)
      _defs[op.args[0]]._flags |= access_flags;
    }
    break;

  case spv::OpConstant:
    if (current_function_id != 0) {
      shader_cat.error()
        << "OpConstant" << " may not occur within a function!\n";
      return;
    }
    record_constant(op.args[1], op.args[0], op.args + 2, op.nargs - 2);
    break;

  case spv::OpConstantNull:
    if (current_function_id != 0) {
      shader_cat.error()
        << "OpConstantNull" << " may not occur within a function!\n";
      return;
    }
    record_constant(op.args[1], op.args[0], nullptr, 0);
    break;

  case spv::OpConstantComposite:
  case spv::OpSpecConstantComposite:
    modify_definition(op.args[1])._flags |= DF_constant_expression;
    break;

  case spv::OpSpecConstantTrue:
  case spv::OpSpecConstantFalse:
  case spv::OpSpecConstant:
    record_spec_constant(op.args[1], op.args[0]);
    break;

  case spv::OpFunction:
    if (current_function_id != 0) {
      shader_cat.error()
        << "OpFunction may not occur within another function!\n";
      return;
    }
    {
      const Definition &func_def = modify_definition(op.args[1]);
      if (func_def.is_function() && func_def._type_id != op.args[0]) {
        shader_cat.error()
          << "OpFunctionCall has mismatched return type ("
          << op.args[0] << " != " << func_def._type_id << ")\n";
        return;
      }
    }
    current_function_id = op.args[1];
    record_function(op.args[1], op.args[0]);
    break;

  case spv::OpFunctionParameter:
    if (current_function_id == 0) {
      shader_cat.error()
        << "OpFunctionParameter" << " may only occur within a function!\n";
      return;
    }
    record_function_parameter(op.args[1], op.args[0], current_function_id);
    break;

  case spv::OpFunctionEnd:
    if (current_function_id == 0) {
      shader_cat.error()
        << "OpFunctionEnd" << " may only occur within a function!\n";
      return;
    }
    current_function_id = 0;
    break;

  case spv::OpFunctionCall:
    if (current_function_id == 0) {
      shader_cat.error()
        << "OpFunctionCall" << " may only occur within a function!\n";
      return;
    }
    {
      Definition &func_def = modify_definition(op.args[2]);

      // Mark all arguments as used.  In the future we could be smart enough to
      // only mark the arguments used if the relevant parameters are used with
      // the function itself.
      for (size_t i = 3; i < op.nargs; ++i) {
        mark_used(op.args[i]);
      }

      // Error checking.  Note that it's valid for the function to not yet have
      // been defined.
      if (func_def.is_function()) {
        if (func_def._type_id != 0 && func_def._type_id != op.args[0]) {
          shader_cat.error()
            << "OpFunctionCall has mismatched return type ("
            << func_def._type_id << " != " << op.args[0] << ")\n";
          return;
        }
      }
      else if (func_def._dtype != DT_none) {
        shader_cat.error()
          << "OpFunctionCall tries to call non-function definition "
          << op.args[2] << "\n";
        return;
      }

      // Mark the function as used (even if its return value is unused - the
      // function may have side effects).  Note that it's legal for the function
      // to not yet have been declared.
      func_def._dtype = DT_function;
      func_def._flags |= DF_used;
      func_def._type_id = op.args[0];
      record_temporary(op.args[1], op.args[0], op.args[2], current_function_id);
    }
    break;

  case spv::OpVariable:
    record_variable(op.args[1], op.args[0], (spv::StorageClass)op.args[2], current_function_id);
    break;

  case spv::OpImageTexelPointer:
    record_temporary(op.args[1], op.args[0], op.args[2], current_function_id);
    break;

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
    record_temporary(op.args[1], op.args[0], op.args[2], current_function_id);

    // A load from the pointer is enough for us to consider it "used", for now.
    mark_used(op.args[1]);
    break;

  case spv::OpStore:
  case spv::OpAtomicStore:
  case spv::OpAtomicFlagClear:
    // An atomic write creates no result ID, but we do consider the var "used".
    mark_used(op.args[0]);
    break;

  case spv::OpCopyMemory:
  case spv::OpCopyMemorySized:
    mark_used(op.args[0]);
    mark_used(op.args[1]);
    break;

  case spv::OpAccessChain:
  case spv::OpInBoundsAccessChain:
  case spv::OpPtrAccessChain:
  case spv::OpInBoundsPtrAccessChain:
    // Record the access chain or pointer copy, so that as soon as something is
    // loaded through them we can transitively mark everything as "used".
    record_temporary(op.args[1], op.args[0], op.args[2], current_function_id);

    // If one of the indices (including the base element for OpPtrAccessChain)
    // isn't a constant expression, we mark the variable as dynamically-indexed.
    for (size_t i = 3; i < op.nargs; ++i) {
      if ((_defs[op.args[i]]._flags & DF_constant_expression) == 0) {
        const Definition &def = get_definition(op.args[1]);
        nassertv(def._origin_id != 0);
        _defs[def._origin_id]._flags |= DF_dynamically_indexed;
        break;
      }
    }
    break;

  case spv::OpArrayLength:
  case spv::OpConvertPtrToU:
    mark_used(op.args[2]);
    break;

  case spv::OpDecorate:
    switch ((spv::Decoration)op.args[1]) {
    case spv::DecorationBufferBlock:
      _defs[op.args[0]]._flags |= DF_buffer_block;
      break;

    case spv::DecorationBuiltIn:
      _defs[op.args[0]]._builtin = (spv::BuiltIn)op.args[2];
      break;

    case spv::DecorationNonWritable:
      _defs[op.args[0]]._flags |= DF_non_writable;
      break;

    case spv::DecorationNonReadable:
      _defs[op.args[0]]._flags |= DF_non_readable;
      break;

    case spv::DecorationLocation:
      _defs[op.args[0]]._location = op.args[2];
      break;

    case spv::DecorationArrayStride:
      _defs[op.args[0]]._array_stride = op.args[2];
      break;

    case spv::DecorationSpecId:
      _defs[op.args[0]]._spec_id = op.args[2];
      break;

    default:
      break;
    }
    break;

  case spv::OpMemberDecorate:
    switch ((spv::Decoration)op.args[2]) {
    case spv::DecorationBuiltIn:
      _defs[op.args[0]].modify_member(op.args[1])._builtin = (spv::BuiltIn)op.args[3];
      break;

    case spv::DecorationNonWritable:
      _defs[op.args[0]].modify_member(op.args[1])._flags |= DF_non_writable;
      break;

    case spv::DecorationNonReadable:
      _defs[op.args[0]].modify_member(op.args[1])._flags |= DF_non_readable;
      break;

    case spv::DecorationLocation:
      _defs[op.args[0]].modify_member(op.args[1])._location = op.args[3];
      break;

    case spv::DecorationBinding:
      shader_cat.error()
        << "Invalid " << "binding" << " decoration on struct member\n";
      break;

    case spv::DecorationDescriptorSet:
      shader_cat.error()
        << "Invalid " << "set" << " decoration on struct member\n";
      break;

    case spv::DecorationOffset:
      _defs[op.args[0]].modify_member(op.args[1])._offset = op.args[3];
      break;

    default:
      break;
    }
    break;

  case spv::OpCompositeConstruct:
    //XXX Not sure that we even need this, since it's probably not possible to
    // construct a composite from pointers?
    for (size_t i = 2; i < op.nargs; ++i) {
      mark_used(op.args[i]);
    }
    break;

  case spv::OpCopyObject:
    record_temporary(op.args[1], op.args[0], op.args[2], current_function_id);
    // fall through

  case spv::OpCompositeExtract:
    // Composite types are used for some arithmetic ops.
    if (_defs[op.args[2]]._flags & DF_constant_expression) {
      _defs[op.args[1]]._flags |= DF_constant_expression;
    }
    break;

  case spv::OpImageSampleImplicitLod:
  case spv::OpImageSampleExplicitLod:
  case spv::OpImageSampleProjImplicitLod:
  case spv::OpImageSampleProjExplicitLod:
  case spv::OpImageFetch:
  case spv::OpImageGather:
  case spv::OpImageSparseSampleImplicitLod:
  case spv::OpImageSparseSampleExplicitLod:
  case spv::OpImageSparseSampleProjImplicitLod:
  case spv::OpImageSparseSampleProjExplicitLod:
  case spv::OpImageSparseFetch:
  case spv::OpImageSparseGather:
    // Indicate that this variable was sampled with a non-dref sampler.
    {
      uint32_t var_id = _defs[op.args[2]]._origin_id;
      if (var_id != 0) {
        _defs[var_id]._flags |= DF_non_dref_sampled;
      }
    }
    break;

  case spv::OpImageSampleDrefImplicitLod:
  case spv::OpImageSampleDrefExplicitLod:
  case spv::OpImageSampleProjDrefImplicitLod:
  case spv::OpImageSampleProjDrefExplicitLod:
  case spv::OpImageDrefGather:
  case spv::OpImageSparseSampleDrefImplicitLod:
  case spv::OpImageSparseSampleDrefExplicitLod:
  case spv::OpImageSparseSampleProjDrefImplicitLod:
  case spv::OpImageSparseSampleProjDrefExplicitLod:
  case spv::OpImageSparseDrefGather:
    // Indicate that this variable was sampled with a dref sampler.
    {
      uint32_t var_id = _defs[op.args[2]]._origin_id;
      if (var_id != 0) {
        _defs[var_id]._flags |= DF_dref_sampled;
      }
    }
    break;

  case spv::OpBitcast:
    record_temporary(op.args[1], op.args[0], op.args[2], current_function_id);

    // Treat this like a load if it is casting to a non-pointer type.
    if (_defs[op.args[0]]._dtype != DT_pointer_type) {
      mark_used(op.args[1]);
    }
    // fall through, counts as unary arithmetic
  case spv::OpConvertFToU:
  case spv::OpConvertFToS:
  case spv::OpConvertSToF:
  case spv::OpConvertUToF:
  case spv::OpQuantizeToF16:
  case spv::OpSatConvertSToU:
  case spv::OpSatConvertUToS:
  case spv::OpConvertUToPtr:
  case spv::OpSNegate:
  case spv::OpFNegate:
  case spv::OpAny:
  case spv::OpAll:
  case spv::OpIsNan:
  case spv::OpIsInf:
  case spv::OpIsFinite:
  case spv::OpIsNormal:
  case spv::OpSignBitSet:
  case spv::OpNot:
    if ((_defs[op.args[2]]._flags & DF_constant_expression) != 0) {
      _defs[op.args[1]]._flags |= DF_constant_expression;
    }
    break;

  // Binary arithmetic operators
  case spv::OpIAdd:
  case spv::OpFAdd:
  case spv::OpISub:
  case spv::OpFSub:
  case spv::OpIMul:
  case spv::OpFMul:
  case spv::OpUDiv:
  case spv::OpSDiv:
  case spv::OpFDiv:
  case spv::OpUMod:
  case spv::OpSRem:
  case spv::OpSMod:
  case spv::OpFRem:
  case spv::OpFMod:
  case spv::OpVectorTimesScalar:
  case spv::OpMatrixTimesScalar:
  case spv::OpVectorTimesMatrix:
  case spv::OpMatrixTimesVector:
  case spv::OpMatrixTimesMatrix:
  case spv::OpOuterProduct:
  case spv::OpDot:
  case spv::OpIAddCarry:
  case spv::OpISubBorrow:
  case spv::OpUMulExtended:
  case spv::OpSMulExtended:
  case spv::OpShiftRightLogical:
  case spv::OpShiftRightArithmetic:
  case spv::OpShiftLeftLogical:
  case spv::OpBitwiseOr:
  case spv::OpBitwiseXor:
  case spv::OpBitwiseAnd:
    if ((_defs[op.args[2]]._flags & DF_constant_expression) != 0 &&
        (_defs[op.args[3]]._flags & DF_constant_expression) != 0) {
      _defs[op.args[1]]._flags |= DF_constant_expression;
    }
    break;

  case spv::OpSelect:
    // This can in theory operate on pointers, which is why we handle this
    //mark_used(op.args[2]);
    mark_used(op.args[3]);
    mark_used(op.args[4]);

    if ((_defs[op.args[2]]._flags & DF_constant_expression) != 0 &&
        (_defs[op.args[3]]._flags & DF_constant_expression) != 0 &&
        (_defs[op.args[4]]._flags & DF_constant_expression) != 0) {
      _defs[op.args[1]]._flags |= DF_constant_expression;
    }
    break;

  case spv::OpReturnValue:
    // A pointer can be returned when certain caps are present, so track it.
    mark_used(op.args[0]);
    break;

  case spv::OpPtrEqual:
  case spv::OpPtrNotEqual:
  case spv::OpPtrDiff:
    // Consider a variable "used" if its pointer value is being compared, to be
    // on the safe side.
    mark_used(op.args[2]);
    mark_used(op.args[3]);
    break;

  default:
    break;
  }
}

/**
 * Searches for an already-defined type.
 * Returns its id, or 0 if it was not found.
 */
uint32_t SpirVResultDatabase::
find_type(const ShaderType *type) {
  TypeMap::const_iterator tit = _type_map.find(type);
  if (tit != _type_map.end()) {
    return tit->second;
  } else {
    return 0;
  }
}

/**
 * Searches for an already-defined type pointer of the given storage class.
 * Returns its id, or 0 if it was not found.
 */
uint32_t SpirVResultDatabase::
find_pointer_type(const ShaderType *type, spv::StorageClass storage_class) {
  TypeMap::const_iterator tit = _type_map.find(type);
  if (tit == _type_map.end()) {
    return 0;
  }
  uint32_t type_id = tit->second;

  for (uint32_t id = 0; id < _defs.size(); ++id) {
    Definition &def = _defs[id];
    if (def._dtype == DT_pointer_type &&
        def._type_id == type_id &&
        def._storage_class == storage_class) {
      return id;
    }
  }
  return 0;
}

/**
 * Records that the given type has been defined.
 */
void SpirVResultDatabase::
record_type(uint32_t id, const ShaderType *type) {
  Definition &def = modify_definition(id);
  def._dtype = DT_type;
  def._type = type;

  if (shader_cat.is_spam()) {
    if (type != nullptr) {
      shader_cat.spam()
        << "Defined type " << id << ": " << *type << "\n";
    } else {
      shader_cat.spam()
        << "Defined type " << id << ": void\n";
    }
  }

  if (!def.has_builtin()) {
    // Only put types we can fully round-trip in the type map.
    _type_map[type] = id;
  }
}

/**
 * Records that the given type pointer has been defined.
 */
void SpirVResultDatabase::
record_pointer_type(uint32_t id, spv::StorageClass storage_class, uint32_t type_id) {
  // Call modify_definition first, because it may invalidate references
  Definition &def = modify_definition(id);

  const Definition &type_def = get_definition(type_id);
  nassertv(type_def._dtype == DT_type || type_def._dtype == DT_pointer_type);

  def._dtype = DT_pointer_type;
  def._type = type_def._type;
  def._storage_class = storage_class;
  def._type_id = type_id;
}

/**
 * Records that the given variable has been defined.
 */
void SpirVResultDatabase::
record_variable(uint32_t id, uint32_t pointer_type_id, spv::StorageClass storage_class, uint32_t function_id) {
  // Call modify_definition first, because it may invalidate references
  Definition &def = modify_definition(id);

  const Definition &pointer_type_def = get_definition(pointer_type_id);
  if (pointer_type_def._dtype != DT_pointer_type && pointer_type_def._type_id != 0) {
    shader_cat.error()
      << "Variable " << id << " should have valid pointer type\n";
    return;
  }

  const Definition &type_def = get_definition(pointer_type_def._type_id);
  if (type_def._dtype != DT_type) {
    shader_cat.error()
      << "Type pointer " << pointer_type_id << " should point to valid type "
         "for variable " << id << "\n";
    return;
  }

  // In older versions of SPIR-V, an SSBO was defined using BufferBlock.
  if (storage_class == spv::StorageClassUniform &&
      (type_def._flags & DF_buffer_block) != 0) {
    storage_class = spv::StorageClassStorageBuffer;
  }

  def._dtype = DT_variable;
  def._type = type_def._type;
  def._type_id = pointer_type_id;
  def._storage_class = storage_class;
  def._origin_id = id;
  def._function_id = function_id;

  if (storage_class == spv::StorageClassStorageBuffer) {
    // Inherit readonly/writeonly from the variable but also from the struct.
    int flags = def._flags | type_def._flags;
    ShaderType::Access access = ShaderType::Access::read_write;
    if (flags & DF_non_writable) {
      access = (access & ShaderType::Access::read_only);
    }
    if (flags & DF_non_readable) {
      access = (access & ShaderType::Access::write_only);
    }
    def._type = ShaderType::register_type(ShaderType::StorageBuffer(def._type, access));

    if (shader_cat.is_debug()) {
      std::ostream &out = shader_cat.debug()
        << "Defined buffer " << id;
      if (!def._name.empty()) {
        out << ": " << def._name;
      }
      out << " with type " << *def._type << "\n";
    }
  }
  else if (def._flags & (DF_non_writable | DF_non_readable)) {
    // If an image variable has the readonly/writeonly qualifiers, then we'll
    // inject those back into the type.
    if (const ShaderType::Image *image = def._type->as_image()) {
      ShaderType::Access access = image->get_access();
      if (def._flags & DF_non_writable) {
        access = (access & ShaderType::Access::read_only);
      }
      if (def._flags & DF_non_readable) {
        access = (access & ShaderType::Access::write_only);
      }
      if (access != image->get_access()) {
        def._type = ShaderType::register_type(ShaderType::Image(
          image->get_texture_type(),
          image->get_sampled_type(),
          access));
      }
    }
  }

#ifndef NDEBUG
  if (storage_class == spv::StorageClassUniformConstant && shader_cat.is_debug()) {
    std::ostream &out = shader_cat.debug()
      << "Defined uniform " << id;

    if (!def._name.empty()) {
      out << ": " << def._name;
    }

    if (def.has_location()) {
      out << " (location " << def._location << ")";
    }

    out << " with ";

    if (def._type != nullptr) {
      out << "type " << *def._type << "\n";
    } else {
      out << "unknown type\n";
    }
  }
#endif
}

/**
 * Records that the given function parameter has been defined.
 */
void SpirVResultDatabase::
record_function_parameter(uint32_t id, uint32_t type_id, uint32_t function_id) {
  // Call modify_definition first, because it may invalidate references
  Definition &def = modify_definition(id);

  const Definition &type_def = get_definition(type_id);
  nassertv(type_def._dtype == DT_type || type_def._dtype == DT_pointer_type);

  def._dtype = DT_function_parameter;
  def._type_id = type_id;
  def._type = type_def._type;
  def._origin_id = id;
  def._function_id = function_id;

  nassertv(function_id != 0);
}

/**
 * Records that the given constant has been defined.
 */
void SpirVResultDatabase::
record_constant(uint32_t id, uint32_t type_id, const uint32_t *words, uint32_t nwords) {
  // Call modify_definition first, because it may invalidate references
  Definition &def = modify_definition(id);

  const Definition &type_def = get_definition(type_id);

  def._dtype = DT_constant;
  def._type_id = type_id;
  def._type = (type_def._dtype == DT_type) ? type_def._type : nullptr;
  def._constant = (nwords > 0) ? words[0] : 0;
  def._flags |= DF_constant_expression;
}

/**
 * Records an external import.
 */
void SpirVResultDatabase::
record_ext_inst_import(uint32_t id, const char *import) {
  Definition &def = modify_definition(id);
  def._dtype = DT_ext_inst;
  def._name.assign(import);
}

/**
 * Records that the given function has been defined.
 */
void SpirVResultDatabase::
record_function(uint32_t id, uint32_t type_id) {
  // Call modify_definition first, because it may invalidate references
  Definition &def = modify_definition(id);

  const Definition &type_def = get_definition(type_id);

  def._dtype = DT_function;
  def._type = type_def._type;
  def._type_id = type_id;
  def._function_id = id;
}

/**
 * Record a temporary.  We mostly use this to record the chain of loads and
 * copies so that we can figure out whether (and how) a given variable is used.
 *
 * from_id indicates from what this variable is initialized or generated, for
 * the purpose of transitively tracking usage.
 */
void SpirVResultDatabase::
record_temporary(uint32_t id, uint32_t type_id, uint32_t from_id, uint32_t function_id) {
  // Call modify_definition first, because it may invalidate references
  Definition &def = modify_definition(id);

  const Definition &type_def = get_definition(type_id);
  const Definition &from_def = get_definition(from_id);

  def._dtype = DT_temporary;
  def._type = type_def._type;
  def._type_id = type_id;
  def._origin_id = from_def._origin_id;
  def._function_id = function_id;

  nassertv(function_id != 0);
}

/**
 * Records that the given specialization constant has been defined.
 */
void SpirVResultDatabase::
record_spec_constant(uint32_t id, uint32_t type_id) {
  // Call modify_definition first, because it may invalidate references
  Definition &def = modify_definition(id);

  const Definition &type_def = get_definition(type_id);
  nassertv(type_def._dtype == DT_type);

  def._dtype = DT_spec_constant;
  def._type_id = type_id;
  def._type = type_def._type;
  def._flags |= DF_constant_expression;
}

/**
 * Called for a variable, or any id whose value (indirectly) originates from a
 * variable, to mark the variable and any types used thereby as "used".
 */
void SpirVResultDatabase::
mark_used(uint32_t id) {
  nassertv(!_defs[id].is_type());

  uint32_t origin_id = _defs[id]._origin_id;
  if (origin_id != 0) {
    Definition &origin_def = _defs[origin_id];
    if (!origin_def.is_used()) {
      origin_def._flags |= DF_used;

      // Also mark the type pointer as used.
      if (origin_def._type_id != 0) {
        Definition &pointer_type_def = _defs[origin_def._type_id];
        pointer_type_def._flags |= DF_used;

        // And the type that references.
        if (pointer_type_def._type_id != 0) {
          Definition &type_def = _defs[pointer_type_def._type_id];
          type_def._flags |= DF_used;
        }
      }
    }
  } else {
    // Variables must define an origin (even if it is just itself)
    nassertv(!_defs[id].is_variable());
  }
}
