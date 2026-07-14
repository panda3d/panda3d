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
#include "spirVInstructionCursor.h"
#include "string_utils.h"

/**
 *
 */
void SpirVHoistStructResourcesPass::
run(SpirVModule &module) {
  process_declarations(module);

  for (Function &function : module.modify_functions()) {
    process_function(module, function);
  }

  // Assign names to the hoisted variables, before the deletions below erase
  // the names of the variables they were hoisted out of.
  for (const auto &item : _hoisted_vars) {
    const AccessChain &access_chain = item.first;
    Id var_id = item.second;

    std::string name = module.get_name(access_chain._var_id);
    if (!name.empty()) {
      for (size_t i = 0; i < access_chain.size(); ++i) {
        name += "_m" + format_string(access_chain[i]);
      }
      module.set_name(var_id, name);
    }
  }

  // Now apply the deferred deletions.
  for (Id id : _deleted) {
    module.delete_id(id);
  }
}

/**
 * Returns the original (pre-deletion) type id of the given member of the
 * given struct, which may differ from the module's current members.
 */
SpirVId SpirVHoistStructResourcesPass::
get_original_member_type_id(SpirVModule &module, Id struct_id, uint32_t index) const {
  auto it = _original_members.find(struct_id);
  if (it != _original_members.end()) {
    nassertr(index < it->second.size(), Id());
    return it->second[index];
  }
  return module.get_member_type_id(struct_id, index);
}

/**
 * Walks the declarations section (in declaration order, so that a type is
 * processed before its users), collecting the types to hoist, rewriting the
 * affected struct, function type and variable declarations, and creating the
 * hoisted variables.
 */
