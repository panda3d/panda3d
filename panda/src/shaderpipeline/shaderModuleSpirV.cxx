/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModuleSpirV.cxx
 * @author rdb
 * @date 2019-07-15
 */

#include "shaderModuleSpirV.h"
#include "string_utils.h"
#include "shaderType.h"

#include "spirVTransformer.h"
#include "spirVFlattenStructPass.h"
#include "spirVReplaceVariableTypePass.h"
#include "spirVRemoveUnusedVariablesPass.h"

#include "GLSL.std.450.h"

#include <spirv-tools/libspirv.h>

#ifndef NDEBUG
#include <glslang/SPIRV/disassemble.h>
#endif

using Definition = SpirVResultDatabase::Definition;

TypeHandle ShaderModuleSpirV::_type_handle;

/**
 * Takes a stream of SPIR-V instructions, and processes it as follows:
 * - All the definitions are parsed out (requires debug info present)
 * - Makes sure that all the inputs have location indices assigned.
 * - Builds up the lists of inputs, outputs and parameters.
 * - Strips debugging information from the module.
 */
ShaderModuleSpirV::
ShaderModuleSpirV(Stage stage, std::vector<uint32_t> words, BamCacheRecord *record) :
  ShaderModule(stage),
  _instructions(std::move(words))
{
  if (!_instructions.validate_header()) {
    return;
  }
  _record = record;

  // Check for caps and sanity.
  for (InstructionIterator it = _instructions.begin(); it != _instructions.begin_annotations(); ++it) {
    Instruction op = *it;
    switch (op.opcode) {
    case spv::OpMemoryModel:
      if (op.args[0] != spv::AddressingModelLogical) {
        shader_cat.error()
          << "Invalid SPIR-V shader: addressing model Logical must be used.\n";
        return;
      }
      if (op.args[1] != spv::MemoryModelGLSL450) {
        shader_cat.error()
          << "Invalid SPIR-V shader: memory model GLSL450 must be used.\n";
        return;
      }
      break;

    case spv::OpExecutionMode:
      if (op.nargs >= 3 && op.args[1] == spv::ExecutionModeInvocations) {
        if (op.args[2] != 1) {
          _used_caps |= C_geometry_shader_instancing;
        }
      }
      break;

    case spv::OpCapability:
      switch ((spv::Capability)op.args[0]) {
      case spv::CapabilityFloat64:
        _used_caps |= C_double;
        break;

      case spv::CapabilityAtomicStorage:
        _used_caps |= C_atomic_counters;
        break;

      case spv::CapabilityUniformBufferArrayDynamicIndexing:
      case spv::CapabilitySampledImageArrayDynamicIndexing:
      case spv::CapabilityStorageBufferArrayDynamicIndexing:
      case spv::CapabilityStorageImageArrayDynamicIndexing:
      case spv::CapabilityInputAttachmentArrayDynamicIndexing:
      case spv::CapabilityUniformTexelBufferArrayDynamicIndexing:
      case spv::CapabilityStorageTexelBufferArrayDynamicIndexing:
        // It would be great if we could rely on this and call this a day.
        // However, glslang is not currently capable of detecting and generating
        // this capability (see KhronosGroup/glslang#2056).  So we still have to
        // go through the trouble of determining this ourselves.
        _used_caps |= C_dynamic_indexing;
        break;

      case spv::CapabilityClipDistance:
        _used_caps |= C_clip_distance;
        break;

      case spv::CapabilityCullDistance:
        _used_caps |= C_cull_distance;
        break;

      case spv::CapabilityImageCubeArray:
      case spv::CapabilitySampledCubeArray:
        _used_caps |= C_cube_map_array;
        break;

      case spv::CapabilitySampledBuffer:
      case spv::CapabilityImageBuffer:
        _used_caps |= C_texture_buffer;
        break;

      case spv::CapabilityDerivativeControl:
        _used_caps |= C_derivative_control;
        break;

      default:
        break;
      }
      break;

    default:
      break;
    }
  }

  SpirVTransformer transformer(_instructions);
  const SpirVResultDatabase &db = transformer.get_db();

  // Check if there is a $Global uniform block.  This is generated by the HLSL
  // front-end of glslang.  If so, unwrap it back down to individual uniforms.
  uint32_t type_id = db.find_definition("$Global");
  if (type_id) {
    if (shader_cat.is_spam()) {
      shader_cat.spam()
        << "Flattening $Global uniform block with type " << type_id << "\n";
    }
    transformer.run(SpirVFlattenStructPass(type_id));
  }

  // Remove unused variables before assigning locations.
  transformer.run(SpirVRemoveUnusedVariablesPass());

  // Add in location decorations for any inputs that are missing it.
  transformer.assign_locations(stage);

  // Identify the inputs, outputs and uniform parameters.
  for (uint32_t id = 0; id < transformer.get_id_bound(); ++id) {
    const Definition &def = db.get_definition(id);

    if (def.is_used() && def._type != nullptr) {
      if (def._type->contains_scalar_type(ShaderType::ST_double)) {
        _used_caps |= C_double;
      }
      if (const ShaderType::Matrix *matrix_type = def._type->as_matrix()) {
        if (matrix_type->get_num_rows() != matrix_type->get_num_columns()) {
          _used_caps |= C_non_square_matrices;
        }
      }
      if (def._type->contains_scalar_type(ShaderType::ST_uint)) {
        _used_caps |= C_unified_model;
      }
    }

    if (def.is_variable() && !def.is_builtin()) {
      // Ignore empty structs/arrays.
      int num_locations = def._type->get_num_interface_locations();
      if (num_locations == 0) {
        continue;
      }

      Variable var;
      var.type = def._type;
      var.name = InternalName::make(def._name);
      var._location = def._location;
      var.id = id;

      if (def._storage_class == spv::StorageClassInput) {
        _inputs.push_back(std::move(var));

        if (stage == Stage::fragment) {
          // Integer varyings require shader model 4.
          if (def._type->contains_scalar_type(ShaderType::ST_uint) ||
              def._type->contains_scalar_type(ShaderType::ST_int) ||
              def._type->contains_scalar_type(ShaderType::ST_bool)) {
            _used_caps |= C_unified_model;
          }
        }
      }
      else if (def._storage_class == spv::StorageClassOutput) {
        if (!_outputs.empty() || num_locations > 1) {
          // This shader requires MRT.
          _used_caps |= C_draw_buffers;
        }

        _outputs.push_back(std::move(var));
      }
      else if (def._storage_class == spv::StorageClassUniformConstant) {
        const ShaderType::SampledImage *sampled_image_type =
          def._type->as_sampled_image();
        if (sampled_image_type != nullptr) {
          // Image variable sampled with depth ref.  Make sure this is actually
          // a shadow sampler; this isn't always done by the compiler, and the
          // spec isn't clear that this is necessary, but it helps spirv-cross
          // properly generate shadow samplers.
          if (def.is_dref_sampled() && !sampled_image_type->is_shadow()) {
            // No, change the type of this variable.
            var.type = ShaderType::register_type(ShaderType::SampledImage(
              sampled_image_type->get_texture_type(),
              sampled_image_type->get_sampled_type(),
              true));

            transformer.run(SpirVReplaceVariableTypePass(id, var.type, spv::StorageClassUniformConstant));
          }
          if (sampled_image_type->get_sampled_type() == ShaderType::ST_uint ||
              sampled_image_type->get_sampled_type() == ShaderType::ST_int) {
            _used_caps |= C_texture_integer;
          }
          switch (sampled_image_type->get_texture_type()) {
          case Texture::TT_buffer_texture:
            _used_caps |= C_texture_buffer;
            break;
          case Texture::TT_cube_map_array:
            _used_caps |= C_cube_map_array;
            // fall through
          case Texture::TT_1d_texture_array:
          case Texture::TT_2d_texture_array:
            _used_caps |= C_texture_array;
            break;
          default:
            break;
          }
        }
        if (def.is_dynamically_indexed() &&
            (sampled_image_type != nullptr || def._type->contains_opaque_type())) {
          _used_caps |= C_dynamic_indexing;
        }
        _parameters.push_back(std::move(var));
      }
      else if (def._storage_class == spv::StorageClassStorageBuffer) {
        // For whatever reason, in GLSL, the name of an SSBO is derived from the
        // name of the struct type.
        const Definition &type_pointer_def = db.get_definition(def._type_id);
        nassertd(type_pointer_def.is_pointer_type()) continue;
        const Definition &type_def = db.get_definition(type_pointer_def._type_id);
        nassertd(type_def.is_type()) continue;
        nassertd(!type_def._name.empty()) continue;

        var.name = InternalName::make(type_def._name);
        _parameters.push_back(std::move(var));

        _used_caps |= C_storage_buffer;
      }
    }
    else if (def.is_variable() && def.is_used() &&
             def._storage_class == spv::StorageClassInput) {
      // Built-in input variable.
      switch (def._builtin) {
      case spv::BuiltInClipDistance:
        if ((_used_caps & C_clip_distance) == 0) {
          shaderpipeline_cat.warning()
            << "Shader uses " << "ClipDistance"
            << ", but does not declare capability!\n";

          _used_caps |= C_clip_distance;
        }
        break;

      case spv::BuiltInCullDistance:
        if ((_used_caps & C_cull_distance) == 0) {
          shaderpipeline_cat.warning()
            << "Shader uses " << "CullDistance"
            << ", but does not declare capability!\n";

          _used_caps |= C_cull_distance;
        }
        break;

      case spv::BuiltInVertexId:
        _used_caps |= C_vertex_id;
        break;

      case spv::BuiltInInstanceId:
        _used_caps |= C_instance_id;
        break;

      case spv::BuiltInPrimitiveId:
        _used_caps |= C_primitive_id;
        break;

      case spv::BuiltInPointCoord:
        _used_caps |= C_point_coord;
        break;

      case spv::BuiltInSampleId:
      case spv::BuiltInSampleMask:
      case spv::BuiltInSamplePosition:
        _used_caps |= C_sample_variables;
        break;

      default:
        break;
      }
    }
    else if (def.is_type() && def._type != nullptr) {
      if (const ShaderType::Matrix *matrix_type = def._type->as_matrix()) {
        if (matrix_type->get_num_rows() != matrix_type->get_num_columns()) {
          _used_caps |= C_non_square_matrices;
        }
      }
    }
    else if (def.is_spec_constant() && def._type != nullptr) {
      SpecializationConstant spec_constant;
      spec_constant.id = def._spec_id;
      spec_constant.name = InternalName::make(def._name);
      spec_constant.type = def._type;
      if (shader_cat.is_debug()) {
        shader_cat.debug()
          << "Found specialization constant " << def._name << " with type "
          << *def._type << " and ID " << def._spec_id << "\n";
      }
      _spec_constants.push_back(spec_constant);
    }
  }

  _instructions = transformer.get_result();

#ifndef NDEBUG
  if (shader_cat.is_spam()) {
    _instructions.disassemble(shader_cat.spam()
      << "Disassembly for " << *this << ":\n");
  }
#endif

  // We no longer need the debugging information, so it can be safely stripped
  // from the module.
  strip();

#ifndef NDEBUG
  _instructions.validate();
#endif

  // Check for more caps, now that we've optimized the module.
  for (InstructionIterator it = _instructions.begin_annotations(); it != _instructions.end_annotations(); ++it) {
    Instruction op = *it;
    if (op.opcode == spv::OpDecorate) {
      if (db.get_definition(op.args[0]).is_builtin()) {
        continue;
      }
      switch ((spv::Decoration)op.args[1]) {
      case spv::DecorationNoPerspective:
        _used_caps |= C_noperspective_interpolation;
        break;
      case spv::DecorationFlat:
        _used_caps |= C_unified_model;
        break;
      case spv::DecorationSample:
        _used_caps |= C_multisample_interpolation;
        break;
      case spv::DecorationInvariant:
        //_used_caps |= C_invariant;
        break;
      case spv::DecorationComponent:
        _used_caps |= C_enhanced_layouts;
        break;
      default:
        break;
      }
    }
  }

  for (InstructionIterator it = _instructions.begin_functions(); it != _instructions.end(); ++it) {
    Instruction op = *it;
    switch (op.opcode) {
    case spv::OpExtInst:
      {
        const Definition &def = db.get_definition(op.args[2]);
        nassertv(def.is_ext_inst());
        if (def._name == "GLSL.std.450" && op.args[3] == GLSLstd450RoundEven) {
          // We mark the use of the GLSL roundEven() function, which requires
          // GLSL 1.30 or HLSL SM 4.0.
          _used_caps |= C_unified_model;
        }
      }
      break;

    case spv::OpImageTexelPointer:
      // These can only be used for atomic ops.
      _used_caps |= C_image_load_store | C_image_atomic;
      break;

    case spv::OpImageRead:
    case spv::OpImageWrite:
    case spv::OpImageSparseRead:
      _used_caps |= C_image_load_store;
      break;

    case spv::OpImageSampleExplicitLod:
    case spv::OpImageSampleProjExplicitLod:
      if (stage != Stage::vertex) {
        _used_caps |= C_texture_lod;
      }
      // fall through
    case spv::OpImageSampleImplicitLod:
    case spv::OpImageSampleProjImplicitLod:
      if (stage == Stage::vertex) {
        _used_caps |= C_vertex_texture;
      }
      if (op.nargs >= 5 && (op.args[4] & spv::ImageOperandsGradMask) != 0) {
        _used_caps |= C_texture_lod;
      }
      break;

    case spv::OpImageSampleDrefExplicitLod:
    case spv::OpImageSampleProjDrefExplicitLod:
      if (stage != Stage::vertex) {
        _used_caps |= C_texture_lod;
      }
      // fall through
    case spv::OpImageSampleDrefImplicitLod:
    case spv::OpImageSampleProjDrefImplicitLod:
      _used_caps |= C_shadow_samplers;

      {
        const Definition &sampler_def = db.get_definition(op.args[2]);
        if (sampler_def._type != nullptr) {
          const ShaderType::SampledImage *sampler = sampler_def._type->as_sampled_image();
          if (sampler != nullptr &&
              sampler->get_texture_type() == Texture::TT_cube_map) {
            _used_caps |= C_sampler_cube_shadow;
          }
        }
      }
      if (stage == Stage::vertex) {
        _used_caps |= C_vertex_texture;
      }
      if (op.nargs >= 5 && (op.args[4] & spv::ImageOperandsGradMask) != 0) {
        _used_caps |= C_texture_lod;
      }
      break;

    case spv::OpImageFetch:
      _used_caps |= C_unified_model;
      break;

    case spv::OpImageQuerySizeLod:
    case spv::OpImageQuerySize:
      {
        const Definition &image_def = db.get_definition(op.args[2]);
        if (image_def._type != nullptr && image_def._type->as_image() != nullptr) {
          _used_caps |= C_image_query_size;
        } else {
          _used_caps |= C_texture_query_size;
        }
      }
      break;

    case spv::OpImageGather:
    case spv::OpImageSparseGather:
      {
        const Definition &component = db.get_definition(op.args[4]);
        if (component.is_constant() && component._constant == 0) {
          _used_caps |= C_texture_gather_red;
        } else {
          _used_caps |= C_texture_gather_any;
        }
      }
      break;

    case spv::OpImageDrefGather:
    case spv::OpImageSparseDrefGather:
      _used_caps |= C_texture_gather_any;
      break;

    case spv::OpImageQueryLod:
      _used_caps |= C_texture_query_lod;
      break;

    case spv::OpImageQueryLevels:
      _used_caps |= C_texture_query_levels;
      break;

    case spv::OpImageQuerySamples:
      _used_caps |= C_texture_query_samples;
      break;

    case spv::OpBitcast:
      _used_caps |= C_bit_encoding;
      break;

    case spv::OpConvertFToU:
    case spv::OpConvertUToF:
    case spv::OpUConvert:
    case spv::OpUDiv:
    case spv::OpUMod:
    case spv::OpUMulExtended:
    case spv::OpUGreaterThan:
    case spv::OpUGreaterThanEqual:
    case spv::OpULessThan:
    case spv::OpULessThanEqual:
    case spv::OpShiftRightLogical:
      _used_caps |= C_unified_model;
      if (op.opcode != spv::OpUMulExtended) {
        break;
      }
      // fall through

    case spv::OpIAddCarry:
    case spv::OpISubBorrow:
    case spv::OpSMulExtended:
      _used_caps |= C_extended_arithmetic;
      break;

    case spv::OpDPdxFine:
    case spv::OpDPdyFine:
    case spv::OpFwidthFine:
    case spv::OpDPdxCoarse:
    case spv::OpDPdyCoarse:
    case spv::OpFwidthCoarse:
      _used_caps |= C_derivative_control;
      // fall through

    case spv::OpDPdx:
    case spv::OpDPdy:
    case spv::OpFwidth:
      _used_caps |= C_standard_derivatives;
      break;

    default:
      break;
    }
  }
}

