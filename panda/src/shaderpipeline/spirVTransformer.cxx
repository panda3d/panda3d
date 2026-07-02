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

/**
 * Constructs an instruction writer to operate on the given instruction stream.
 */
SpirVTransformer::
SpirVTransformer(const InstructionStream &stream) {
  _db.modify_definition(stream.get_id_bound() - 1);

  uint32_t current_function_id = 0;

  InstructionIterator begin = ((InstructionStream &)stream).begin();
  InstructionIterator end = ((InstructionStream &)stream).end();
  InstructionIterator it = begin;
  while (it != end) {
    Instruction op = *it;
    if (op.opcode != spv::OpNop &&
        op.opcode != spv::OpCapability &&
        op.opcode != spv::OpExtension &&
        op.opcode != spv::OpExtInstImport &&
        op.opcode != spv::OpMemoryModel &&
        op.opcode != spv::OpEntryPoint &&
        op.opcode != spv::OpExecutionMode &&
        op.opcode != spv::OpExecutionModeId &&
        op.opcode != spv::OpString &&
        op.opcode != spv::OpSourceExtension &&
        op.opcode != spv::OpSource &&
        op.opcode != spv::OpSourceContinued &&
        op.opcode != spv::OpName &&
        op.opcode != spv::OpMemberName &&
        op.opcode != spv::OpModuleProcessed) {
      break;
    }

    _db.parse_instruction(op.opcode, op.args, op.nargs, current_function_id);
    ++it;
  }
  _preamble = std::vector<uint32_t>((const uint32_t *)stream._words.data(), (const uint32_t *)it._words);

  begin = it;
  while (it != end) {
    Instruction op = *it;
    if (!op.is_annotation()) {
      break;
    }

    _db.parse_instruction(op.opcode, op.args, op.nargs, current_function_id);
    ++it;
  }
  _annotations = std::vector<uint32_t>(begin._words, it._words);

  begin = it;
  while (it != end) {
    Instruction op = *it;
    if (op.opcode == spv::OpFunction) {
      break;
    }

    _db.parse_instruction(op.opcode, op.args, op.nargs, current_function_id);
    ++it;
  }
  _definitions = std::vector<uint32_t>(begin._words, it._words);

  begin = it;
  while (it != end) {
    Instruction op = *it;
    _db.parse_instruction(op.opcode, op.args, op.nargs, current_function_id);
    ++it;
  }
  _functions = std::vector<uint32_t>(begin._words, it._words);
}

/**
 * Runs the given transformation pass object (which can be used only once) on
 * the module stored in the SpirVTransformer, and updates the database.
 */
void SpirVTransformer::
run(SpirVTransformPass &pass) {
  pass._db = std::move(_db);

  // Put this in before preprocess(), since it contains the ID bound
  pass._new_preamble.insert(pass._new_preamble.end(), _preamble.begin(), _preamble.begin() + 5);

  pass.preprocess();
  pass.process_preamble(_preamble);
  pass.process_annotations(_annotations);
  pass.process_definitions(_definitions);
  pass.process_functions(_functions);
  pass.postprocess();

  _preamble = std::move(pass._new_preamble);
  _annotations = std::move(pass._new_annotations);
  _definitions = std::move(pass._new_definitions);
  _functions = std::move(pass._new_functions);

  _db = std::move(pass._db);

  for (uint32_t id : pass._deleted_ids) {
    _db.modify_definition(id).clear();
  }
  for (const auto &pair : pass._deleted_members) {
    SpirVResultDatabase::Definition &def = _db.modify_definition(pair.first);

    // The set contains original member indices, so erase in descending order,
    // which leaves the positions of the members yet to be erased unaffected.
    for (auto rit = pair.second.rbegin(); rit != pair.second.rend(); ++rit) {
      nassertd(*rit < def._members.size()) continue;
      def._members.erase(def._members.begin() + *rit);
    }

#ifndef NDEBUG
    for (size_t i = 0; i < def._members.size(); ++i) {
      nassertd(def._members[i]._new_index == (int)i) break;
    }
#endif
  }
}

/**
 *
 */
ShaderModuleSpirV::InstructionStream SpirVTransformer::
get_result() const {
  InstructionStream stream(_preamble);
  stream._words.insert(stream._words.end(), _annotations.begin(), _annotations.end());
  stream._words.insert(stream._words.end(), _definitions.begin(), _definitions.end());
  stream._words.insert(stream._words.end(), _functions.begin(), _functions.end());
  return stream;
}

