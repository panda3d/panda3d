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

//#include <glslang/SPIRV/disassemble.h>

TypeHandle ShaderModuleSpirV::_type_handle;

/**
 * Takes a stream of SPIR-V instructions, and processes it as follows:
 * - All the definitions are parsed out (requires debug info present)
 * - Makes sure that all the inputs have location indices assigned.
 * - Builds up the lists of inputs, outputs and parameters.
 * - Strips debugging information from the module.
 */
ShaderModuleSpirV::
ShaderModuleSpirV(Stage stage, const uint32_t *words, size_t size) :
  ShaderModule(stage),
  _words(words, words + size)
{
  Definitions defs;
  if (!parse(defs)) {
    shader_cat.error()
      << "Failed to parse SPIR-V shader code.\n";
    return;
  }

  // Add in location decorations for any inputs that are missing it.
  assign_locations(defs);

  // Identify the inputs and outputs.
  for (uint32_t id = 0; id < defs.size(); ++id) {
    Definition &def = defs[id];
    if (def._dtype == DT_variable && def._builtin == SpvBuiltInMax) {
      Variable var;
      var.type = def._type;
      var.name = def._name;
      var._location = def._location;
      //var._id = id;

      if (def._storage_class == SpvStorageClassInput) {
        _inputs.push_back(std::move(var));
      }
      else if (def._storage_class == SpvStorageClassOutput) {
        _outputs.push_back(std::move(var));
      }
      else if (def._storage_class == SpvStorageClassUniformConstant) {
        _parameters.push_back(std::move(var));
      }
    }
  }

  // We no longer need the debugging information, so it can be safely stripped
  // from the module.
  strip();

  //std::vector<uint32_t> words2(_words.begin(), _words.end());
  //spv::Disassemble(std::cerr, words2);
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
 * the outputs of the previous stage and assigning locations.
 */
bool ShaderModuleSpirV::
link_inputs(const ShaderModule *previous) {
  if (!previous->is_of_type(ShaderModuleSpirV::get_class_type())) {
    return false;
  }
  if (previous->get_stage() >= get_stage()) {
    return false;
  }

  pmap<int, int> location_remap;

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
      location_remap[input.get_location()] = output.get_location();
    }
  }

  if (!location_remap.empty()) {
    remap_locations(SpvStorageClassInput, location_remap);
  }
  return true;
}

/**
 * Remaps parameters with a given location to a given other location.  Locations
 * not included in the map remain untouched.
 */