ShaderModuleSpirV::
~ShaderModuleSpirV() {
}

/**
 * Required to implement CopyOnWriteObject.
 */
PT(CopyOnWriteObject) ShaderModuleSpirV::
make_cow_copy() {
  return new ShaderModuleSpirV(*this);
}

std::string ShaderModuleSpirV::
get_ir() const {
  return std::string();
}

/**
 * Links the stage with the given previous stage, by matching up its inputs with
 * the outputs of the previous stage.  Rather than reassigning the locations
 * directly, this method just returns the location remappings that need to be
 * made, or returns false if the stages cannot be linked.
 */
bool ShaderModuleSpirV::
link_inputs(const ShaderModule *previous, pmap<int, int> &remap) const {
  if (!previous->is_of_type(ShaderModuleSpirV::get_class_type())) {
    return false;
  }
  if (previous->get_stage() >= get_stage()) {
    return false;
  }

  ShaderModuleSpirV *spv_prev = (ShaderModuleSpirV *)previous;

  for (const Variable &input : _inputs) {
    int i = spv_prev->find_output(input.name);
    if (i < 0) {
      shader_cat.error()
        << "Input " << *input.name << " in stage " << get_stage()
        << " does not match any output in stage " << previous->get_stage() << "!\n";
      return false;
    }

    const Variable &output = spv_prev->get_output((size_t)i);
    if (!output.has_location()) {
      shader_cat.error()
        << "Output " << *input.name << " in stage " << previous->get_stage()
        << " has no output location!\n";
      return false;
    }

    if (!input.has_location() || output.get_location() != input.get_location()) {
      remap[input.get_location()] = output.get_location();
    }
  }

  return true;
}