void SpirVHoistStructResourcesPass::
process_declarations(SpirVModule &module) {
  // Walk only the declarations present now; the pointer types and hoisted
  // variables appended during the walk get higher indices and are not
  // visited.
  size_t num_declarations = module.get_num_declarations();
  for (size_t i = 0; i < num_declarations; ++i) {
    // Make a copy so that this remains safe with the mutations done below.
    Instruction op = module.get_declaration(i);

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
      _hoist_types.insert(Id(op.args[0]));
      break;

    case spv::OpTypeStruct:
      if (op.args.size() >= 2) {
        Id type_id(op.args[0]);

        // Keep the original member type ids around for the function body
        // rewrite, which needs to resolve pre-deletion member indices.
        pvector<Id> original_members;
        original_members.reserve(op.args.size() - 1);
        for (size_t ai = 1; ai < op.args.size(); ++ai) {
          original_members.push_back(Id(op.args[ai]));
        }
        pvector<int> remap(op.args.size() - 1, -1);

        // Does this contain a type we should hoist?
        bool changed = false;
        uint32_t num_kept = 0;
        for (size_t ai = 1; ai < op.args.size(); ++ai) {
          uint32_t member_index = (uint32_t)(ai - 1);
          Id member_type_id(op.args[ai]);

          auto ait = _affected_types.find(member_type_id);
          if (ait != _affected_types.end()) {
            for (const auto &pair : ait->second) {
              AccessChain access_chain_copy(type_id, {member_index});
              access_chain_copy.extend(pair.second);
              _affected_types[type_id].emplace_back(pair.first, std::move(access_chain_copy));
            }
          }

          if (_hoist_types.count(member_type_id)) {
            // Start a new access chain, with the target type in the beginning.
            _affected_types[type_id].emplace_back(
              module.resolve_type(member_type_id),
              AccessChain(member_type_id, {member_index}));
            module.delete_struct_member(type_id, num_kept);
            changed = true;
          }
          else if (is_deleted(member_type_id)) {
            // This nested struct/array became empty since it had only samplers.
            module.delete_struct_member(type_id, num_kept);
            changed = true;
          }
          else {
            remap[member_index] = (int)num_kept;
            ++num_kept;
          }
        }

        if (!changed) {
          break;
        }

        _original_members[type_id] = std::move(original_members);
        _member_remap[type_id] = std::move(remap);

        if (num_kept == 0) {
          // No members left (and the struct wasn't already empty before this).
          if (_remove_empty_structs) {
            _deleted.insert(type_id);
            break;
          }
          // Keep the (now empty) struct declaration itself, since it may be a
          // member of an enclosing struct, where removing it would change the
          // member numbering.  Nothing can usefully be accessed through it
          // anymore, however, so pointers, variables and function parameters
          // of it are removed below.
          _emptied_structs.insert(type_id);
        }
      }
      break;

    case spv::OpTypeArray:
      if (op.args.size() >= 3) {
        Id elem_type_id(op.args[1]);
        uint32_t size = module.resolve_constant(Id(op.args[2]));
        if (_hoist_types.count(elem_type_id)) {
          // Arrays of hoisted types are also hoisted.
          _hoist_types.insert(Id(op.args[0]));
        }
        auto ait = _affected_types.find(elem_type_id);
        if (ait != _affected_types.end() && size > 0) {
          // Copy over the access chains to the new type, but wrap the type
          // with the array.
          auto &access_chains = _affected_types[Id(op.args[0])];
          for (auto &pair : ait->second) {
            access_chains.emplace_back(ShaderType::register_type(ShaderType::Array(pair.first, size)), pair.second);
          }
        }

        // If the struct contained only resources, delete the array as well.
        if (is_deleted(elem_type_id)) {
          _deleted.insert(Id(op.args[0]));
          break;
        }
        if (_emptied_structs.count(elem_type_id)) {
          // An array of an emptied struct is itself emptied: the declaration
          // remains, but variables and parameters of it are removed.
          _emptied_structs.insert(Id(op.args[0]));
        }
      }
      break;

    case spv::OpTypePointer:
      if (op.args.size() >= 3) {
        if (_affected_types.count(Id(op.args[2]))) {
          _affected_pointer_types.insert(Id(op.args[0]));
        }
        if (is_deleted(Id(op.args[2])) || _emptied_structs.count(Id(op.args[2]))) {
          // The pointee type was deleted, or was emptied to the point that
          // nothing can be accessed through this pointer anymore.
          _deleted.insert(Id(op.args[0]));
        }
      }
      break;

    case spv::OpTypeFunction:
      // Erase deleted types in the function parameter list, and add
      // parameters for the variables hoisted out of structs passed in.
      if (op.args.size() >= 2) {
        SpirVModule::Args new_args({op.args[0], op.args[1]});

        for (size_t ai = 2; ai < op.args.size(); ++ai) {
          Id arg(op.args[ai]);
          if (!is_deleted(arg)) {
            new_args.push_back(arg);
          }

          // Structs with non-opaque types must be passed through pointers.
          nassertd(!_affected_types.count(arg)) continue;

          if (_affected_pointer_types.count(arg)) {
            auto ait = _affected_types.find(module.unwrap_pointer_type(arg));
            nassertd(ait != _affected_types.end()) continue;

            // Passing a struct with non-opaque types to a function.  That
            // means adding additional parameters for the hoisted variables.
            for (auto &pair : ait->second) {
              Id type_ptr_id = module.define_pointer_type(pair.first, spv::StorageClassUniformConstant);
              new_args.push_back(type_ptr_id);
            }
          }
        }

        // Updating the declaration may make it identical to another function
        // type, which is illegal SPIR-V; the deduplication that runs after the
        // pass merges those.
        Instruction *decl = module.find_declaration(Id(op.args[0]));
        nassertd(decl != nullptr) break;
        decl->args = std::move(new_args);
      }
      break;

    case spv::OpVariable:
      if (op.args.size() >= 3) {
        Id type_id = module.unwrap_pointer_type(Id(op.args[0]));

        // If the struct contained only resources, the variable is deleted, but
        // the resources therein must of course still be hoisted out of it.
        bool remove = is_deleted(type_id) || is_deleted(Id(op.args[0])) ||
                      _emptied_structs.count(type_id) != 0;

        auto ait = _affected_types.find(type_id);
        if (ait != _affected_types.end()) {
          Id var_id(op.args[1]);
          spv::StorageClass storage_class = (spv::StorageClass)op.args[2];

          for (const auto &pair : ait->second) {
            const ShaderType *new_type = pair.first;
            const AccessChain &access_chain = pair.second;

            Id new_id = module.define_variable(new_type, storage_class);

            AccessChain new_access_chain(access_chain);
            new_access_chain._var_id = var_id;

            _hoisted_vars[std::move(new_access_chain)] = new_id;
          }
        }

        if (remove) {
          _deleted.insert(Id(op.args[1]));
        }
      }
      break;

    default:
      break;
    }
  }
}

/**
 * Rewrites the body of the given function.
 */
