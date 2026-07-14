/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVEmulateTextureQueriesPass.cxx
 * @author rdb
 * @date 2024-11-19
 */

#include "spirVEmulateTextureQueriesPass.h"
#include "spirVInstructionCursor.h"
#include "spirVUsageAnalysis.h"

/**
 *
 */
void SpirVEmulateTextureQueriesPass::
run(SpirVModule &module) {
  if (_emulate_caps & Shader::C_sampler_cube_shadow) {
    // Cube shadow samplers are not supported; the Dref sample operations are
    // rewritten below to do the depth comparison in the shader, so these
    // must become regular, non-depth cube image types.
    pvector<std::pair<Id, Id> > replacements;

    // Scan the type declarations for depth cube images.  Caching the id_bound
    // ensures we never match the new replacement definitions.
    uint32_t id_bound = module.get_id_bound();
    for (uint32_t word = 0; word < id_bound; ++word) {
      Id id(word);
      if (module.get_definition_type(id) != SpirVModule::DT_type) {
        continue;
      }
      // Preserve the original image properties other than depthness.
      const Instruction *decl = module.find_declaration(id);
      if (decl != nullptr && decl->opcode == spv::OpTypeImage &&
          decl->args.size() >= 8 && decl->args[2] == spv::DimCube &&
          decl->args[3] > 0) {
        Id new_id = module.define_image_type(
          module.resolve_type(id), 0, decl->args[6],
          (spv::ImageFormat)decl->args[7]);
        nassertd(new_id != 0) continue;
        replacements.push_back(std::make_pair(id, new_id));
      }
    }

    // Apply afterwards, so that the walk above cannot encounter a rewritten
    // declaration.
    for (const auto &item : replacements) {
      module.replace_type_id(item.first, item.second);
    }
  }

  if (_emulate_caps & (Shader::C_image_query_size | Shader::C_texture_query_size | Shader::C_texture_query_levels)) {
    SpirVUsageAnalysis usage = module.analyze_usage();
    for (uint32_t word = 0; word < module.get_id_bound(); ++word) {
      Id id(word);
      if (module.get_definition_type(id) == SpirVModule::DT_variable &&
          usage.was_size_or_levels_queried(id)) {
        _access_chains.insert({id, AccessChain(id)});
      }
    }
  }

  for (Function &function : module.modify_functions()) {
    SpirVInstructionCursor cursor(module, function);
    while (cursor.next()) {
      Instruction &op = *cursor;
      switch (op.opcode) {
      case spv::OpAccessChain:
      case spv::OpInBoundsAccessChain:
        if (op.args.size() >= 4) {
          auto it = _access_chains.find(Id(op.args[2]));
          if (it == _access_chains.end()) {
            break;
          }
          AccessChain chain = it->second;
          Id parent_id = module.unwrap_pointer_type(module.get_type_id(Id(op.args[2])));

          bool constant_chain = true;
          for (size_t ai = 3; ai < op.args.size(); ++ai) {
            if (module.get_definition_type(Id(op.args[ai])) != SpirVModule::DT_constant) {
              constant_chain = false;
              break;
            }
            uint32_t index = module.resolve_constant(Id(op.args[ai]));
            chain.append(index);

            const Instruction *type_decl = module.find_declaration(parent_id);
            nassertd(type_decl != nullptr) break;
            if (type_decl->opcode == spv::OpTypeStruct) {
              parent_id = module.get_composite_member_type_id(parent_id, index);
            } else { // array
              parent_id = module.get_composite_member_type_id(parent_id, 0);
              nassertd(parent_id != 0) break;
            }
          }
          if (constant_chain) {
            _access_chains.insert({Id(op.args[1]), std::move(chain)});
          }
        }
        break;

      case spv::OpLoad:
      case spv::OpCopyObject:
      case spv::OpCopyLogical:
      case spv::OpExpectKHR:
      case spv::OpImage:
      case spv::OpSampledImage:
        if (op.args.size() >= 3) {
          auto it = _access_chains.find(Id(op.args[2]));
          if (it != _access_chains.end()) {
            _access_chains.insert({Id(op.args[1]), it->second});
          }
        }
        break;

      case spv::OpImageSampleDrefImplicitLod:
      case spv::OpImageSampleDrefExplicitLod:
        if (_emulate_caps & Shader::C_sampler_cube_shadow) {
          const ShaderType *type = module.resolve_type(Id(op.args[2]));
          const ShaderType::SampledImage *sampled_image =
            type != nullptr ? type->as_sampled_image() : nullptr;
          if (sampled_image == nullptr ||
              (sampled_image->get_texture_type() != Texture::TT_cube_map &&
               sampled_image->get_texture_type() != Texture::TT_cube_map_array)) {
            break;
          }

          if (_float_one_id == 0) {
            _float_one_id = module.define_float_constant(1.0f);
          }
          if (_float_zero_id == 0) {
            _float_zero_id = module.define_float_constant(0.0f);
          }

          // Copy the instruction, since the insertions below invalidate the
          // reference.
          Instruction orig = op;

          cursor.replace([&](SpirVBuilder &builder) -> Id {
            Id sample = builder.op_image_sample(
              Id(orig.args[2]), Id(orig.args[3]),
              orig.args.size() >= 6 ? orig.args[5] : 0u,
              orig.args.size() > 6 ? orig.args.data() + 6 : nullptr);
            Id depth = builder.op_composite_extract(sample, {0});
            Id cmp = builder.op_compare(spv::OpFOrdGreaterThan, depth, Id(orig.args[4]));
            return builder.op_select(cmp, _float_one_id, _float_zero_id);
          });
        }
        break;

      case spv::OpImageQuerySize:
      case spv::OpImageQuerySizeLod:
      case spv::OpImageQueryLevels:
        if (op.args.size() >= 3) {
          if (op.opcode == spv::OpImageQueryLevels) {
            if ((_emulate_caps & Shader::C_texture_query_levels) == 0) {
              break;
            }
          }
          else if ((_emulate_caps & (Shader::C_texture_query_size | Shader::C_image_query_size)) == 0) {
            break;
          }

          auto acit = _access_chains.find(Id(op.args[2]));
          if (acit == _access_chains.end()) {
            break;
          }
          const AccessChain &chain = acit->second;

          if (op.opcode == spv::OpImageQuerySizeLod && op.args.size() >= 4) {
            if (module.get_definition_type(Id(op.args[3])) != SpirVModule::DT_constant ||
                module.resolve_constant(Id(op.args[3])) != 0) {
              // Can't handle a non-zero level of detail parameter.
              break;
            }
          }

          Id size_var_id;
          auto it = _size_var_ids.find(chain);
          if (it != _size_var_ids.end()) {
            size_var_id = it->second;
          } else {
            // It's always a vec4, with number of levels in fourth component
            const ShaderType *var_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));
            size_var_id = module.define_variable(var_type, spv::StorageClassUniformConstant);
            _size_var_ids.insert({chain, size_var_id});
          }

          // Copy what we need before inserting anything.
          spv::Op query_opcode = op.opcode;
          Id result_type_id(op.args[0]);

          const ShaderType *result_type = module.resolve_type(result_type_id);
          if (query_opcode != spv::OpImageQueryLevels &&
              result_type->as_scalar() == nullptr &&
              result_type->as_vector() == nullptr) {
            nassertd(false) break;
          }

          cursor.replace([&](SpirVBuilder &builder) -> Id {
            Id temp = builder.op_load(size_var_id);

            // Grab the components we need out of the vec4 size variable.
            if (query_opcode == spv::OpImageQueryLevels) {
              temp = builder.op_composite_extract(temp, {3});
            }
            else if (result_type->as_scalar() != nullptr) {
              temp = builder.op_composite_extract(temp, {0});
            }
            else {
              const ShaderType::Vector *vector = result_type->as_vector();
              pvector<uint32_t> components;
              for (size_t ci = 0; ci < vector->get_num_components(); ++ci) {
                components.push_back(ci);
              }
              temp = builder.op_vector_shuffle(temp, temp, components);
            }

            return builder.op_convert(ShaderType::ST_int, temp);
          });
        }
        break;

      default:
        break;
      }
    }
  }
}