/**
 * Remaps parameters with a given location to a given other location.  Locations
 * not included in the map remain untouched.
 */
void ShaderModuleSpirV::
remap_input_locations(const pmap<int, int> &locations) {
  remap_locations(spv::StorageClassInput, locations);

  for (Variable &input : _inputs) {
    if (input.has_location()) {
      pmap<int, int>::const_iterator it = locations.find(input.get_location());
      if (it != locations.end()) {
        input._location = it->second;
      }
    }
  }
}

/**
 * Remaps parameters with a given location to a given other location.  Locations
 * not included in the map remain untouched.
 */
void ShaderModuleSpirV::
remap_parameter_locations(const pmap<int, int> &locations) {
  remap_locations(spv::StorageClassUniformConstant, locations);

  // If we extracted out the parameters, replace the locations there as well.
  for (Variable &parameter : _parameters) {
    if (parameter.has_location()) {
      pmap<int, int>::const_iterator it = locations.find(parameter.get_location());
      if (it != locations.end()) {
        parameter._location = it->second;
      }
    }
  }
}

/**
 * Validates the header of the instruction stream.
 */
bool ShaderModuleSpirV::InstructionStream::
validate_header() const {
  if (_words.size() < 5) {
    shader_cat.error()
      << "Invalid SPIR-V file: too short.\n";
    return false;
  }

  // Validate the header.
  const uint32_t *words = (const uint32_t *)&_words[0];
  if (*words++ != spv::MagicNumber) {
    shader_cat.error()
      << "Invalid SPIR-V file: wrong magic number.\n";
    return false;
  }

  return true;
}