void ShaderModuleSpirV::
remap_parameter_locations(pmap<int, int> &locations) {
  remap_locations(SpvStorageClassUniformConstant, locations);

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
 * Parses the SPIR-V file, extracting the definitions into the given vector.
 */
bool ShaderModuleSpirV::
parse(Definitions &defs) {
  const uint32_t *words = (const uint32_t *)_words.data();
  const size_t length = _words.size();
  const uint32_t *end = words + length;

  if (length < 5) {
    shader_cat.error()
      << "Invalid SPIR-V file: too short.\n";
    return false;
  }

  if (*words++ != SpvMagicNumber) {
    shader_cat.error()
      << "Invalid SPIR-V file: wrong magic number.\n";
    return false;
  }

  ++words; // version
  ++words; // generator
  uint32_t bound = *words++;
  ++words; // schema (reserved)

  defs = Definitions(bound);

  while (words < end) {
    uint16_t wcount = words[0] >> SpvWordCountShift;
    SpvOp opcode = (SpvOp)(words[0] & SpvOpCodeMask);
    nassertr(wcount > 0, false);

    if (!parse_instruction(defs, opcode, words + 1, wcount - 1)) {
      return false;
    }
    words += wcount;
  }

  return true;
}

/**
 * Parses the instruction with the given SPIR-V opcode and arguments.
 */
bool ShaderModuleSpirV::
parse_instruction(Definitions &defs, SpvOp opcode, const uint32_t *args, size_t nargs) {
  switch (opcode) {
  case SpvOpMemoryModel:
    if (args[0] != SpvAddressingModelLogical) {
      shader_cat.error()
        << "Invalid SPIR-V shader: addressing model Logical must be used.\n";
      return false;
    }
    if (args[1] != SpvMemoryModelGLSL450) {
      shader_cat.error()
        << "Invalid SPIR-V shader: memory model GLSL450 must be used.\n";
      return false;
    }
    break;

  case SpvOpEntryPoint:
    /*switch ((SpvExecutionModel)args[0]) {
    case SpvExecutionModelVertex:
      _stage = Stage::vertex;
      break;
    case SpvExecutionModelTessellationControl:
      _stage = Stage::tess_control;
      break;
    case SpvExecutionModelTessellationEvaluation:
      _stage = Stage::tess_evaluation;
      break;
    case SpvExecutionModelGeometry:
      _stage = Stage::geometry;
      break;
    case SpvExecutionModelFragment:
      _stage = Stage::fragment;
      break;
    default:
      break;
    }*/
    break;

  case SpvOpName:
    defs[args[0]].set_name((const char *)&args[1]);
    break;

  case SpvOpMemberName:
    defs[args[0]].set_member_name(args[1], (const char *)&args[2]);
    break;

  case SpvOpTypeVoid:
    defs[args[0]].set_type(nullptr);
    break;

  case SpvOpTypeInt:
    {
      if (args[2]) {
        defs[args[0]].set_type(ShaderType::int_type);
      } else {
        defs[args[0]].set_type(ShaderType::uint_type);
      }
    }
    break;

  case SpvOpTypeFloat:
    {
      defs[args[0]].set_type(ShaderType::float_type);
    }
    break;

  case SpvOpTypeVector:
    {
      const ShaderType *component_type = defs[args[1]]._type;
      uint32_t component_count = args[2];
      defs[args[0]].set_type(ShaderType::register_type(
        ShaderType::Vector(component_type, component_count)));
    }
    break;

  case SpvOpTypeMatrix:
    {
      const ShaderType::Vector *column_type;
      DCAST_INTO_R(column_type, defs[args[1]]._type, false);
      uint32_t column_count = args[2];
      //TODO: handle asymmetric matrices
      defs[args[0]].set_type(ShaderType::register_type(
        ShaderType::Matrix(column_type->get_base_type(), column_count, column_count)));
    }
    break;

  case SpvOpTypePointer:
    defs[args[0]].set_type_pointer((SpvStorageClass)args[1], defs[args[2]]._type);
    break;

  case SpvOpTypeImage:
    {
      Texture::TextureType texture_type;
      switch ((SpvDim)args[2]) {
      case SpvDim1D:
        if (args[4]) {
          texture_type = Texture::TT_1d_texture_array;
        } else {
          texture_type = Texture::TT_1d_texture;
        }
        break;

      case SpvDim2D:
        if (args[4]) {
          texture_type = Texture::TT_2d_texture_array;
        } else {
          texture_type = Texture::TT_2d_texture;
        }
        break;

      case SpvDim3D:
        texture_type = Texture::TT_3d_texture;
        break;

      case SpvDimCube:
        if (args[4]) {
          texture_type = Texture::TT_cube_map_array;
        } else {
          texture_type = Texture::TT_cube_map;
        }
        break;
      case SpvDimRect:
        shader_cat.error()
          << "imageRect shader inputs are not supported.\n";
        return false;

      case SpvDimBuffer:
        texture_type = Texture::TT_buffer_texture;
        break;

      case SpvDimSubpassData:
        shader_cat.error()
          << "subpassInput shader inputs are not supported.\n";
        return false;

      default:
        shader_cat.error()
          << "Unknown image dimensionality in OpTypeImage instruction.\n";
        return false;
      }

      ShaderType::Image::Access access = ShaderType::Image::Access::unknown;
      if (nargs > 8) {
        switch ((SpvAccessQualifier)args[8]) {
        case SpvAccessQualifierReadOnly:
          access = ShaderType::Image::Access::read_only;
          break;
        case SpvAccessQualifierWriteOnly:
          access = ShaderType::Image::Access::write_only;
          break;
        case SpvAccessQualifierReadWrite:
          access = ShaderType::Image::Access::read_write;
          break;
        default:
          shader_cat.error()
            << "Invalid access qualifier in OpTypeImage instruction.\n";
          break;
        }
      }

      defs[args[0]].set_type(ShaderType::register_type(
        ShaderType::Image(texture_type, access)));
    }
    break;

  case SpvOpTypeSampler:
    // A sampler that's not bound to a particular image.
    //defs[args[0]].set_type(SAT_sampler);
    break;

  case SpvOpTypeSampledImage:
    if (const ShaderType::Image *image = defs[args[1]]._type->as_image()) {
      defs[args[0]].set_type(ShaderType::register_type(
        ShaderType::SampledImage(image->get_texture_type())));
    } else {
      shader_cat.error()
        << "OpTypeSampledImage must refer to an image type!\n";
      return false;
    }
    break;

  case SpvOpTypeArray:
    if (defs[args[1]]._type != nullptr) {
      defs[args[0]].set_type(ShaderType::register_type(
        ShaderType::Array(defs[args[1]]._type, defs[args[2]]._constant)));
    }
    break;

  case SpvOpTypeStruct:
    {
      ShaderType::Struct type;
      for (size_t i = 0; i < nargs - 1; ++i) {
        type.add_member(
          defs[args[i + 1]]._type,
          InternalName::make(defs[args[0]]._member_names[i])
        );
      }
      defs[args[0]].set_type(ShaderType::register_type(std::move(type)));
    }
    break;

  case SpvOpConstant:
    defs[args[1]].set_constant(defs[args[0]]._type, args + 2, nargs - 2);
    break;

  case SpvOpVariable:
    {
      const Definition &ptr = defs[args[0]];
      if (ptr._dtype != DT_type_pointer) {
        shader_cat.error()
          << "Variable with id " << args[1] << " should use pointer type\n";
        return false;
      }
      defs[args[1]].set_variable(ptr._type, (SpvStorageClass)args[2]);
    }
    break;

  case SpvOpDecorate:
    switch ((SpvDecoration)args[1]) {
    case SpvDecorationBuiltIn:
      defs[args[0]]._builtin = (SpvBuiltIn)args[2];
      break;

    case SpvDecorationLocation:
      defs[args[0]]._location = args[2];
      break;
    }
    /*if (args[1] == SpvDecorationLocation || args[1] == SpvDecorationBinding) {
      vars[args[0]]._location = args[2];
    } else if (args[1] == SpvDecorationDescriptorSet) {
      vars[args[0]]._set = args[2];
    }*/
    break;
  }

  return true;
}

/**
 * Assigns location decorations to all input, output and uniform variables that
 * do not have a location decoration yet.
 */
void ShaderModuleSpirV::
assign_locations(Definitions &defs) {
  // Determine which locations have already been assigned.
  bool has_unassigned_locations = false;
  BitArray input_locations;
  BitArray output_locations;
  BitArray uniform_locations;

  for (const Definition &def : defs) {
    if (def._dtype == DT_variable) {
      if (def._location < 0) {
        if (def._builtin == SpvBuiltInMax) {
          // A non-built-in variable definition without a location.
          has_unassigned_locations = true;
        }
      }
      else if (def._storage_class == SpvStorageClassInput) {
        input_locations.set_bit(def._location);
      }
      else if (def._storage_class == SpvStorageClassOutput) {
        output_locations.set_bit(def._location);
      }
      else if (def._storage_class == SpvStorageClassUniformConstant) {
        uniform_locations.set_range(def._location, def._type ? def._type->get_num_parameter_locations() : 1);
      }
    }
  }

  if (!has_unassigned_locations) {
    return;
  }

  // Find the end of the annotation block, so that we know where to insert the
  // new locations.
  size_t i = 5;
  for (; i < _words.size();) {
    uint16_t wcount = _words[i] >> SpvWordCountShift;
    SpvOp opcode = (SpvOp)(_words[i] & SpvOpCodeMask);
    nassertv(wcount > 0);

    if (opcode != SpvOpCapability &&
        opcode != SpvOpExtension &&
        opcode != SpvOpExtInstImport &&
        opcode != SpvOpMemoryModel &&
        opcode != SpvOpEntryPoint &&
        opcode != SpvOpExecutionMode &&
        opcode != SpvOpString &&
        opcode != SpvOpSourceExtension &&
        opcode != SpvOpSource &&
        opcode != SpvOpSourceContinued &&
        opcode != SpvOpName &&
        opcode != SpvOpMemberName &&
        opcode != SpvOpModuleProcessed &&
        opcode != SpvOpDecorate &&
        opcode != SpvOpMemberDecorate &&
        opcode != SpvOpGroupDecorate &&
        opcode != SpvOpGroupMemberDecorate &&
        opcode != SpvOpDecorationGroup) {
      break;
    }

    i += wcount;
  }

  // Now insert decorations for every unassigned variable.
  for (uint32_t id = 0; id < defs.size(); ++id) {
    Definition &def = defs[id];
    if (def._dtype == DT_variable &&
        def._location < 0 &&
        def._builtin == SpvBuiltInMax) {
      int location;
      if (def._storage_class == SpvStorageClassInput) {
        if (get_stage() == Stage::vertex && !input_locations.get_bit(0)) {
          if (def._name == "vertex" || def._name == "p3d_Vertex" ||
              def._name == "vtx_position") {
            // Prefer assigning the vertex column to location 0.
            location = 0;
          } else if (!input_locations.get_bit(1)) {
            location = 1;
          } else {
            location = input_locations.get_next_higher_different_bit(1);
          }
        } else {
          location = input_locations.get_lowest_off_bit();
        }
        input_locations.set_bit(location);

        if (shader_cat.is_debug()) {
          shader_cat.debug()
            << "Assigning " << def._name << " to input location " << location << "\n";
        }
      }
      else if (def._storage_class == SpvStorageClassOutput) {
        location = output_locations.get_lowest_off_bit();
        output_locations.set_bit(location);

        if (shader_cat.is_debug()) {
          shader_cat.debug()
            << "Assigning " << def._name << " to output location " << location << "\n";
        }
      }
      else if (def._storage_class == SpvStorageClassUniformConstant) {
        int num_locations = def._type->get_num_parameter_locations();
        location = uniform_locations.get_lowest_off_bit();
        while (num_locations > 1 && uniform_locations.has_any_of(location, num_locations)) {
          // Not enough bits free, try the next open range.
          int next_bit = uniform_locations.get_next_higher_different_bit(location);
          assert(next_bit > location);
          location = uniform_locations.get_next_higher_different_bit(next_bit);
          assert(location >= 0);
        }
        uniform_locations.set_bit(location);

        if (shader_cat.is_debug()) {
          if (num_locations == 1) {
            shader_cat.debug()
              << "Assigning " << def._name << " to uniform location " << location << "\n";
          } else {
            shader_cat.debug()
              << "Assigning " << def._name << " to uniform locations " << location
              << ".." << (location + num_locations - 1) << "\n";
          }
        }
      }
      else {
        continue;
      }

      def._location = location;
      _words.insert(_words.begin() + i, {
        (4 << SpvWordCountShift) | SpvOpDecorate,
        id,
        SpvDecorationLocation,
        (uint32_t)location,
      });
      i += 4;
    }
  }
}

/**
 * Changes the locations for all inputs of the given storage class based on the
 * indicated map.  Note that this only works for inputs that already have an
 * assigned location; assign_locations() may have to be called first to ensure
 * that.
 */
void ShaderModuleSpirV::
remap_locations(SpvStorageClass storage_class, const pmap<int, int> &locations) {
  uint32_t *words = (uint32_t *)_words.data();
  const size_t length = _words.size();
  const uint32_t *end = words + length;

  pmap<uint32_t, uint32_t *> decorations;

  // Skip header
  words += 5;

  while (words < end) {
    uint16_t wcount = words[0] >> SpvWordCountShift;
    SpvOp opcode = (SpvOp)(words[0] & SpvOpCodeMask);
    nassertv(wcount > 0);

    if (opcode == SpvOpDecorate) {
      // Store the location of this decoration in the bytecode.
      if ((SpvDecoration)words[2] == SpvDecorationLocation && wcount >= 4) {
        decorations[words[1]] = &words[3];
      }
    }
    else if (opcode == SpvOpVariable && (SpvStorageClass)words[3] == storage_class) {
      // Found a variable, did we store the location for its decoration?
      pmap<uint32_t, uint32_t *>::const_iterator it = decorations.find(words[2]);
      if (it != decorations.end()) {
        // Yes, do we have a remapping for it?
        pmap<int, int>::const_iterator it2 = locations.find((int)*(it->second));
        if (it2 != locations.end()) {
          // Yes, write the new location into the bytecode.
          *(it->second) = (uint32_t)it2->second;
        }
      }
    }

    words += wcount;
  }
}

/**
 * Strips debugging information from the SPIR-V binary.
 */
void ShaderModuleSpirV::
strip() {
  pvector<uint32_t> old_words;
  old_words.swap(_words);

  const uint32_t *words = (const uint32_t *)old_words.data();
  const size_t length = old_words.size();
  const uint32_t *end = words + length;

  // Copy header.
  _words.insert(_words.end(), words, words + 5);
  words += 5;

  // Copy all non-debug instructions to the new vector.
  while (words < end) {
    uint16_t wcount = words[0] >> SpvWordCountShift;
    SpvOp opcode = (SpvOp)(words[0] & SpvOpCodeMask);
    nassertv(wcount > 0);

    if (opcode != SpvOpSourceContinued &&
        opcode != SpvOpSource &&
        opcode != SpvOpSourceExtension &&
        opcode != SpvOpName &&
        opcode != SpvOpMemberName &&
        opcode != SpvOpString &&
        opcode != SpvOpLine &&
        opcode != SpvOpNoLine &&
        opcode != SpvOpModuleProcessed) {

      _words.insert(_words.end(), words, words + wcount);
    }
    words += wcount;
  }
}

/**
 * Called when an OpName is encountered in the SPIR-V instruction stream.
 */
void ShaderModuleSpirV::Definition::
set_name(const char *name) {
  _name.assign(name);
}

/**
 * Called when an OpMemberName is encountered in the SPIR-V instruction stream.
 */
void ShaderModuleSpirV::Definition::
set_member_name(uint32_t i, const char *name) {
  if (i >= _member_names.size()) {
    _member_names.resize(i + 1);
  }
  _member_names[i].assign(name);
}

/**
 * Called when an OpType is encountered in the SPIR-V instruction stream.
 */
void ShaderModuleSpirV::Definition::
set_type(const ShaderType *type) {
  _dtype = DT_type;
  _type = type;

  if (shader_cat.is_spam()) {
    if (type != nullptr) {
      shader_cat.spam()
        << "Defined type " << *type << "\n";
    } else {
      shader_cat.spam()
        << "Defined type void\n";
    }
  }
}

/**
 * Called when an OpTypePointer is encountered in the SPIR-V instruction stream.
 */
void ShaderModuleSpirV::Definition::
set_type_pointer(SpvStorageClass storage_class, const ShaderType *type) {
  _dtype = DT_type_pointer;
  _type = type;
}

/**
 * Called when an OpVariable is encountered in the SPIR-V instruction stream.
 */
void ShaderModuleSpirV::Definition::
set_variable(const ShaderType *type, SpvStorageClass storage_class) {
  _dtype = DT_variable;
  _type = type;
  _storage_class = storage_class;

  if (shader_cat.is_debug() && storage_class == SpvStorageClassUniformConstant) {
    shader_cat.debug()
      << "Defined uniform " << _name;

    if (_location >= 0) {
      shader_cat.debug(false) << " (location " << _location << ")";
    }

    shader_cat.debug(false) << " with ";

    if (type != nullptr) {
      shader_cat.debug(false) << "type " << *type << "\n";
    } else {
      shader_cat.debug(false) << "unknown type\n";
    }
  }

  switch (storage_class) {
  case SpvStorageClassUniformConstant:
    //_uniform_constants.push_back(&def);
    break;
  case SpvStorageClassInput:
    //_inputs.push_back(Varying({}));
    break;
  case SpvStorageClassUniform:
    break;
  case SpvStorageClassOutput:
    //_outputs.push_back(&def);
    break;
  case SpvStorageClassWorkgroup:
  case SpvStorageClassCrossWorkgroup:
  case SpvStorageClassPrivate:
  case SpvStorageClassFunction:
  case SpvStorageClassGeneric:
  case SpvStorageClassPushConstant:
  case SpvStorageClassAtomicCounter:
  case SpvStorageClassImage:
    break;
  }
}

/**
 * Called when an OpConstant is encountered in the SPIR-V instruction stream.
 */
void ShaderModuleSpirV::Definition::
set_constant(const ShaderType *type, const uint32_t *words, uint32_t nwords) {
  _dtype = DT_constant;
  _type = type;
  if (nwords > 0) {
    _constant = words[0];
  } else {
    _constant = 0;
  }
}
