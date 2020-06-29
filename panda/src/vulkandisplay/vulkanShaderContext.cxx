/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanShaderContext.cxx
 * @author rdb
 * @date 2016-02-18
 */

#include "vulkanShaderContext.h"

TypeHandle VulkanShaderContext::_type_handle;

/**
 * Creates the shader modules.
 */
bool VulkanShaderContext::
create_modules(VkDevice device, const ShaderType::Struct *push_constant_block_type) {
  nassertr(push_constant_block_type != nullptr, false);

  _push_constant_block_type = push_constant_block_type;

  // Compose a struct type for all the mat inputs, also gathering ones that
  // should go into a separate push constant block.  This will become a new
  // uniform block in the shader that replaces the regular uniforms.
  pvector<int> mat_struct_locations;
  pvector<int> push_constant_locations = {-1, -1};

  if (!_shader->_mat_spec.empty()) {
    ShaderType::Struct struct_type;

    for (const Shader::ShaderMatSpec &spec : _shader->_mat_spec) {
      if (spec._id._location >= 0) {
        if (spec._func == Shader::SMF_compose &&
            spec._piece == Shader::SMP_whole &&
            spec._part[0] == Shader::SMO_model_to_apiview &&
            spec._part[1] == Shader::SMO_apiview_to_apiclip) {
          // This is p3d_ModelViewProjectionMatrix, which is a push constant.
          push_constant_locations[0] = spec._id._location;
          _projection_mat_stage_mask |= spec._id._stage_mask;
          _push_constant_stage_mask |= spec._id._stage_mask;
        }
        else if (spec._func == Shader::SMF_first &&
                 (spec._piece == Shader::SMP_row3 || spec._piece == Shader::SMP_row3x3) &&
                 spec._part[0] == Shader::SMO_attr_colorscale) {
          // This is p3d_ColorScale or equivalent, which is a push constant.
          push_constant_locations[1] = spec._id._location;
          _color_scale_stage_mask |= spec._id._stage_mask;
          _push_constant_stage_mask |= spec._id._stage_mask;
        }
        else {
          // Other inputs are done via UBOs.
          struct_type.add_member(spec._id._type, spec._id._name->get_name());
          mat_struct_locations.push_back(spec._id._location);
          _mat_deps |= spec._dep;
          _mat_block_stage_mask |= spec._id._stage_mask;
          _mat_spec.push_back(spec);
        }
      }
    }

    if (struct_type.get_num_members() > 0) {
      _mat_block_type = ShaderType::register_type(std::move(struct_type));
      _mat_block_size = _mat_block_type->get_size_bytes();
      ++_num_uniform_offsets;
    }
  }

  // Compose a struct type for all the ptr inputs.
  pvector<int> ptr_struct_locations;
  if (!_shader->_ptr_spec.empty()) {
    ShaderType::Struct struct_type;

    for (const Shader::ShaderPtrSpec &spec : _shader->_ptr_spec) {
      if (spec._id._location >= 0) {
        struct_type.add_member(spec._id._type, spec._id._name->get_name());
        ptr_struct_locations.push_back(spec._id._location);
        _ptr_block_stage_mask |= spec._id._stage_mask;
      }
    }

    if (struct_type.get_num_members() > 0) {
      _ptr_block_type = ShaderType::register_type(std::move(struct_type));
      _ptr_block_size = _ptr_block_type->get_size_bytes();
      ++_num_uniform_offsets;
    }
  }

  for (COWPT(ShaderModule) &cow_module : _shader->_modules) {
    CPT(ShaderModule) module = cow_module.get_read_pointer();

    const ShaderModuleSpirV *spv_module = DCAST(ShaderModuleSpirV, module.p());
    nassertd(spv_module != nullptr) continue;

    // Make a clean copy, so we can do some transformations on it.
    ShaderModuleSpirV::InstructionStream instructions = spv_module->_instructions;
    ShaderModuleSpirV::InstructionWriter writer(instructions);

    size_t count = 0;
    if (_mat_block_size > 0) {
      writer.make_block(_mat_block_type, mat_struct_locations, spv::StorageClassUniform, count++, 1);
    }
    if (_ptr_block_size > 0) {
      writer.make_block(_ptr_block_type, ptr_struct_locations, spv::StorageClassUniform, count++, 1);
    }
    if (_push_constant_stage_mask & (1 << (int)module->get_stage())) {
      writer.make_block(push_constant_block_type, push_constant_locations, spv::StorageClassPushConstant, 0, 0);
    }

    // Change OpenGL conventions to Vulkan conventions.
    for (ShaderModuleSpirV::Instruction op : instructions) {
      if (op.opcode == spv::OpExecutionMode) {
        if (op.nargs >= 2 && (spv::ExecutionMode)op.args[1] == spv::ExecutionModeOriginLowerLeft) {
          op.args[1] = spv::ExecutionModeOriginUpperLeft;
        }
      }
      else if (op.opcode == spv::OpDecorate) {
        if (op.nargs >= 3 && op.args[1] == spv::DecorationBuiltIn) {
          switch ((spv::BuiltIn)op.args[2]) {
          case spv::BuiltInVertexId:
            op.args[2] = spv::BuiltInVertexIndex;
            break;
          case spv::BuiltInInstanceId:
            op.args[2] = spv::BuiltInInstanceIndex;
            break;
          default:
            break;
          }
        }
      }
    }

    VkShaderModuleCreateInfo module_info;
    module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_info.pNext = nullptr;
    module_info.flags = 0;
    module_info.codeSize = instructions.get_data_size() * 4;
    module_info.pCode = (const uint32_t *)instructions.get_data();

    VkResult err;
    err = vkCreateShaderModule(device, &module_info, nullptr, &_modules[(size_t)spv_module->get_stage()]);
    if (err) {
      vulkan_error(err, "Failed to create shader modules");
      return false;
    }
  }

  return true;
}

