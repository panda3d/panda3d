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
SpirVTransformer(InstructionStream &stream) {
  _db.modify_definition(stream.get_id_bound() - 1);

  uint32_t current_function_id = 0;

  InstructionIterator begin = stream.begin();
  InstructionIterator it = begin;
  while (it != stream.end()) {
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
  _preamble = std::vector<uint32_t>(stream._words.data(), it._words);

  begin = it;
  while (it != stream.end()) {
    Instruction op = *it;
    if (!op.is_annotation()) {
      break;
    }

    _db.parse_instruction(op.opcode, op.args, op.nargs, current_function_id);
    ++it;
  }
  _annotations = std::vector<uint32_t>(begin._words, it._words);

  begin = it;
  while (it != stream.end()) {
    Instruction op = *it;
    if (op.opcode == spv::OpFunction) {
      break;
    }

    _db.parse_instruction(op.opcode, op.args, op.nargs, current_function_id);
    ++it;
  }
  _definitions = std::vector<uint32_t>(begin._words, it._words);

  begin = it;
  while (it != stream.end()) {
    Instruction op = *it;
    _db.parse_instruction(op.opcode, op.args, op.nargs, current_function_id);
    ++it;
  }
  _functions = std::vector<uint32_t>(begin._words, it._words);
}

/**
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
        nassertd(def._members[i]._new_index == i) continue;
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
 * Assign descriptor bindings for a descriptor set based on the given locations.
 * Assumes there are already binding and set decorations.
 * To create gaps in the descriptor set, entries in locations may be -1.
 */
void SpirVTransformer::
bind_descriptor_set(uint32_t set, const vector_int &locations) {
  InstructionIterator it(_annotations.data());
  InstructionIterator end(_annotations.data() + _annotations.size());

  while (it != end) {
    Instruction op = *it;

    if (op.opcode == spv::OpDecorate && op.nargs >= 3) {
      const Definition &def = _db.get_definition(op.args[0]);

      auto lit = std::find(locations.begin(), locations.end(), def._location);
      if (lit != locations.end() && def.has_location()) {
        if (op.args[1] == spv::DecorationBinding) {
          op.args[2] = std::distance(locations.begin(), lit);
        }
        else if (op.args[1] == spv::DecorationDescriptorSet) {
          op.args[2] = set;
        }
      }
    }

    ++it;
  }
}

/**
 * Creates a new uniform block using the parameters specified by the given
 * locations and types.  The opposite of flatten_struct, if you will.
 */
/*uint32_t SpirVTransformer::
make_block(const ShaderType::Struct *block_type, const pvector<int> &member_locations,
           spv::StorageClass storage_class, uint32_t binding, uint32_t set) {
  nassertr(block_type->get_num_members() == member_locations.size(), false);

  // Define block struct variable, which will implicitly define its type.
  uint32_t block_var_id = define_variable(block_type, storage_class);
  uint32_t block_type_id = _type_map[block_type];
  nassertr(block_type_id != 0, 0);

  // Collect type pointers that we have to create.
  pvector<uint32_t> insert_pointer_types;

  // Find the variables we should replace with members of this block by looking
  // at the locations.  Collect a map of defined type pointers while we're at
  // it, so we don't unnecessarily duplicate them.
  pmap<uint32_t, uint32_t> member_indices;
  pmap<uint32_t, uint32_t> pointer_type_map;

  for (uint32_t id = 0; id < _defs.size(); ++id) {
    Definition &def = _defs[id];
    if (def.is_pointer_type()) {
      if (!def.has_builtin() && def._storage_class == storage_class) {
        // This is the storage class we need, store it in case we need it.
        pointer_type_map[def._type_id] = id;
      }
    }
    else if (def.is_variable() && def.has_location() &&
             def._storage_class == spv::StorageClassUniformConstant) {

      auto lit = std::find(member_locations.begin(), member_locations.end(), def._location);
      if (lit != member_locations.end()) {
        member_indices[id] = std::distance(member_locations.begin(), lit);
      }
    }
  }

  uint32_t num_members = member_locations.size();
  uint32_t *allocation = (uint32_t *)alloca(num_members * sizeof(uint32_t) * 2);
  memset(allocation, 0, num_members * sizeof(uint32_t) * 2);

  uint32_t *member_type_ids = allocation;
  uint32_t *member_constant_ids = allocation + num_members;

  // Now add the decorations for the uniform block itself.
  InstructionIterator it = _instructions.end_annotations();
  it = _instructions.insert(it, spv::OpDecorate, {block_type_id, spv::DecorationBlock});
  ++it;

  if (storage_class != spv::StorageClassPushConstant) {
    it = _instructions.insert(it, spv::OpDecorate, {block_var_id, spv::DecorationBinding, binding});
    ++it;
    it = _instructions.insert(it, spv::OpDecorate, {block_var_id, spv::DecorationDescriptorSet, set});
    ++it;
  }

  it = _instructions.begin();
  while (it != _instructions.end()) {
    Instruction op = *it;

    switch (op.opcode) {
    case spv::OpName:
      // Translate an OpName to an OpMemberName for vars that become struct
      // members.  We could just strip them, but this is useful for debugging.
      if (member_indices.count(op.args[0])) {
        uint32_t member_index = member_indices[op.args[0]];

        uint32_t nargs = op.nargs + 1;
        uint32_t *args = (uint32_t *)alloca(nargs * sizeof(uint32_t));
        args[0] = block_type_id;
        args[1] = member_index;
        memcpy(args + 2, op.args + 1, (op.nargs - 1) * sizeof(uint32_t));

        it = _instructions.insert(it, spv::OpMemberName, args, nargs);
        ++it;
        it = _instructions.erase(it);
        continue;
      }
      break;

    case spv::OpMemberName:
    case spv::OpDecorate:
    case spv::OpDecorateId:
    case spv::OpDecorateString:
    case spv::OpMemberDecorate:
    case spv::OpMemberDecorateString:
      // Remove other annotations on the members.
      if (op.nargs >= 1 && member_indices.count(op.args[0])) {
        it = _instructions.erase(it);
        continue;
      }
      break;

    case spv::OpConstant:
      // Store integer constants that are already defined in the file that may
      // be useful for defining our struct indices.
      if (op.args[2] < num_members &&
          (_defs[op.args[0]]._type == ShaderType::int_type ||
           _defs[op.args[0]]._type == ShaderType::uint_type)) {
        member_constant_ids[op.args[2]] = op.args[1];
      }
      break;

    case spv::OpVariable:
      if (member_indices.count(op.args[1])) {
        // Remove this variable.  We'll replace it with an access chain later.
        uint32_t pointer_type_id = op.args[0];
        uint32_t member_id = op.args[1];
        uint32_t member_index = member_indices[member_id];

        if (_defs[pointer_type_id]._storage_class != storage_class) {
          // Get or create a type pointer with the correct storage class.
          uint32_t type_id = _defs[pointer_type_id]._type_id;
          auto tpi = pointer_type_map.find(type_id);
          if (tpi != pointer_type_map.end()) {
            pointer_type_id = tpi->second;
          } else {
            pointer_type_id = _instructions.allocate_id();
            pointer_type_map[type_id] = pointer_type_id;
            record_pointer_type(pointer_type_id, storage_class, type_id);

            it = _instructions.insert(it, spv::OpTypePointer,
              {pointer_type_id, (uint32_t)storage_class, type_id});
            ++it;
          }
        }

        member_type_ids[member_index] = pointer_type_id;

        it = _instructions.erase(it);
        continue;
      }
      break;

    case spv::OpFunction:
      // Before we get to the function section, make sure that all the
      // remaining constants we need are defined.
      for (uint32_t i =  0; i < num_members; ++i) {
        uint32_t constant_id = member_constant_ids[i];
        if (constant_id == 0) {
          // Doesn't matter whether we pick uint or int, prefer whatever is
          // already defined.
          const ShaderType *type =
            _type_map.count(ShaderType::uint_type)
              ? ShaderType::uint_type
              : ShaderType::int_type;
          constant_id = r_define_constant(it, type, i);
          member_constant_ids[i] = constant_id;
        }
      }
      break;

    case spv::OpAccessChain:
    case spv::OpInBoundsAccessChain:
      if (member_indices.count(op.args[2])) {
        uint32_t member_index = member_indices[op.args[2]];
        uint32_t constant_id = member_constant_ids[member_index];

        // Get or create a type pointer with the correct storage class.
        uint32_t type_id = _defs[op.args[0]]._type_id;
        auto tpi = pointer_type_map.find(type_id);
        uint32_t pointer_type_id;
        if (tpi != pointer_type_map.end()) {
          pointer_type_id = tpi->second;
        } else {
          pointer_type_id = _instructions.allocate_id();
          pointer_type_map[type_id] = pointer_type_id;
          record_pointer_type(pointer_type_id, storage_class, type_id);

          // Can't create the type pointer immediately, since we're no longer
          // in the type declaration block.  We'll add it at the end.
          insert_pointer_types.push_back(pointer_type_id);
        }
        op.args[0] = pointer_type_id;

        // Prepend our new block variable to the existing access chain.
        op.args[2] = block_var_id;
        it = _instructions.insert_arg(it, 3, constant_id);
      }
      break;

    case spv::OpImageTexelPointer:
    case spv::OpLoad:
    case spv::OpCopyObject:
      // Add access chains before all loads to access the right block member.
      if (member_indices.count(op.args[2])) {
        uint32_t member_index = member_indices[op.args[2]];
        uint32_t type_id = member_type_ids[member_index];
        uint32_t constant_id = member_constant_ids[member_index];
        uint32_t chain_id = _instructions.allocate_id();

        op.args[2] = chain_id;
        it = _instructions.insert(it, spv::OpInBoundsAccessChain,
          {type_id, chain_id, block_var_id, constant_id});
        ++it;
      }
      break;

    case spv::OpCopyMemory:
    case spv::OpCopyMemorySized:
      // Same as above, but these take the pointer in a different argument.
      if (member_indices.count(op.args[1])) {
        uint32_t member_index = member_indices[op.args[1]];
        uint32_t type_id = member_type_ids[member_index];
        uint32_t constant_id = member_constant_ids[member_index];
        uint32_t chain_id = _instructions.allocate_id();

        op.args[1] = chain_id;
        it = _instructions.insert(it, spv::OpInBoundsAccessChain,
          {type_id, chain_id, block_var_id, constant_id});
        ++it;
      }
      break;

    default:
      break;
    }

    ++it;
  }

  it = _instructions.begin_functions();

  // Insert all the type pointers for the access chains.
  for (uint32_t id : insert_pointer_types) {
    it = _instructions.insert(it, spv::OpTypePointer,
      {id, (uint32_t)_defs[id]._storage_class, _defs[id]._type_id});
    ++it;
  }

  return block_var_id;
}*/