/**
 * Checks whether this is valid SPIR-V.
 */
bool ShaderModuleSpirV::InstructionStream::
validate() const {
  spv_context context = spvContextCreate(SPV_ENV_UNIVERSAL_1_0);
  spv_const_binary_t binary = {_words.data(), _words.size()};
  spv_diagnostic diagnostic = nullptr;

  spv_result_t result = spvValidate(context, &binary, &diagnostic);

  if (diagnostic != nullptr) {
    shader_cat.error()
      << "SPIR-V validation failed:\n" << diagnostic->error << "\n";

    disassemble(shader_cat.error() << "Disassembly follows:\n");
  }
  spvDiagnosticDestroy(diagnostic);
  spvContextDestroy(context);

  return result == SPV_SUCCESS;
}

/**
 * Writes a disassembly, for debug purposes.  Returns false if the disassembler
 * is disabled, eg. in a release build.
 */
bool ShaderModuleSpirV::InstructionStream::
disassemble(std::ostream &out) const {
#ifdef NDEBUG
  return false;
#else
  spv::Disassemble(out, _words);
  return true;
#endif
}

/**
 * Changes the locations for all inputs of the given storage class based on the
 * indicated map.  Note that this only works for inputs that already have an
 * assigned location; assign_locations() may have to be called first to ensure
 * that.
 */