void SpirVHoistStructResourcesPass::
process_function(SpirVModule &module, Function &function) {
  // Rewrite the structured parameter list to match the function type that
  // process_declarations() rebuilt.  The id index retains each original
  // parameter's type, which is needed to decide what to remove and hoist.
  pvector<Id> new_parameters;
  for (Id param_id : function.parameters) {
    Id param_type_id = module.get_type_id(param_id);

    if (is_deleted(param_type_id)) {
      // Keep its id state queryable until the deferred deletion at the end of
      // the pass, but leave it out of the rebuilt parameter list immediately.
      _deleted.insert(param_id);
    } else {
      new_parameters.push_back(param_id);
    }

    // Structs with non-opaque types must be passed through pointers.
    nassertv(!_affected_types.count(param_type_id));

    if (_affected_pointer_types.count(param_type_id)) {
      auto ait = _affected_types.find(module.unwrap_pointer_type(param_type_id));
      nassertv(ait != _affected_types.end());

      // Add parameters for the variables hoisted out of this struct.
      for (auto &pair : ait->second) {
        Id type_ptr_id = module.define_pointer_type(
          pair.first, spv::StorageClassUniformConstant);
        Id id = module.allocate_id();
        new_parameters.push_back(id);
        module.record_result(Instruction(spv::OpFunctionParameter,
                                         {type_ptr_id, id}), function.id);

        AccessChain access_chain(pair.second);
        access_chain._var_id = param_id;
        _hoisted_vars[std::move(access_chain)] = id;
      }
    }
  }
  function.parameters = std::move(new_parameters);

  SpirVInstructionCursor cursor(module, function);
  while (cursor.next()) {
    switch (cursor->opcode) {
    case spv::OpAccessChain:
    case spv::OpInBoundsAccessChain:
      if (cursor->args.size() >= 4) {
        Id result_pointer_type_id(cursor->args[0]);
        if (is_deleted(result_pointer_type_id)) {
          // The pointer type was deleted because it pointed into a struct that
          // contained only resources; the chain must also be deleted.
          _deleted.insert(Id(cursor->args[1]));
          cursor.detach();
          break;
        }
        Id result_type_id = module.unwrap_pointer_type(result_pointer_type_id);

        if (is_deleted(result_type_id)) {
          // Empty struct, so access chain must also be deleted.
          _deleted.insert(Id(cursor->args[1]));
          cursor.detach();
          break;
        }

        // Walk through the access chain.  The base variable and its pointer
        // type may be slated for deletion (if the struct contained nothing
        // but resources), but their declarations are still intact.
        Id base_pointer_type_id = module.get_type_id(Id(cursor->args[2]));
        Id parent_id = module.unwrap_pointer_type(base_pointer_type_id);

        Instruction &op = *cursor;

        if (_hoist_types.count(result_type_id)) {
          // Construct the access chain with just struct members, to figure
          // out which variable it's referring to.
          AccessChain access_chain(Id(op.args[2]));

          SpirVModule::Args new_args({op.args[0], op.args[1], op.args[2]});

          for (size_t ai = 3; ai < op.args.size(); ++ai) {
            const Instruction *parent_decl =
              module.find_declaration(parent_id);
            nassertv(parent_decl != nullptr);
            if (parent_decl->opcode != spv::OpTypeStruct) { // array
              parent_id = module.get_composite_member_type_id(parent_id, 0);
              nassertv(parent_id != 0);
              new_args.push_back(op.args[ai]);
            } else {
              uint32_t index = module.resolve_constant(Id(op.args[ai]));
              access_chain.append(index);

              parent_id = get_original_member_type_id(module, parent_id, index);
            }
          }

          if (access_chain.size() == 0) {
            // There are no structs involved, this is probably just an array of
            // samplers.  No need to hoist.
            break;
          }

          auto hit = _hoisted_vars.find(access_chain);
          nassertv(hit != _hoisted_vars.end());
          Id new_var_id = hit->second;

          // Change the access chain to remove the struct member indices, and
          // the base id to our variable.
          new_args[2] = new_var_id;
          op.args = std::move(new_args);
          break;
        }

        if (_affected_types.count(parent_id)) {
          // We may still need to remap the struct member indices.
          // It may also still be pointing to a struct containing a non-opaque
          // type, so add additional access chains for the hoisted members.
          AccessChain access_chain(Id(op.args[2]));

          SpirVModule::Args new_args({op.args[0], op.args[1], op.args[2]});
          SpirVModule::Args hoisted_new_args({0, 0, 0});
          bool deleted_member = false;

          for (size_t ai = 3; ai < op.args.size(); ++ai) {
            const Instruction *parent_decl =
              module.find_declaration(parent_id);
            nassertv(parent_decl != nullptr);
            if (parent_decl->opcode == spv::OpTypeArray ||
                parent_decl->opcode == spv::OpTypeRuntimeArray) {
              parent_id = module.get_composite_member_type_id(parent_id, 0);
              new_args.push_back(op.args[ai]);
              hoisted_new_args.push_back(op.args[ai]);
              nassertv(parent_id != 0);
            }
            else if (parent_decl->opcode == spv::OpTypeStruct) {
              uint32_t index = module.resolve_constant(Id(op.args[ai]));

              int new_index = (int)index;
              auto rit = _member_remap.find(parent_id);
              if (rit != _member_remap.end() && index < rit->second.size()) {
                new_index = rit->second[index];
              }
              if (new_index < 0) {
                // This one was removed, huh.  Let's hope this access chain
                // never gets loaded.  Remove it.
                deleted_member = true;
                break;
              }

              if (new_index != (int)index) {
                new_args.push_back(module.define_int_constant(new_index));
              } else {
                new_args.push_back(op.args[ai]);
              }
              access_chain.append((uint32_t)new_index);

              parent_id = get_original_member_type_id(module, parent_id, index);
            }
            else {
              // Indexing into a non-aggregate composite type (vector/matrix).
              // Just leave the rest of the access chain intact.
              new_args.insert(new_args.end(), op.args.begin() + ai, op.args.end());
              break;
            }
          }

          if (deleted_member) {
            _deleted.insert(Id(op.args[1]));
            cursor.detach();
            break;
          }

          Id orig_chain_id(op.args[1]);
          op.args = std::move(new_args);

          auto ait = _affected_types.find(result_type_id);
          if (ait != _affected_types.end()) {
            cursor.insert_after([&](SpirVBuilder &builder) {
              for (auto &pair : ait->second) {
                AccessChain full(access_chain);
                full.extend(pair.second);

                auto hit = _hoisted_vars.find(full);
                nassertv(hit != _hoisted_vars.end());
                Id hoisted_var_id = hit->second;

                Id hoisted_type_ptr_id = module.define_pointer_type(pair.first, spv::StorageClassUniformConstant);
                nassertv(hoisted_type_ptr_id != 0);

                Id id = builder.allocate_id();
                hoisted_new_args[0] = hoisted_type_ptr_id;
                hoisted_new_args[1] = id;
                hoisted_new_args[2] = hoisted_var_id;
                builder.insert(spv::OpAccessChain, hoisted_new_args.data(), hoisted_new_args.size());

                AccessChain new_access_chain(pair.second);
                new_access_chain._var_id = orig_chain_id;
                _hoisted_vars[std::move(new_access_chain)] = id;
              }
            });
          }
        }
      }
      break;

    case spv::OpFunctionCall:
      if (cursor->args.size() >= 3) {
        Instruction &op = *cursor;
        SpirVModule::Args new_args({op.args[0], op.args[1], op.args[2]});

        for (size_t ai = 3; ai < op.args.size(); ++ai) {
          Id arg(op.args[ai]);
          Id arg_type_id = module.get_type_id(arg);
          if (!is_deleted(arg_type_id) && !is_deleted(arg)) {
            new_args.push_back(arg);
          }

          // Structs with samplers must be passed through a pointer.
          nassertd(!_affected_types.count(arg_type_id)) continue;

          if (_affected_pointer_types.count(arg_type_id)) {
            auto ait = _affected_types.find(module.unwrap_pointer_type(arg_type_id));
            nassertd(ait != _affected_types.end()) continue;

            // Passing a struct with non-opaque types to a function.  That means
            // adding additional parameters for the hoisted variables.
            for (auto &pair : ait->second) {
              AccessChain access_chain(pair.second);
              access_chain._var_id = arg;

              auto hit = _hoisted_vars.find(access_chain);
              nassertv(hit != _hoisted_vars.end());
              new_args.push_back(hit->second);
            }
          }
        }

        op.args = std::move(new_args);
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
      nassertv(!_affected_types.count(Id(cursor->args[0])));
      nassertv(!is_deleted(Id(cursor->args[2])));
      break;

    case spv::OpCopyObject:
    case spv::OpCopyLogical:
    case spv::OpExpectKHR:
      // Not allowed to copy structs containing resources.
      nassertv(!_affected_types.count(Id(cursor->args[0])));
      nassertv(!_affected_pointer_types.count(Id(cursor->args[0])));

      // Copying an empty struct means deleting the copy.
      if (is_deleted(Id(cursor->args[2])) || is_deleted(Id(cursor->args[0]))) {
        _deleted.insert(Id(cursor->args[1]));
        cursor.detach();
      }
      break;

    case spv::OpReturnValue:
      // Cannot return a struct with an opaque type from a function.
      if (cursor->args.size() >= 1) {
        Id type_id = module.get_type_id(Id(cursor->args[0]));
        nassertv(!is_deleted(Id(cursor->args[0])));
        nassertv(!is_deleted(type_id));
        nassertv(!_affected_types.count(type_id));
        nassertv(!_affected_pointer_types.count(type_id));
      }
      break;

    default:
      break;
    }
  }
}
