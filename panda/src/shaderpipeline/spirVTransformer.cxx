/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVTransformer.cxx
 * @author rdb
 * @date 2024-10-08
 */

#include "spirVTransformer.h"
#include "spirVTransformPass.h"
#include "config_shaderpipeline.h"

/**
 * Materializes the given instruction stream.
 */
SpirVTransformer::
SpirVTransformer(const InstructionStream &stream) : _module(stream) {
  if (shader_paranoid_validation && !_module.validate()) {
    Notify::assert_failure("SPIR-V module failed validation before running "
                           "transformations", __LINE__, __FILE__);
  }
}

/**
 * Runs the given transformation pass object (which can be used only once) on
 * the module.  Afterwards, merges any duplicate unique-type declarations the
 * pass may have created and, if shader-paranoid-validation is set, validates
 * the resulting module.
 */
void SpirVTransformer::
run(SpirVTransformPass &pass) {
  // A pass object carries per-run state and may only be used once.
  nassertv(!pass._ran);
  pass._ran = true;

  pass.run(_module);

  _module.deduplicate_types();

  if (shader_paranoid_validation && !_module.validate()) {
    std::string message("SPIR-V module failed validation after running ");
    message += pass.get_name();
    Notify::assert_failure(message, __LINE__, __FILE__);
  }
}

/**
 * Serializes the current state of the module.
 */
ShaderModuleSpirV::InstructionStream SpirVTransformer::
get_result() const {
  return _module.emit();
}

/**
 * Assigns location decorations to all input and output variables that do not
 * have a location decoration yet.  Does not touch uniform constants.
 */
void SpirVTransformer::
assign_interface_locations(ShaderModule::Stage stage) {
  // Determine which locations have already been assigned.
  bool has_unassigned_locations = false;
  BitArray input_locations;
  BitArray output_locations;

  for (uint32_t word = 0; word < get_id_bound(); ++word) {
    Id id(word);
    if (_module.get_definition_type(id) != SpirVModule::DT_variable ||
        _module.get_function_id(id) != 0) {
      continue;
    }
    spv::StorageClass storage_class = _module.get_storage_class(id);
    int location = _module.get_location(id);
    if (location < 0) {
      if (_module.get_builtin(id) == spv::BuiltInMax &&
          (storage_class == spv::StorageClassInput ||
           storage_class == spv::StorageClassOutput)) {
        // A non-built-in variable definition without a location.
        has_unassigned_locations = true;
      }
    }
    else if (storage_class == spv::StorageClassInput) {
      const ShaderType *type = _module.resolve_type(id);
      input_locations.set_range(location, type != nullptr ? type->get_num_interface_locations() : 1);
    }
    else if (storage_class == spv::StorageClassOutput) {
      const ShaderType *type = _module.resolve_type(id);
      output_locations.set_range(location, type != nullptr ? type->get_num_interface_locations() : 1);
    }
  }

  if (!has_unassigned_locations) {
    return;
  }

  for (uint32_t word = 0; word < get_id_bound(); ++word) {
    Id id(word);
    if (_module.get_definition_type(id) != SpirVModule::DT_variable ||
        _module.get_function_id(id) != 0 ||
        _module.get_location(id) >= 0 ||
        _module.get_builtin(id) != spv::BuiltInMax) {
      continue;
    }
    const ShaderType *type = _module.resolve_type(id);
    if (type == nullptr) {
      continue;
    }
    spv::StorageClass storage_class = _module.get_storage_class(id);

    int location;
    int num_locations;
    const char *sc_str;

    if (storage_class == spv::StorageClassInput) {
      num_locations = type->get_num_interface_locations();
      if (num_locations == 0) {
        continue;
      }

      const std::string &name = _module.get_name(id);
      if (stage == ShaderModule::Stage::VERTEX && !input_locations.get_bit(0) &&
          name != "vertex" && name != "p3d_Vertex" &&
          name != "vtx_position") {
        // Leave location 0 open for the vertex position attribute.
        location = input_locations.find_off_range(num_locations, 1);
      } else {
        location = input_locations.find_off_range(num_locations);
      }
      input_locations.set_range(location, num_locations);

      sc_str = "input";
    }
    else if (storage_class == spv::StorageClassOutput) {
      num_locations = type->get_num_interface_locations();
      if (num_locations == 0) {
        continue;
      }

      location = output_locations.find_off_range(num_locations);
      output_locations.set_range(location, num_locations);

      sc_str = "output";
    }
    else {
      continue;
    }
    nassertd(location >= 0) continue;

    if (shader_cat.is_debug()) {
      if (num_locations == 1) {
        shader_cat.debug()
          << "Assigning " << _module.get_name(id) << " to " << sc_str
          << " location " << location << "\n";
      } else {
        shader_cat.debug()
          << "Assigning " << _module.get_name(id) << " to " << sc_str
          << " locations " << location << ".."
          << (location + num_locations - 1) << "\n";
      }
    }

    _module.decorate(id, spv::DecorationLocation, (uint32_t)location);
  }
}