/**
 * Checks that the definition database, which is updated incrementally by the
 * transform passes, is consistent with a database that is freshly parsed from
 * the resulting module.  Logs an error for every mismatch and returns false
 * if any was found.  Intended for validation in the test suite.
 *
 * Names are not compared, since the database keeps qualified names (such as
 * "block.member" after running SpirVFlattenStructPass) that intentionally
 * differ from the names stored in the module.  Usage tracking is compared in
 * one direction only: the passes may conservatively consider a definition
 * used, but a variable or function parameter that the module uses must never
 * be marked unused, or SpirVRemoveUnusedVariablesPass would delete it.
 */
bool SpirVTransformer::
validate_db() const {
  SpirVTransformer fresh(get_result());
  nassertr(get_id_bound() == fresh.get_id_bound(), false);

  bool consistent = true;
  auto report = [&](uint32_t id, const char *field, int64_t db_value, int64_t fresh_value) {
    shader_cat.error()
      << "Inconsistent database after transformation: id " << id << " has "
      << field << " " << db_value << ", but " << fresh_value
      << " when parsed from the module\n";
    consistent = false;
  };

  // Decoration-derived flags that both the incremental updates and a fresh
  // parse must agree on.  Usage-tracking flags are excluded.
  static const int flag_mask =
    SpirVResultDatabase::DF_buffer_block |
    SpirVResultDatabase::DF_block |
    SpirVResultDatabase::DF_non_writable |
    SpirVResultDatabase::DF_non_readable |
    SpirVResultDatabase::DF_relaxed_precision |
    SpirVResultDatabase::DF_flat;

  // Member decorations that affect the database and should round-trip.
  static const int member_flag_mask =
    SpirVResultDatabase::DF_non_writable |
    SpirVResultDatabase::DF_non_readable |
    SpirVResultDatabase::DF_flat;

  // The instructions inserted by a pass may record the results of arithmetic
  // operations as fully-typed temporaries, whereas parse_instruction only
  // records the result type id for these, or the other way around.  Consider
  // these definitions equivalent as long as the type id matches.
  auto is_expression = [](const Definition &def) {
    return def.is_temporary() || def._dtype == 0 /* DT_none */;
  };

  for (uint32_t id = 0; id < get_id_bound(); ++id) {
    const Definition &db_def = _db.get_definition(id);
    const Definition &fresh_def = fresh._db.get_definition(id);

    if (db_def._dtype != fresh_def._dtype) {
      if (!is_expression(db_def) || !is_expression(fresh_def)) {
        report(id, "definition type", db_def._dtype, fresh_def._dtype);
      }
      else if (db_def._type_id != fresh_def._type_id) {
        report(id, "type id", db_def._type_id, fresh_def._type_id);
      }
      // The other fields are meaningless if the definition type differs.
      continue;
    }

    if (db_def._type != fresh_def._type) {
      bool equivalent = false;
      if (db_def._type == nullptr || fresh_def._type == nullptr) {
        // A fresh parse doesn't resolve the type of arithmetic results, so
        // only require the type id to match in that case.
        equivalent = (db_def._type_id == fresh_def._type_id);
      }
      else if (const ShaderType::StorageBuffer *sbuffer = fresh_def._type->as_storage_buffer()) {
        // A buffer declared the legacy way (Uniform storage class with the
        // BufferBlock decoration) is canonicalized to a StorageBuffer type
        // when parsed, but the passes work with the unwrapped struct type.
        equivalent = (sbuffer->get_contained_type() == db_def._type);
      }
      if (!equivalent) {
        shader_cat.error()
          << "Inconsistent database after transformation: id " << id << " has type ";
        if (db_def._type != nullptr) {
          shader_cat.error(false) << *db_def._type;
        } else {
          shader_cat.error(false) << "(null)";
        }
        shader_cat.error(false) << ", but ";
        if (fresh_def._type != nullptr) {
          shader_cat.error(false) << *fresh_def._type;
        } else {
          shader_cat.error(false) << "(null)";
        }
        shader_cat.error(false) << " when parsed from the module\n";
        consistent = false;
      }
    }
    if (db_def._type_id != fresh_def._type_id) {
      report(id, "type id", db_def._type_id, fresh_def._type_id);
    }
    if (db_def._location != fresh_def._location) {
      report(id, "location", db_def._location, fresh_def._location);
    }
    if (db_def._builtin != fresh_def._builtin) {
      shader_cat.error()
        << "Inconsistent database after transformation: id " << id
        << " has builtin " << spv::BuiltInToString(db_def._builtin) << ", but "
        << spv::BuiltInToString(fresh_def._builtin)
        << " when parsed from the module\n";
      consistent = false;
    }
    if ((db_def._flags & flag_mask) != (fresh_def._flags & flag_mask)) {
      report(id, "flags", db_def._flags & flag_mask, fresh_def._flags & flag_mask);
    }
    if (db_def._array_stride != fresh_def._array_stride) {
      report(id, "array stride", db_def._array_stride, fresh_def._array_stride);
    }

    if ((db_def.is_constant() || db_def.is_spec_constant()) &&
        db_def._constant != fresh_def._constant) {
      report(id, "constant value", db_def._constant, fresh_def._constant);
    }
    if (db_def.is_spec_constant() && db_def._spec_id != fresh_def._spec_id) {
      report(id, "spec id", db_def._spec_id, fresh_def._spec_id);
    }
    if ((db_def.is_variable() || db_def.is_pointer_type()) &&
        db_def._storage_class != fresh_def._storage_class &&
        // See above: parsing canonicalizes legacy-style buffer blocks to the
        // StorageBuffer storage class.
        !(db_def._storage_class == spv::StorageClassUniform &&
          fresh_def._storage_class == spv::StorageClassStorageBuffer)) {
      shader_cat.error()
        << "Inconsistent database after transformation: id " << id
        << " has storage class " << spv::StorageClassToString(db_def._storage_class)
        << ", but " << spv::StorageClassToString(fresh_def._storage_class)
        << " when parsed from the module\n";
      consistent = false;
    }

    // Usage is tracked conservatively by the passes, so a definition may be
    // marked as used even if the module no longer uses it.  The reverse is a
    // genuine problem, however: if a variable or function parameter that the
    // module relies on is considered unused, SpirVRemoveUnusedVariablesPass
    // would delete it.
    if ((db_def.is_variable() || db_def.is_function_parameter()) &&
        fresh_def.is_used() && !db_def.is_used()) {
      shader_cat.error()
        << "Inconsistent database after transformation: id " << id
        << " is not marked as used, but the module uses it\n";
      consistent = false;
    }
    if ((db_def.is_variable() || db_def.is_function_parameter() ||
         db_def.is_temporary()) &&
        db_def._function_id != fresh_def._function_id) {
      report(id, "function id", db_def._function_id, fresh_def._function_id);
    }

    if (db_def.is_function() || db_def.is_function_type()) {
      if (db_def._parameters != fresh_def._parameters) {
        report(id, "parameter count", db_def._parameters.size(), fresh_def._parameters.size());
      }
    }

    if (db_def._members.size() != fresh_def._members.size()) {
      report(id, "member count", db_def._members.size(), fresh_def._members.size());
    } else {
      for (size_t mi = 0; mi < db_def._members.size(); ++mi) {
        const MemberDefinition &db_member = db_def._members[mi];
        const MemberDefinition &fresh_member = fresh_def._members[mi];
        if (db_member._type_id != fresh_member._type_id) {
          report(id, "member type id", db_member._type_id, fresh_member._type_id);
        }
        if (db_member._offset != fresh_member._offset) {
          report(id, "member offset", db_member._offset, fresh_member._offset);
        }
        if (db_member._location != fresh_member._location) {
          report(id, "member location", db_member._location, fresh_member._location);
        }
        if (db_member._builtin != fresh_member._builtin) {
          report(id, "member builtin", db_member._builtin, fresh_member._builtin);
        }
        if ((db_member._flags & member_flag_mask) !=
            (fresh_member._flags & member_flag_mask)) {
          report(id, "member flags", db_member._flags & member_flag_mask,
                 fresh_member._flags & member_flag_mask);
        }
        if (db_member._matrix_stride != fresh_member._matrix_stride) {
          report(id, "member matrix stride", db_member._matrix_stride,
                 fresh_member._matrix_stride);
        }
      }
    }
  }

  return consistent;
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
  BitArray uniform_locations;

  for (uint32_t id = 0; id < get_id_bound(); ++id) {
    const Definition &def = _db.get_definition(id);
    if (def.is_variable()) {
      if (!def.has_location()) {
        if (!def.is_builtin() &&
            (def._storage_class == spv::StorageClassInput ||
             def._storage_class == spv::StorageClassOutput)) {
          // A non-built-in variable definition without a location.
          has_unassigned_locations = true;
        }
      }
      else if (def._storage_class == spv::StorageClassInput) {
        input_locations.set_range(def._location, def._type ? def._type->get_num_interface_locations() : 1);
      }
      else if (def._storage_class == spv::StorageClassOutput) {
        output_locations.set_range(def._location, def._type ? def._type->get_num_interface_locations() : 1);
      }
    }
  }

  if (!has_unassigned_locations) {
    return;
  }

  // Insert decorations for every unassigned variable at the beginning of the
  // annotations block.
  for (uint32_t id = 0; id < get_id_bound(); ++id) {
    Definition &def = _db.modify_definition(id);
    if (def.is_variable() && !def.has_location() && !def.is_builtin()) {
      int location;
      int num_locations;
      const char *sc_str;

      if (def._storage_class == spv::StorageClassInput) {
        num_locations = def._type->get_num_interface_locations();
        if (num_locations == 0) {
          continue;
        }

        if (stage == ShaderModule::Stage::VERTEX && !input_locations.get_bit(0) &&
            def._name != "vertex" && def._name != "p3d_Vertex" &&
            def._name != "vtx_position") {
          // Leave location 0 open for the vertex position attribute.
          location = input_locations.find_off_range(num_locations, 1);
        } else {
          location = input_locations.find_off_range(num_locations);
        }
        input_locations.set_range(location, num_locations);

        sc_str = "input";
      }
      else if (def._storage_class == spv::StorageClassOutput) {
        num_locations = def._type->get_num_interface_locations();
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
            << "Assigning " << def._name << " to " << sc_str << " location "
            << location << "\n";
        } else {
          shader_cat.debug()
            << "Assigning " << def._name << " to " << sc_str << " locations "
            << location << ".." << (location + num_locations - 1) << "\n";
        }
      }

      def._location = location;
      _annotations.insert(_annotations.end(), {spv::OpDecorate | (4 << spv::WordCountShift), id, spv::DecorationLocation, (uint32_t)location});
    }
  }
}