/**
 *
 */
bool VulkanShaderContext::
make_pipeline_layout(VkDevice device) {
  // Create a layout containing two sets: one for textures, one for uniforms.
  uint32_t tex_binding_count = (uint32_t)_shader->_tex_spec.size();
  VkDescriptorSetLayoutBinding *ds_bindings[2] = {
    (VkDescriptorSetLayoutBinding *)alloca(sizeof(VkDescriptorSetLayoutBinding) * tex_binding_count),
    (VkDescriptorSetLayoutBinding *)alloca(sizeof(VkDescriptorSetLayoutBinding) * 2),
  };

  uint32_t index = 0;
  for (const Shader::ShaderTexSpec &spec : _shader->_tex_spec) {
    VkDescriptorSetLayoutBinding &binding = ds_bindings[0][index];
    binding.binding = index;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = spec._id._stage_mask;
    binding.pImmutableSamplers = nullptr;
    ++index;
  }

  {
    VkDescriptorSetLayoutCreateInfo set_info;
    set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_info.pNext = nullptr;
    set_info.flags = 0;
    set_info.bindingCount = tex_binding_count;
    set_info.pBindings = ds_bindings[0];

    VkResult
    err = vkCreateDescriptorSetLayout(device, &set_info, nullptr,
      &_descriptor_set_layouts[VulkanGraphicsStateGuardian::DS_texture_attrib]);
    if (err) {
      vulkan_error(err, "Failed to create descriptor set layout");
      return false;
    }
  }

  // Same for the UBOs.
  size_t count = 0;
  if (_mat_block_size > 0) {
    VkDescriptorSetLayoutBinding &binding = ds_bindings[1][count];
    binding.binding = count++;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    binding.descriptorCount = 1;
    binding.stageFlags = _mat_block_stage_mask;
    binding.pImmutableSamplers = nullptr;
  }
  if (_ptr_block_size > 0) {
    VkDescriptorSetLayoutBinding &binding = ds_bindings[1][count];
    binding.binding = count++;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    binding.descriptorCount = 1;
    binding.stageFlags = _ptr_block_stage_mask;
    binding.pImmutableSamplers = nullptr;
  }

  {
    VkDescriptorSetLayoutCreateInfo set_info;
    set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_info.pNext = nullptr;
    set_info.flags = 0;
    set_info.bindingCount = count;
    set_info.pBindings = ds_bindings[1];

    VkResult
    err = vkCreateDescriptorSetLayout(device, &set_info, nullptr,
      &_descriptor_set_layouts[VulkanGraphicsStateGuardian::DS_dynamic_uniforms]);
    if (err) {
      vulkan_error(err, "Failed to create descriptor set layout");
      return false;
    }
  }

  // Create the push constant range.  Because we have declared the same block
  // in all stages that use any push constant, we only need to define one range.
  // I'm happy with that, because I've gone down the route of trying to sort out
  // the ranges exactly in the way that Vulkan wants it, and it's not fun.
  VkPushConstantRange range;
  range.stageFlags = _push_constant_stage_mask;
  range.offset = 0;
  range.size = _push_constant_stage_mask ? _push_constant_block_type->get_size_bytes() : 0;

  VkPipelineLayoutCreateInfo layout_info;
  layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layout_info.pNext = nullptr;
  layout_info.flags = 0;
  layout_info.setLayoutCount = VulkanGraphicsStateGuardian::DS_SET_COUNT;
  layout_info.pSetLayouts = _descriptor_set_layouts;
  layout_info.pushConstantRangeCount = _push_constant_stage_mask ? 1 : 0;
  layout_info.pPushConstantRanges = &range;

  VkResult
  err = vkCreatePipelineLayout(device, &layout_info, nullptr, &_pipeline_layout);
  if (err) {
    vulkan_error(err, "Failed to create pipeline layout");
    return false;
  }

  return true;
}

