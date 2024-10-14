/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVHoistStructResourcesPass.cxx
 * @author rdb
 * @date 2024-10-08
 */

#include "spirVHoistStructResourcesPass.h"

/**
 *
 */
bool SpirVHoistStructResourcesPass::
transform_definition_op(Instruction op) {
  switch (op.opcode) {
  case spv::OpTypeImage:
  case spv::OpTypeSampler:
  case spv::OpTypeSampledImage:
  case spv::OpTypeOpaque:
  case spv::OpTypeEvent:
  case spv::OpTypeDeviceEvent:
  case spv::OpTypeReserveId:
  case spv::OpTypeQueue:
  case spv::OpTypePipe:
  case spv::OpTypeForwardPointer:
  case spv::OpTypePipeStorage:
  case spv::OpTypeNamedBarrier:
    _hoist_types.insert(op.args[0]);
    break;

  case spv::OpTypeStruct:
    if (op.nargs >= 2) {
      uint32_t type_id = op.args[0];
      Definition &def = _db.modify_definition(type_id);
      const ShaderType::Struct *struct_type;
      DCAST_INTO_R(struct_type, def._type, false);

      // Does this contain a type we should hoist?
      pvector<uint32_t> new_args({op.args[0]});
      ShaderType::Struct new_struct;
      bool changed = false;
      for (size_t i = 1; i < op.nargs; ++i) {
        auto ait = _affected_types.find(op.args[i]);
        if (ait != _affected_types.end()) {
          for (const auto &pair : ait->second) {
            AccessChain access_chain_copy(type_id, {(uint32_t)(i - 1)});
            access_chain_copy.extend(pair.second);
            _affected_types[type_id].emplace_back(pair.first, std::move(access_chain_copy));
          }
        }

        if (_hoist_types.count(op.args[i])) {
          // Start a new access chain, with the target type in the beginning.
          _affected_types[type_id].emplace_back(resolve_type(op.args[i]), AccessChain(op.args[i], {(uint32_t)(i - 1)}));
          delete_struct_member(type_id, i - 1);
          changed = true;
        }
        else if (is_deleted(op.args[i])) {
          // This nested struct/array became empty since it had only samplers.
          delete_struct_member(type_id, i - 1);
          changed = true;
        }
        else {
          const ShaderType::Struct::Member &member = struct_type->get_member(i - 1);
          new_struct.add_member(member.type, member.name, member.offset);
          new_args.push_back(op.args[i]);
        }
      }

      if (_remove_empty_structs && new_args.size() == 1 && op.nargs > 1) {
        // No members left, delete this struct (but only if it wasn't
        // already empty before this).
        delete_id(type_id);
        return false;
      }

      if (changed) {
        def._type = ShaderType::register_type(std::move(new_struct));
        add_definition(op.opcode, new_args.data(), new_args.size());
        return false;
      }
    }
    break;

  case spv::OpTypeArray:
    if (op.nargs >= 3) {
      uint32_t type_id = op.args[1];
      uint32_t size = resolve_constant(op.args[2]);
      if (_hoist_types.count(type_id)) {
        // Arrays of hoisted types are also hoisted.
        _hoist_types.insert(op.args[0]);
      }
      auto ait = _affected_types.find(type_id);
      if (ait != _affected_types.end() && size > 0) {
        // Copy over the access chains to the new type, but wrap the type
        // with the array.
        auto &access_chains = _affected_types[op.args[0]];
        for (auto &pair : ait->second) {
          access_chains.emplace_back(ShaderType::register_type(ShaderType::Array(pair.first, size)), pair.second);
        }
      }

      // If the struct contained only resources, delete the array as well.
      if (is_deleted(type_id)) {
        delete_id(op.args[0]);
        return false;
      }
    }
    break;

  case spv::OpTypePointer:
    if (op.nargs >= 3) {
      if (_affected_types.count(op.args[2])) {
        _affected_pointer_types.insert(op.args[0]);
      }
      if (is_deleted(op.args[2])) {
        delete_id(op.args[0]);
        return false;
      }
      _db.modify_definition(op.args[0])._type = resolve_type(op.args[2]);
    }
    break;

  case spv::OpTypeFunction:
    // Erase deleted types in function parameter list.
    if (op.nargs >= 2) {
      pvector<uint32_t> new_args({op.args[0], op.args[1]});

      for (size_t i = 2; i < op.nargs; ++i) {
        uint32_t arg = op.args[i];
        if (!is_deleted(arg)) {
          new_args.push_back(arg);
        }

        // Structs with non-opaque types must be passed through pointers.
        nassertd(!_affected_types.count(arg)) continue;

        if (_affected_pointer_types.count(arg)) {
          auto ait = _affected_types.find(unwrap_pointer_type(arg));
          nassertd(ait != _affected_types.end()) continue;

          // Passing a struct with non-opaque types to a function.  That
          // means adding additional parameters for the hoisted variables.
          for (auto &pair : ait->second) {
            uint32_t type_ptr_id = define_pointer_type(pair.first, spv::StorageClassUniformConstant);
            new_args.push_back(type_ptr_id);
          }
        }
      }

      // Let's go ahead and modify the definition now.
      _db.modify_definition(op.args[0])._parameters = pvector<uint32_t>(new_args.data() + 2, new_args.data() + new_args.size());

      add_definition(spv::OpTypeFunction, new_args.data(), new_args.size());
      return false;
    }
    break;

  case spv::OpVariable:
    if (op.nargs >= 3) {
      uint32_t type_id = unwrap_pointer_type(op.args[0]);
      auto ait = _affected_types.find(type_id);
      if (ait != _affected_types.end()) {
        uint32_t var_id = op.args[1];
        spv::StorageClass storage_class = (spv::StorageClass)op.args[2];

        for (const auto &pair : ait->second) {
          const ShaderType *new_type = pair.first;
          const AccessChain &access_chain = pair.second;

          uint32_t new_id = define_variable(new_type, storage_class);

          AccessChain new_access_chain(access_chain);
          new_access_chain._var_id = var_id;

          _hoisted_vars[std::move(new_access_chain)] = new_id;
          //result[var_id].push_back(new_id);
        }
      }

      // If the struct contained only samplers, delete the old variable.
      if (is_deleted(type_id)) {
        delete_id(op.args[1]);
        return false;
      }
    }
    break;

  default:
    return SpirVTransformPass::transform_definition_op(op);
  }

  return true;
}