/**
 * Assigns location decorations based on the given remapping.
 */
void SpirVTransformer::
assign_locations(pmap<uint32_t, int> remap) {
  // Replace existing locations.
  InstructionIterator it(_annotations.data());
  InstructionIterator end(_annotations.data() + _annotations.size());
  while (it != end) {
    Instruction op = *it;

    if (op.opcode == spv::OpDecorate &&
        (spv::Decoration)op.args[1] == spv::DecorationLocation && op.nargs >= 3) {
      auto it = remap.find(op.args[0]);
      if (it != remap.end()) {
        _db.modify_definition(op.args[0])._location = it->second;
        op.args[2] = it->second;
        remap.erase(it);
      }
    }

    ++it;
  }

  // Insert decorations for every unassigned variable at the beginning of the
  // annotations block.
  for (auto it = remap.begin(); it != remap.end(); ++it) {
    _db.modify_definition(it->first)._location = it->second;
    _annotations.insert(_annotations.end(), {spv::OpDecorate | (4 << spv::WordCountShift), it->first, spv::DecorationLocation, (uint32_t)it->second});
  }
}

/**
 * Assigns missing Flat decoration on uint/double outputs.
 */
void SpirVTransformer::
assign_flat_decorations() {
  for (uint32_t id = 0; id < get_id_bound(); ++id) {
    const Definition &def = _db.get_definition(id);
    if (def.is_variable() && !def.is_builtin() && def._storage_class == spv::StorageClassOutput && (def._flags & SpirVResultDatabase::DF_flat) == 0) {
      if (def._type->contains_scalar_type(ShaderType::ST_int) ||
          def._type->contains_scalar_type(ShaderType::ST_uint) ||
          def._type->contains_scalar_type(ShaderType::ST_double)) {
        // These must be flat.
        const ShaderType *unwrapped = def._type;
        while (const ShaderType::Array *array = unwrapped->as_array()) {
          unwrapped = array->get_element_type();
        }
        //TODO: handle assigning "flat" to struct members.
        if (unwrapped->as_struct() == nullptr) {
          _annotations.insert(_annotations.end(), {spv::OpDecorate | (3 << spv::WordCountShift), id, spv::DecorationFlat});
          _db.modify_definition(id)._flags |= SpirVResultDatabase::DF_flat;
        }
      }
    }
  }
}