void ShaderModuleSpirV::
remap_locations(spv::StorageClass storage_class, const pmap<int, int> &locations) {
  pmap<uint32_t, uint32_t *> decorations;

  for (Instruction op : _instructions) {
    if (op.opcode == spv::OpDecorate) {
      // Store the location of this decoration in the bytecode.
      if ((spv::Decoration)op.args[1] == spv::DecorationLocation && op.nargs >= 3) {
        decorations[op.args[0]] = &op.args[2];
      }
    }
    else if (op.opcode == spv::OpVariable && (spv::StorageClass)op.args[2] == storage_class) {
      // Found a variable, did we store the location for its decoration?
      pmap<uint32_t, uint32_t *>::const_iterator it = decorations.find(op.args[1]);
      if (it != decorations.end()) {
        // Yes, do we have a remapping for it?
        pmap<int, int>::const_iterator it2 = locations.find((int)*(it->second));
        if (it2 != locations.end()) {
          // Yes, write the new location into the bytecode.
          *(it->second) = (uint32_t)it2->second;
        }
      }
    }
  }
}

/**
 * Strips debugging information from the SPIR-V binary.
 */
void ShaderModuleSpirV::
strip() {
  // Create a new instruction stream, in which we copy the header for now.
  InstructionStream copy(get_data(), 5);

  // Copy all non-debug instructions to the new vector.
  for (Instruction op : _instructions) {
    if (op.opcode != spv::OpNop && !op.is_debug()) {
      copy.insert(copy.end(), op);
    }
  }

  _instructions = copy;
}