/**
 * Assigns location decorations based on the given remapping.
 */
void SpirVTransformer::
assign_locations(pmap<uint32_t, int> remap) {
  for (const auto &item : remap) {
    _module.set_location(Id(item.first), item.second);
  }
}

/**
 * Assigns missing Flat decoration on uint/double outputs.
 */
void SpirVTransformer::
assign_flat_decorations() {
  for (uint32_t word = 0; word < get_id_bound(); ++word) {
    Id id(word);
    if (_module.get_definition_type(id) != SpirVModule::DT_variable ||
        _module.get_function_id(id) != 0 ||
        _module.get_builtin(id) != spv::BuiltInMax ||
        _module.get_storage_class(id) != spv::StorageClassOutput ||
        _module.has_decoration(id, spv::DecorationFlat)) {
      continue;
    }
    const ShaderType *type = _module.resolve_type(id);
    if (type == nullptr) {
      continue;
    }
    if (type->contains_scalar_type(ShaderType::ST_int) ||
        type->contains_scalar_type(ShaderType::ST_uint) ||
        type->contains_scalar_type(ShaderType::ST_double)) {
      // These must be flat.
      const ShaderType *unwrapped = type;
      while (const ShaderType::Array *array = unwrapped->as_array()) {
        unwrapped = array->get_element_type();
      }
      //TODO: handle assigning "flat" to struct members.
      if (unwrapped->as_struct() == nullptr) {
        _module.decorate(id, spv::DecorationFlat);
      }
    }
  }
}

/**
 * Assigns procedural names consisting of a prefix followed by an index.
 */
void SpirVTransformer::
assign_procedural_names(const char *prefix, const pmap<uint32_t, int> &suffixes) {
  char buffer[32];
  for (const auto &item : suffixes) {
    sprintf(buffer, "%d", item.second);
    _module.set_name(Id(item.first), std::string(prefix) + buffer);
  }
}

/**
 * Removes location decorations from uniforms.
 */
void SpirVTransformer::
strip_uniform_locations() {
  for (uint32_t word = 0; word < get_id_bound(); ++word) {
    Id id(word);
    if (_module.get_definition_type(id) == SpirVModule::DT_variable &&
        _module.get_function_id(id) == 0 &&
        _module.get_storage_class(id) == spv::StorageClassUniformConstant) {
      _module.remove_decoration(id, spv::DecorationLocation);
    }
  }
}

/**
 * Removes all binding and descriptor set decorations.
 */
void SpirVTransformer::
strip_bindings() {
  _module.strip_decoration(spv::DecorationBinding);
  _module.strip_decoration(spv::DecorationDescriptorSet);
}

/**
 * Assign descriptor bindings for a descriptor set based on the given ids.
 * To create gaps in the descriptor set, entries in ids may be 0.
 */
void SpirVTransformer::
bind_descriptor_set(uint32_t set, const pvector<uint32_t> &ids) {
  for (uint32_t binding = 0; binding < ids.size(); ++binding) {
    Id id(ids[binding]);
    if (id != 0) {
      _module.set_decoration(id, spv::DecorationDescriptorSet, set);
      _module.set_decoration(id, spv::DecorationBinding, binding);
    }
  }
}