/**
 * Assigns procedural names consisting of a prefix followed by an index.
 */
void SpirVTransformer::
assign_procedural_names(const char *prefix, const pmap<uint32_t, int> &suffixes) {
  // Remove existing names matching theses ids.
  auto it = _preamble.begin() + 5;
  while (it != _preamble.end()) {
    spv::Op opcode = (spv::Op)(*it & spv::OpCodeMask);
    uint32_t wcount = *it >> spv::WordCountShift;
    nassertd(wcount > 0) break;

    if (opcode == spv::OpName && wcount >= 2) {
      if (suffixes.count(*(it + 1))) {
        it = _preamble.erase(it, it + wcount);
        continue;
      }
    }

    std::advance(it, wcount);
  }

  // Insert names before the annotations block.
  uint32_t *words = (uint32_t *)alloca(sizeof(uint32_t) + strlen(prefix) + 32);
  for (auto it = suffixes.begin(); it != suffixes.end(); ++it) {
    words[1] = it->first;
    int len = sprintf((char *)(words + 2), "%s%d", prefix, it->second);
    uint32_t num_words = len / 4 + 3;
    words[0] = spv::OpName | (num_words << spv::WordCountShift);
    _preamble.insert(_preamble.end(), words, words + num_words);
  }
}

/**
 * Removes location decorations from uniforms.
 */