/**
 *
 */
bool SpirVHoistStructResourcesPass::
begin_function(Instruction op) {
  // We will be re-recording the parameters.
  _db.modify_definition(op.args[1])._parameters.clear();
  return true;
}

/**
 *
 */
bool SpirVHoistStructResourcesPass::
transform_function_op(Instruction op) {
  switch (op.opcode) {
  case spv::OpFunctionParameter:
    // Erase deleted types in function parameter list.
    if (op.nargs >= 2) {
      uint32_t param_id = op.args[1];
      if (is_deleted(op.args[0])) {
        delete_id(param_id);
      } else {
        add_instruction(op.opcode, op.args, op.nargs);
        _db.modify_definition(_current_function_id)._parameters.push_back(op.args[1]);
      }

      // Structs with non-opaque types must be passed through pointers.
      nassertr(!_affected_types.count(op.args[0]), false);

      if (_affected_pointer_types.count(op.args[0])) {
        auto ait = _affected_types.find(unwrap_pointer_type(op.args[0]));
        nassertr(ait != _affected_types.end(), false);

        // Passing a struct with non-opaque types to a function.  That means
        // adding additional parameters for the hoisted variables.
        for (auto &pair : ait->second) {
          uint32_t type_ptr_id = define_pointer_type(pair.first, spv::StorageClassUniformConstant);
          uint32_t id = allocate_id();
          add_instruction(op.opcode, {type_ptr_id, id});

          AccessChain access_chain(pair.second);
          access_chain._var_id = param_id;
          _hoisted_vars[std::move(access_chain)] = id;

          _db.record_function_parameter(id, type_ptr_id, _current_function_id);
        }
      }

      return false;
    }
    break;

  case spv::OpAccessChain:
  case spv::OpInBoundsAccessChain:
    if (op.nargs >= 4) {
      // Walk through the access chain.
      uint32_t result_pointer_type_id = op.args[0];
      uint32_t result_type_id = unwrap_pointer_type(result_pointer_type_id);

      uint32_t parent_id = unwrap_pointer_type(get_type_id(op.args[2]));

      if (is_deleted(result_type_id)) {
        // Empty struct, so access chain must also be deleted.
        delete_id(op.args[1]);
        return false;
      }

      pvector<uint32_t> new_args({op.args[0], op.args[1], op.args[2]});

      if (_hoist_types.count(result_type_id)) {
        // Construct the access chain with just struct members, to figure
        // out which variable it's referring to.
        AccessChain access_chain(op.args[2]);

        for (size_t i = 3; i < op.nargs; ++i) {
          const Definition &def = _db.get_definition(parent_id);
          if (def._members.empty()) { // array
            parent_id = def._type_id;
            nassertr(parent_id > 0, false);
            new_args.push_back(op.args[i]);
          } else {
            // Must be a struct.
            uint32_t index = resolve_constant(op.args[i]);
            access_chain.append(index);

            parent_id = def._members[index]._type_id;
          }
        }

        if (access_chain.size() == 0) {
          // There are no structs involved, this is probably just an array of
          // samplers.  No need to hoist.
          return true;
        }

        auto hit = _hoisted_vars.find(access_chain);
        nassertr(hit != _hoisted_vars.end(), false);
        uint32_t new_var_id = hit->second;
        mark_used(new_var_id);

        // Change the access chain to remove the struct member indices, and
        // the base id to our variable.
        new_args[2] = new_var_id;
        add_instruction(op.opcode, new_args.data(), new_args.size());
        return false;
      }

      if (_affected_types.count(parent_id)) {
        // We may still need to remap the struct member indices.
        // It may also still be pointing to a struct containing a non-opaque
        // type, so add additional access chains for the hoisted members.
        AccessChain access_chain(op.args[2]);

        pvector<uint32_t> new_args({op.args[0], op.args[1], op.args[2]});
        pvector<uint32_t> hoisted_new_args({0, 0, 0});

        for (size_t i = 3; i < op.nargs; ++i) {
          const Definition &def = _db.get_definition(parent_id);
          if (def._type->as_array()) {
            parent_id = def._type_id;
            new_args.push_back(op.args[i]);
            hoisted_new_args.push_back(op.args[i]);
            nassertr(parent_id > 0, false);
          }
          else if (def._type->as_struct()) {
            uint32_t index = resolve_constant(op.args[i]);

            if (is_member_deleted(parent_id, index)) {
              // This one was removed, huh.  Let's hope this access chain
              // never gets loaded.  Remove it.
              delete_id(op.args[1]);
              return false;
            }

            const MemberDefinition &member_def = def._members[index];
            if (member_def._new_index != (int)index) {
              new_args.push_back(define_int_constant(member_def._new_index));
              access_chain.append(member_def._new_index);
            } else {
              new_args.push_back(op.args[i]);
              access_chain.append(index);
            }

            parent_id = def._members[index]._type_id;
          }
          else {
            // Indexing into a non-aggregate composite type (vector/matrix).
            // Just leave the rest of the access chain intact.
            new_args.insert(new_args.end(), op.args + i, op.args + op.nargs);
            break;
          }
        }

        add_instruction(op.opcode, new_args.data(), new_args.size());

        auto ait = _affected_types.find(result_type_id);
        if (ait != _affected_types.end()) {
          uint32_t orig_chain_id = op.args[1];
          for (auto &pair : ait->second) {
            AccessChain full(access_chain);
            full.extend(pair.second);

            auto hit = _hoisted_vars.find(full);
            nassertr(hit != _hoisted_vars.end(), false);
            uint32_t hoisted_var_id = hit->second;

            uint32_t hoisted_type_ptr_id = define_pointer_type(pair.first, spv::StorageClassUniformConstant);
            nassertr(hoisted_type_ptr_id != 0, false);

            uint32_t id = allocate_id();
            hoisted_new_args[0] = hoisted_type_ptr_id;
            hoisted_new_args[1] = id;
            hoisted_new_args[2] = hoisted_var_id;
            add_instruction(spv::OpAccessChain, hoisted_new_args.data(), hoisted_new_args.size());
            _db.record_temporary(id, hoisted_type_ptr_id, hoisted_var_id, _current_function_id);

            AccessChain new_access_chain(pair.second);
            new_access_chain._var_id = orig_chain_id;
            _hoisted_vars[std::move(new_access_chain)] = id;
          }
        }

        return false;
      }
    }
    break;

  case spv::OpFunctionCall:
    if (op.nargs >= 3) {
      pvector<uint32_t> new_args({op.args[0], op.args[1], op.args[2]});

      for (size_t i = 3; i < op.nargs; ++i) {
        uint32_t arg = op.args[i];
        uint32_t arg_type_id = get_type_id(arg);
        if (!is_deleted(arg_type_id)) {
          new_args.push_back(arg);
        }

        // Structs with samplers must be passed through a pointer.
        nassertd(!_affected_types.count(arg_type_id)) continue;

        if (_affected_pointer_types.count(arg_type_id)) {
          auto ait = _affected_types.find(unwrap_pointer_type(arg_type_id));
          nassertd(ait != _affected_types.end()) continue;

          // Passing a struct with non-opaque types to a function.  That means
          // adding additional parameters for the hoisted variables.
          for (auto &pair : ait->second) {
            AccessChain access_chain(pair.second);
            access_chain._var_id = arg;

            auto hit = _hoisted_vars.find(access_chain);
            nassertr(hit != _hoisted_vars.end(), false);
            uint32_t hoisted_var_id = hit->second;
            mark_used(hoisted_var_id);

            new_args.push_back(hoisted_var_id);
          }
        }
      }

      add_instruction(spv::OpFunctionCall, new_args.data(), new_args.size());
      return false;
    }
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
    // If this triggers, the struct is being loaded into another variable,
    // which means we can't unwrap this (for now).  If it turns out that
    // people actually do this, we can add support.
    nassertr(!_affected_types.count(op.args[0]), false);
    nassertr(!is_deleted(op.args[2]), false);
    mark_used(op.args[2]);
    break;

  case spv::OpCopyObject:
  case spv::OpCopyLogical:
  case spv::OpExpectKHR:
    // Not allowed to copy structs containing resources.
    nassertr(!_affected_types.count(op.args[0]), false);
    nassertr(!_affected_pointer_types.count(op.args[0]), false);

    // Copying an empty struct means deleting the copy.
    if (is_deleted(op.args[2])) {
      delete_id(op.args[1]);
      return false;
    }
    break;

  case spv::OpReturnValue:
    // Cannot return a struct with an opaque type from a function.
    if (op.nargs >= 1) {
      uint32_t value_id = op.args[0];
      uint32_t type_id = get_type_id(op.args[0]);
      mark_used(value_id);
      nassertr(!is_deleted(value_id), true);
      nassertr(!is_deleted(type_id), true);
      nassertr(!_affected_types.count(type_id), true);
      nassertr(!_affected_pointer_types.count(type_id), true);
    }
    break;

  default:
    return SpirVTransformPass::transform_function_op(op);
  }

  return true;
}

/**
 *
 */
void SpirVHoistStructResourcesPass::
postprocess() {
  for (auto vit = _hoisted_vars.begin(); vit != _hoisted_vars.end(); ++vit) {
    const auto &access_chain = vit->first;
    uint32_t var_id = vit->second;

    std::string name = _db.get_definition(access_chain._var_id)._name;
    if (!name.empty()) {
      for (size_t i = 0; i < access_chain.size(); ++i) {
        name += "_m" + format_string(access_chain[i]);
      }
      set_name(var_id, name);
    }
  }
}