/**
 * Tells the BamReader how to create objects of type ShaderModuleSpirV.
 */
void ShaderModuleSpirV::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderModuleSpirV::
write_datagram(BamWriter *manager, Datagram &dg) {
  ShaderModule::write_datagram(manager, dg);

  dg.add_uint32(_inputs.size());
  for (const Variable &input : _inputs) {
    manager->write_pointer(dg, input.type);
    manager->write_pointer(dg, input.name);
    dg.add_uint32(input.id);
    dg.add_int32(input._location);
  }

  dg.add_uint32(_outputs.size());
  for (const Variable &output : _outputs) {
    manager->write_pointer(dg, output.type);
    manager->write_pointer(dg, output.name);
    dg.add_uint32(output.id);
    dg.add_int32(output._location);
  }

  dg.add_uint32(_parameters.size());
  for (const Variable &parameter : _parameters) {
    manager->write_pointer(dg, parameter.type);
    manager->write_pointer(dg, parameter.name);
    dg.add_uint32(parameter.id);
    dg.add_int32(parameter._location);
  }

  dg.add_uint32(_spec_constants.size());
  for (const SpecializationConstant &spec_constant : _spec_constants) {
    manager->write_pointer(dg, spec_constant.type);
    manager->write_pointer(dg, spec_constant.name);
    dg.add_uint32(spec_constant.id);
  }

  size_t num_words = _instructions.get_data_size();
  const uint32_t *words = _instructions.get_data();

  nassertv(num_words < UINT32_MAX);
  dg.add_uint32(num_words);
  for (size_t i = 0; i < num_words; ++i) {
    dg.add_uint32(words[i]);
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ShaderModule is encountered in the Bam file.  It should create the
 * ShaderModule and extract its information from the file.
 */
TypedWritable *ShaderModuleSpirV::
make_from_bam(const FactoryParams &params) {
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  Stage stage = (Stage)scan.get_uint8();
  ShaderModuleSpirV *module = new ShaderModuleSpirV(stage);
  module->fillin(scan, manager);

  return module;
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int ShaderModuleSpirV::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = ShaderModule::complete_pointers(p_list, manager);

  for (Variable &input : _inputs) {
    input.type = DCAST(ShaderType, p_list[pi++]);
    input.name = DCAST(InternalName, p_list[pi++]);
  }
  for (Variable &output : _outputs) {
    output.type = DCAST(ShaderType, p_list[pi++]);
    output.name = DCAST(InternalName, p_list[pi++]);
  }
  for (Variable &parameter : _parameters) {
    parameter.type = DCAST(ShaderType, p_list[pi++]);
    parameter.name = DCAST(InternalName, p_list[pi++]);
  }
  for (SpecializationConstant &spec_constant : _spec_constants) {
    spec_constant.type = DCAST(ShaderType, p_list[pi++]);
    spec_constant.name = DCAST(InternalName, p_list[pi++]);
  }

  return pi;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ShaderModuleSpirV.
 */
void ShaderModuleSpirV::
fillin(DatagramIterator &scan, BamReader *manager) {
  _source_filename = scan.get_string();
  _used_caps = (int)scan.get_uint64();

  uint32_t num_inputs = scan.get_uint32();
  _inputs.resize(num_inputs);
  for (uint32_t i = 0; i < num_inputs; ++i) {
    manager->read_pointer(scan); // type
    manager->read_pointer(scan); // name
    _inputs[i].id = scan.get_uint32();
    _inputs[i]._location = scan.get_int32();
  }

  uint32_t num_outputs = scan.get_uint32();
  _outputs.resize(num_outputs);
  for (uint32_t i = 0; i < num_outputs; ++i) {
    manager->read_pointer(scan); // type
    manager->read_pointer(scan); // name
    _outputs[i].id = scan.get_uint32();
    _outputs[i]._location = scan.get_int32();
  }

  uint32_t num_parameters = scan.get_uint32();
  _parameters.resize(num_parameters);
  for (uint32_t i = 0; i < num_parameters; ++i) {
    manager->read_pointer(scan); // type
    manager->read_pointer(scan); // name
    _parameters[i].id = scan.get_uint32();
    _parameters[i]._location = scan.get_int32();
  }

  uint32_t num_spec_constants = scan.get_uint32();
  _spec_constants.resize(num_spec_constants);
  for (uint32_t i = 0; i < num_spec_constants; ++i) {
    manager->read_pointer(scan); // type
    manager->read_pointer(scan); // name
    _spec_constants[i].id = scan.get_uint32();
  }

  uint32_t num_words = scan.get_uint32();
  std::vector<uint32_t> words(num_words);
  for (uint32_t i = 0; i < num_words; ++i) {
    words[i] = scan.get_uint32();
  }
  _instructions = std::move(words);
  nassertv(_instructions.validate_header());
}