void SpirVTransformer::
strip_uniform_locations() {
  auto it = _annotations.begin();
  while (it != _annotations.end()) {
    spv::Op opcode = (spv::Op)(*it & spv::OpCodeMask);
    uint32_t wcount = *it >> spv::WordCountShift;
    nassertd(wcount > 0) break;

    if (opcode == spv::OpDecorate && wcount >= 3 && *(it + 2) == spv::DecorationLocation) {
      Definition &def = _db.modify_definition(*(it + 1));
      if (def._storage_class == spv::StorageClassUniformConstant) {
        it = _annotations.erase(it, it + wcount);
        def._location = -1;
        continue;
      }
    }

    std::advance(it, wcount);
  }
}

/**
 * Removes all binding and descriptor set decorations.
 */
void SpirVTransformer::
strip_bindings() {
  auto it = _annotations.begin();
  while (it != _annotations.end()) {
    spv::Op opcode = (spv::Op)(*it & spv::OpCodeMask);
    uint32_t wcount = *it >> spv::WordCountShift;
    nassertd(wcount > 0) break;

    if (opcode == spv::OpDecorate && wcount >= 3) {
      spv::Decoration decoration = (spv::Decoration)*(it + 2);
      if (decoration == spv::DecorationBinding ||
          decoration == spv::DecorationDescriptorSet) {
        it = _annotations.erase(it, it + wcount);
        continue;
      }
    }

    std::advance(it, wcount);
  }
}

/**
 * Assign descriptor bindings for a descriptor set based on the given ids.
 * To create gaps in the descriptor set, entries in ids may be 0.
 */
void SpirVTransformer::
bind_descriptor_set(uint32_t set, const pvector<uint32_t> &ids) {
  InstructionIterator it(_annotations.data());
  InstructionIterator end(_annotations.data() + _annotations.size());

  BitArray assigned_sets;
  BitArray assigned_bindings;

  while (it != end) {
    Instruction op = *it;

    if (op.opcode == spv::OpDecorate && op.nargs >= 3 &&
        (op.args[1] == spv::DecorationBinding ||
         op.args[1] == spv::DecorationDescriptorSet)) {
      auto iit = std::find(ids.begin(), ids.end(), op.args[0]);
      if (iit != ids.end()) {
        uint32_t binding = std::distance(ids.begin(), iit);
        if (op.args[1] == spv::DecorationBinding) {
          op.args[2] = binding;
          assigned_bindings.set_bit(binding);
        }
        else if (op.args[1] == spv::DecorationDescriptorSet) {
          op.args[2] = set;
          assigned_sets.set_bit(binding);
        }
      }
    }

    ++it;
  }

  // Anything left?
  for (uint32_t binding = 0; binding < ids.size(); ++binding) {
    uint32_t id = ids[binding];
    if (id > 0) {
      if (!assigned_sets.get_bit(binding)) {
        _annotations.insert(_annotations.end(), {spv::OpDecorate | (4 << spv::WordCountShift), id, spv::DecorationDescriptorSet, set});
      }
      if (!assigned_bindings.get_bit(binding)) {
        _annotations.insert(_annotations.end(), {spv::OpDecorate | (4 << spv::WordCountShift), id, spv::DecorationBinding, binding});
      }
    }
  }
}
