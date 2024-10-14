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

    for (size_t i = 0; i < def._members.size();) {
      if (pair.second.count(i)) {
        def._members.erase(def._members.begin() + i);
        nassertd(def._members[i]._new_index == (int)i) continue;
      }
      ++i;
    }
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
 * Assigns location decorations to all input, output and uniform variables that
 * do not have a location decoration yet.
 */
void SpirVTransformer::
assign_locations(ShaderModule::Stage stage) {
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
             def._storage_class == spv::StorageClassOutput ||
             def._storage_class == spv::StorageClassUniformConstant)) {
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
      /*else if (def._storage_class == spv::StorageClassUniformConstant) {
        uniform_locations.set_range(def._location, def._type ? def._type->get_num_parameter_locations() : 1);
      }*/
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

        if (stage == ShaderModule::Stage::vertex && !input_locations.get_bit(0) &&
            def._name != "vertex" && def._name != "p3d_Vertex" &&
            def._name != "vtx_position") {
          // Leave location 0 open for the vertex attribute.
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
      /*else if (def._storage_class == spv::StorageClassUniformConstant) {
        num_locations = def._type->get_num_parameter_locations();
        if (num_locations == 0) {
          continue;
        }

        location = uniform_locations.find_off_range(num_locations);
        uniform_locations.set_range(location, num_locations);

        sc_str = "uniform";
      }*/
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
 * Assign descriptor bindings for a descriptor set based on the given ids.
 * Assumes there are already binding and set decorations.
 * To create gaps in the descriptor set, entries in ids may be 0.
 */
void SpirVTransformer::
bind_descriptor_set(uint32_t set, const pvector<uint32_t> &ids) {
  InstructionIterator it(_annotations.data());
  InstructionIterator end(_annotations.data() + _annotations.size());

  while (it != end) {
    Instruction op = *it;

    if (op.opcode == spv::OpDecorate && op.nargs >= 3 &&
        (op.args[1] == spv::DecorationBinding ||
         op.args[1] == spv::DecorationDescriptorSet)) {
      auto iit = std::find(ids.begin(), ids.end(), op.args[0]);
      if (iit != ids.end()) {
        if (op.args[1] == spv::DecorationBinding) {
          op.args[2] = std::distance(ids.begin(), iit);
        }
        else if (op.args[1] == spv::DecorationDescriptorSet) {
          op.args[2] = set;
        }
      }
    }

    ++it;
  }
}