/**
 * Updates the ShaderMatSpec uniforms.
 */
void VulkanShaderContext::
update_uniform_buffers(VulkanGraphicsStateGuardian *gsg, int altered) {
  if (_mat_block_size == 0 && _ptr_block_size == 0) {
    return;
  }

  size_t count = 0;
  if (altered & _mat_deps) {
    gsg->update_shader_matrix_cache(_shader, _mat_part_cache, altered);

    void *ptr = alloca(_mat_block_size);

    size_t i = 0;
    for (Shader::ShaderMatSpec &spec : _mat_spec) {
      const LMatrix4 *val = gsg->fetch_specified_value(spec, _mat_part_cache, altered);
      if (!val) continue;
  #ifndef STDFLOAT_DOUBLE
      // In this case, the data is already single-precision.
      const PN_float32 *data = val->get_data();
  #else
      // In this case, we have to convert it.
      LMatrix4f valf = LCAST(PN_float32, *val);
      const PN_float32 *data = valf.get_data();
  #endif

      uint32_t offset = _mat_block_type->get_member(i++).offset;
      char *dest = (char *)ptr + offset;

      switch (spec._piece) {
      case Shader::SMP_whole:
        memcpy(dest, data, 64);
        break;
      case Shader::SMP_row0:
        memcpy(dest, data + 0, 16);
        break;
      case Shader::SMP_row1:
        memcpy(dest, data + 4, 16);
        break;
      case Shader::SMP_row2:
        memcpy(dest, data + 8, 16);
        break;
      case Shader::SMP_row3:
        memcpy(dest, data + 12, 16);
        break;
      case Shader::SMP_row3x1:
        memcpy(dest, data + 12, 4);
        break;
      case Shader::SMP_row3x2:
        memcpy(dest, data + 12, 8);
        break;
      case Shader::SMP_row3x3:
        memcpy(dest, data + 12, 12);
        break;
      case Shader::SMP_cell15:
        memcpy(dest, data + 15, 4);
        break;
      case Shader::SMP_cell14:
        memcpy(dest, data + 14, 4);
        break;
      case Shader::SMP_cell13:
        memcpy(dest, data + 13, 4);
        break;
      default:
        //TODO: transpositions and such
        assert(false);
        break;
      }
    }

    _uniform_offsets[count] = gsg->update_dynamic_uniform_buffer(ptr, _mat_block_size);
  }

  if (_mat_block_size != 0) {
    ++count;
  }

  //TODO: ptr inputs
  _uniform_offsets[count] = 0;
}

/**
 * Updates a VkDescriptorSet with the resources of the current render state.
 */
bool VulkanShaderContext::
update_descriptor_set(VulkanGraphicsStateGuardian *gsg, VkDescriptorSet ds) {
  PreparedGraphicsObjects *pgo = gsg->get_prepared_objects();

  size_t num_textures = _shader->_tex_spec.size();
  VkWriteDescriptorSet *writes = (VkWriteDescriptorSet *)alloca(num_textures * sizeof(VkWriteDescriptorSet));
  VkDescriptorImageInfo *image_infos = (VkDescriptorImageInfo *)alloca(num_textures * sizeof(VkDescriptorImageInfo));

  for (size_t i = 0; i < num_textures; ++i) {
    Shader::ShaderTexSpec &spec = _shader->_tex_spec[i];

    SamplerState sampler;
    int view = gsg->get_current_tex_view_offset();

    PT(Texture) tex = gsg->fetch_specified_texture(spec, sampler, view);
    if (tex.is_null()) {
      tex = gsg->_white_texture;
    }

    if (tex->get_texture_type() != spec._desired_type) {
      switch (spec._part) {
      case Shader::STO_named_input:
        vulkandisplay_cat.error()
          << "Sampler type of shader input '" << *spec._name << "' does not "
             "match type of texture " << *tex << ".\n";
        break;

      case Shader::STO_stage_i:
        vulkandisplay_cat.error()
          << "Sampler type of shader input p3d_Texture" << spec._stage
          << " does not match type of texture " << *tex << ".\n";
        break;

      case Shader::STO_light_i_shadow_map:
        vulkandisplay_cat.error()
          << "Sampler type of shader input p3d_LightSource[" << spec._stage
          << "].shadowMap does not match type of texture " << *tex << ".\n";
        break;
      }
      // TODO: also check whether shadow sampler textures have shadow filter
      // enabled.
    }

    VulkanTextureContext *tc;
    DCAST_INTO_R(tc, tex->prepare_now(view, pgo, gsg), false);

    VulkanSamplerContext *sampc;
    DCAST_INTO_R(sampc, sampler.prepare_now(pgo, gsg), false);

    tc->set_active(true);
    gsg->update_texture(tc, true);

    // Transition the texture so that it can be read by the shader.  This has
    // to happen on the transfer command buffer, since it can't happen during
    // an active render pass.
    VkPipelineStageFlags stage_flags = 0;
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::vertex)) {
      stage_flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::tess_control)) {
      stage_flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
    }
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::tess_evaluation)) {
      stage_flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    }
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::fragment)) {
      stage_flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::geometry)) {
      stage_flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    }
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::compute)) {
      stage_flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    tc->transition(gsg->_transfer_cmd, gsg->_graphics_queue_family_index,
                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                   stage_flags, VK_ACCESS_SHADER_READ_BIT);

    VkDescriptorImageInfo &image_info = image_infos[i];
    image_info.sampler = sampc->_sampler;
    image_info.imageView = tc->_image_view;
    image_info.imageLayout = tc->_layout;

    VkWriteDescriptorSet &write = writes[i];
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = ds;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &image_info;
    write.pBufferInfo = nullptr;
    write.pTexelBufferView = nullptr;
  }

  vkUpdateDescriptorSets(gsg->_device, num_textures, writes, 0, nullptr);
  return true;
}

/**
 * Returns a VkPipeline for the given RenderState+GeomVertexFormat combination.
 */
VkPipeline VulkanShaderContext::
get_pipeline(VulkanGraphicsStateGuardian *gsg, const RenderState *state,
             const GeomVertexFormat *format, VkPrimitiveTopology topology,
             VkSampleCountFlagBits multisamples) {
  PipelineKey key;
  key._state = state;
  key._format = format;
  key._topology = topology;
  key._multisamples = multisamples;

  PipelineMap::const_iterator it;
  it = _pipeline_map.find(key);
  if (it == _pipeline_map.end()) {
    VkPipeline pipeline = gsg->make_pipeline(this, state, format, topology, multisamples);
    _pipeline_map[std::move(key)] = pipeline;
    return pipeline;
  } else {
    return it->second;
  }
}

/**
 * Returns a VkPipeline for running a compute shader.
 */
VkPipeline VulkanShaderContext::
get_compute_pipeline(VulkanGraphicsStateGuardian *gsg) {
  if (_compute_pipeline != VK_NULL_HANDLE) {
    return _compute_pipeline;
  }

  nassertr(_modules[(size_t)Shader::Stage::compute] != VK_NULL_HANDLE, VK_NULL_HANDLE);

  VkPipeline pipeline = gsg->make_compute_pipeline(this);
  _compute_pipeline = pipeline;
  return pipeline;
}
