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
#include "spirVTransformer.h"
#include "spirVConvertBoolToIntPass.h"
#include "spirVMakeBlockPass.h"

#define SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
#include <spirv_cross/spirv_glsl.hpp>

TypeHandle VulkanShaderContext::_type_handle;

/**
 * Creates the shader modules.
 */
bool VulkanShaderContext::
create_modules(VkDevice device, const ShaderType::Struct *push_constant_block_type) {
  for (const Shader::ShaderVarSpec &spec : _shader->_var_spec) {
    if (spec._name == InternalName::get_color()) {
      _uses_vertex_color = true;
      break;
    }
  }

  // Compose a struct type for all the mat inputs, also gathering ones that
  // should go into a separate push constant block.  This will become a new
  // uniform block in the shader that replaces the regular uniforms.
  pvector<const InternalName *> push_constant_params({nullptr, nullptr});
  pvector<const InternalName *> shader_input_block_params;
  pvector<const InternalName *> other_state_block_params;

  pvector<const InternalName *> tex_stage_set_params;
  pvector<const InternalName *> tex_input_set_params;

  ShaderType::Struct shader_input_block_struct;
  ShaderType::Struct other_state_block_struct;
  bool replace_bools = false;

  for (const Shader::Parameter &param : _shader->_parameters) {
    if (param._binding == nullptr) {
      continue;
    }
    const ShaderType *element_type;
    uint32_t num_elements;
    param._type->unwrap_array(element_type, num_elements);
    if (element_type->as_resource() != nullptr) {
      Descriptor desc;
      desc._binding = param._binding;
      desc._resource_ids.resize(num_elements);
      desc._stage_mask = param._stage_mask;

      if (const ShaderType::SampledImage *sampler = param._type->as_sampled_image()) {
        desc._type =
          (sampler->get_texture_type() == Texture::TT_buffer_texture)
            ? VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
            : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      }
      else if (const ShaderType::Image *image = param._type->as_image()) {
        desc._type =
          (image->get_texture_type() == Texture::TT_buffer_texture)
            ? VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
            : VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
      }
      else {
        continue;
      }

      for (uint32_t i = 0; i < num_elements; ++i) {
        desc._resource_ids[i] = param._binding->get_resource_id(i, element_type);
      }

      if (param._binding->get_state_dep() & Shader::D_texture) {
        tex_stage_set_params.push_back(param._name);
        _tex_stage_descriptors.push_back(std::move(desc));
        _num_tex_stage_descriptor_elements += num_elements;
      }
      else if (param._binding->get_state_dep() & Shader::D_shader_inputs) {
        tex_input_set_params.push_back(param._name);
        _tex_input_descriptors.push_back(std::move(desc));
        _num_tex_input_descriptor_elements += num_elements;
      }
      continue;
    }
    if (push_constant_block_type != nullptr) {
      if (param._binding->is_model_to_apiclip_matrix()) {
        push_constant_params[0] = param._name.p();
        _projection_mat_stage_mask |= param._stage_mask;
        _push_constant_stage_mask |= param._stage_mask;
        continue;
      }
      if (param._binding->is_color_scale()) {
        push_constant_params[1] = param._name.p();
        _color_scale_stage_mask |= param._stage_mask;
        _push_constant_stage_mask |= param._stage_mask;
        continue;
      }
    }
    const ShaderType *type = param._type->replace_scalar_type(ShaderType::ST_bool, ShaderType::ST_int);
    if (param._type != type) {
      replace_bools = true;
    }
    int dep = param._binding->get_state_dep();
    if ((dep & ~(Shader::D_frame | Shader::D_shader_inputs)) == 0) {
      // Purely dependent on shader inputs, goes into separate block.
      shader_input_block_struct.add_member(type, param._name->get_name());
      shader_input_block_params.push_back(param._name.p());
      _shader_input_block._deps |= dep;
      _shader_input_block._stage_mask |= param._stage_mask;
      _shader_input_block._bindings.push_back({param._binding, shader_input_block_struct.get_member(shader_input_block_struct.get_num_members() - 1).offset});
    } else {
      // Goes into the UBO containing all other inputs.
      other_state_block_struct.add_member(type, param._name->get_name());
      other_state_block_params.push_back(param._name.p());
      _other_state_block._deps |= dep;
      _other_state_block._stage_mask |= param._stage_mask;
      _other_state_block._bindings.push_back({param._binding, other_state_block_struct.get_member(other_state_block_struct.get_num_members() - 1).offset});
    }
  }

  const ShaderType *shader_input_block_type = nullptr;
  if (shader_input_block_struct.get_num_members() > 0) {
    shader_input_block_type = ShaderType::register_type(std::move(shader_input_block_struct));
    _shader_input_block._size = shader_input_block_type->get_size_bytes();
  }

  const ShaderType *other_state_block_type = nullptr;
  if (other_state_block_struct.get_num_members() > 0) {
    other_state_block_type = ShaderType::register_type(std::move(other_state_block_struct));
    _other_state_block._size = other_state_block_type->get_size_bytes();
  }

  bool success = true;
  for (Shader::LinkedModule &linked_module : _shader->_modules) {
    CPT(ShaderModule) module = linked_module._module.get_read_pointer();
    nassertd(module != nullptr) {
      success = false;
      continue;
    }

    if (!module->is_of_type(ShaderModuleSpirV::get_class_type())) {
      vulkandisplay_cat.error()
        << "Shader modules of type " << module->get_type()
        << " are not supported in Vulkan\n";
      success = false;
      continue;
    }

    const ShaderModuleSpirV *spv_module = (const ShaderModuleSpirV *)module.p();
    SpirVTransformer transformer(spv_module->_instructions);

    // These are not used in Vulkan, and the validation layers trip over them.
    transformer.strip_uniform_locations();

    if (replace_bools) {
      transformer.run(SpirVConvertBoolToIntPass());
    }

    if (_push_constant_stage_mask != 0) {
      auto ids = spv_module->get_parameter_ids_from_names(push_constant_params);
      transformer.run(SpirVMakeBlockPass(push_constant_block_type->as_struct(),
                                         ids, spv::StorageClassPushConstant));
    }

    // Create UBOs and a push constant block for the uniforms.
    if (other_state_block_type != nullptr) {
      auto ids = spv_module->get_parameter_ids_from_names(other_state_block_params);
      transformer.run(SpirVMakeBlockPass(other_state_block_type->as_struct(),
                                         ids, spv::StorageClassUniform,
                                         0, VulkanGraphicsStateGuardian::DS_dynamic_uniforms));
    }
    if (shader_input_block_type != nullptr) {
      auto ids = spv_module->get_parameter_ids_from_names(shader_input_block_params);
      transformer.run(SpirVMakeBlockPass(shader_input_block_type->as_struct(),
                                         ids, spv::StorageClassUniform,
                                         0, VulkanGraphicsStateGuardian::DS_shader_attrib));
    }

    // Bind the textures to the desired descriptor sets.
    if (!tex_stage_set_params.empty()) {
      auto ids = spv_module->get_parameter_ids_from_names(tex_stage_set_params);
      transformer.bind_descriptor_set(VulkanGraphicsStateGuardian::DS_texture_attrib, ids);
    }
    if (!tex_input_set_params.empty()) {
      auto ids = spv_module->get_parameter_ids_from_names(tex_input_set_params);

      if (_shader_input_block._size > 0) {
        // Make room for the uniform buffer binding.
        ids.insert(ids.begin(), 0);
      }

      transformer.bind_descriptor_set(VulkanGraphicsStateGuardian::DS_shader_attrib, ids);
    }

    // Change OpenGL conventions to Vulkan conventions.
    ShaderModuleSpirV::InstructionStream instructions = transformer.get_result();
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

#ifndef NDEBUG
    if (!instructions.validate(SPV_ENV_VULKAN_1_0)) {
      success = false;
      break;
    }
#endif

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
      success = false;
    }
  }

  if (!success) {
    for (size_t i = 0; i <= (size_t)Shader::Stage::compute; ++i) {
      if (_modules[i] != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device, _modules[i], nullptr);
        _modules[i] = VK_NULL_HANDLE;
      }
    }
  }

  return success;
}

/**
 * Creates a descriptor set to hold the TextureAttrib textures in this shader.
 * Result is returned and also stored in _tattr_descriptor_set_layout.
 */
VkDescriptorSetLayout VulkanShaderContext::
make_texture_attrib_descriptor_set_layout(VkDevice device) {
  size_t num_descriptors = _tex_stage_descriptors.size();
  VkDescriptorSetLayoutBinding *bindings;
  bindings = (VkDescriptorSetLayoutBinding *)alloca(sizeof(VkDescriptorSetLayoutBinding) * num_descriptors);

  size_t i = 0;
  for (const Descriptor &desc : _tex_stage_descriptors) {
    VkDescriptorSetLayoutBinding &binding = bindings[i];
    binding.binding = i++;
    binding.descriptorType = desc._type;
    binding.descriptorCount = desc._resource_ids.size();
    binding.stageFlags = desc._stage_mask;
    binding.pImmutableSamplers = nullptr;
  }

  VkDescriptorSetLayoutCreateInfo set_info;
  set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  set_info.pNext = nullptr;
  set_info.flags = 0;
  set_info.bindingCount = i;
  set_info.pBindings = bindings;

  VkDescriptorSetLayout result;
  VkResult
  err = vkCreateDescriptorSetLayout(device, &set_info, nullptr, &result);
  if (err) {
    vulkan_error(err, "Failed to create descriptor set layout for TextureAttrib");
    return VK_NULL_HANDLE;
  }

  _tattr_descriptor_set_layout = result;
  return result;
}

/**
 * Creates a descriptor set to hold the shader inputs in this shader.
 * Result is returned and also stored in _sattr_descriptor_set_layout.
 */
VkDescriptorSetLayout VulkanShaderContext::
make_shader_attrib_descriptor_set_layout(VkDevice device) {
  size_t num_descriptors = _tex_input_descriptors.size() + 1;
  VkDescriptorSetLayoutBinding *bindings;
  bindings = (VkDescriptorSetLayoutBinding *)alloca(sizeof(VkDescriptorSetLayoutBinding) * num_descriptors);

  size_t i = 0;

  // First binding is for the UBO.
  if (_shader_input_block._size > 0) {
    VkDescriptorSetLayoutBinding &binding = bindings[i];
    binding.binding = i++;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.descriptorCount = 1;
    binding.stageFlags = _shader_input_block._stage_mask;
    binding.pImmutableSamplers = nullptr;
  }

  // Then the descriptors.
  for (const Descriptor &desc : _tex_input_descriptors) {
    VkDescriptorSetLayoutBinding &binding = bindings[i];
    binding.binding = i++;
    binding.descriptorType = desc._type;
    binding.descriptorCount = desc._resource_ids.size();
    binding.stageFlags = desc._stage_mask;
    binding.pImmutableSamplers = nullptr;
  }

  VkDescriptorSetLayoutCreateInfo set_info;
  set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  set_info.pNext = nullptr;
  set_info.flags = 0;
  set_info.bindingCount = i;
  set_info.pBindings = bindings;

  VkDescriptorSetLayout result;
  VkResult
  err = vkCreateDescriptorSetLayout(device, &set_info, nullptr, &result);
  if (err) {
    vulkan_error(err, "Failed to create descriptor set layout for ShaderAttrib");
    return VK_NULL_HANDLE;
  }

  _sattr_descriptor_set_layout = result;
  return result;
}

/**
 * Creates a descriptor set to hold the dynamic uniform blocks in this shader.
 */
VkDescriptorSetLayout VulkanShaderContext::
make_dynamic_uniform_descriptor_set_layout(VkDevice device) {
  VkDescriptorSetLayoutBinding bindings[2];

  // This is a dynamic UBO, which means that we'll be specifying the offsets in
  // the bind call, rather than when writing the descriptor set.
  size_t count = 0;
  if (_other_state_block._size > 0) {
    VkDescriptorSetLayoutBinding &binding = bindings[count];
    binding.binding = count++;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    binding.descriptorCount = 1;
    binding.stageFlags = _other_state_block._stage_mask;
    binding.pImmutableSamplers = nullptr;
  }

  VkDescriptorSetLayoutCreateInfo set_info;
  set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  set_info.pNext = nullptr;
  set_info.flags = 0;
  set_info.bindingCount = count;
  set_info.pBindings = bindings;

  VkDescriptorSetLayout result;
  VkResult
  err = vkCreateDescriptorSetLayout(device, &set_info, nullptr, &result);
  if (err) {
    vulkan_error(err, "Failed to create descriptor set layout for dynamic uniforms");
    return VK_NULL_HANDLE;
  }

  _dynamic_uniform_descriptor_set_layout = result;
  return result;
}

/**
 *
 */
bool VulkanShaderContext::
fetch_descriptor(VulkanGraphicsStateGuardian *gsg, const Descriptor &desc,
                 VkWriteDescriptorSet &write, VkDescriptorImageInfo *&image_infos) {

  ShaderInputBinding::State state;
  state.gsg = gsg;
  state.matrix_cache = &_matrix_cache[0];

  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.pNext = nullptr;
  write.dstArrayElement = 0;
  write.descriptorCount = desc._resource_ids.size();
  write.descriptorType = desc._type;
  write.pImageInfo = nullptr;
  write.pBufferInfo = nullptr;
  write.pTexelBufferView = nullptr;

  PreparedGraphicsObjects *pgo = gsg->get_prepared_objects();

  switch (desc._type) {
  case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    for (ResourceId id : desc._resource_ids) {
      SamplerState sampler;
      int view = gsg->get_current_tex_view_offset();
      PT(Texture) texture = desc._binding->fetch_texture(state, id, sampler, view);

      VulkanTextureContext *tc;
      DCAST_INTO_R(tc, texture->prepare_now(pgo, gsg), false);

      VulkanSamplerContext *sc;
      DCAST_INTO_R(sc, sampler.prepare_now(pgo, gsg), false);

      tc->set_active(true);
      gsg->update_texture(tc, true);

      tc->transition(gsg->_frame_data->_transfer_cmd,
                     gsg->_graphics_queue_family_index,
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     desc._stage_mask, VK_ACCESS_SHADER_READ_BIT);

      VkDescriptorImageInfo &image_info = *image_infos++;
      image_info.sampler = sc->_sampler;
      image_info.imageView = tc->get_image_view(view);
      image_info.imageLayout = tc->_layout;
      write.pImageInfo = &image_info;
    }
    break;

  case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    for (ResourceId id : desc._resource_ids) {
      SamplerState sampler;
      int view = gsg->get_current_tex_view_offset();
      PT(Texture) texture = desc._binding->fetch_texture(state, id, sampler, view);

      VulkanTextureContext *tc;
      DCAST_INTO_R(tc, texture->prepare_now(pgo, gsg), false);

      tc->set_active(true);
      gsg->update_texture(tc, true);

      tc->transition(gsg->_frame_data->_transfer_cmd,
                     gsg->_graphics_queue_family_index,
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     desc._stage_mask, VK_ACCESS_SHADER_READ_BIT);

      write.pTexelBufferView = &tc->get_buffer_view(view);
    }
    break;

  case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
  case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
    for (ResourceId id : desc._resource_ids) {
      ShaderType::Access access = ShaderType::Access::read_write;
      int z = -1;
      int n = 0;
      PT(Texture) texture = desc._binding->fetch_texture_image(state, id, access, z, n);
      access = access & desc._access;

      VkAccessFlags access_mask = 0;
      if ((access & ShaderType::Access::read_only) != ShaderType::Access::none) {
        access_mask |= VK_ACCESS_SHADER_READ_BIT;
      }
      if ((access & ShaderType::Access::write_only) != ShaderType::Access::none) {
        access_mask |= VK_ACCESS_SHADER_WRITE_BIT;
      }

      VulkanTextureContext *tc;
      DCAST_INTO_R(tc, texture->prepare_now(pgo, gsg), false);

      tc->set_active(true);
      gsg->update_texture(tc, true);

      tc->transition(gsg->_frame_data->_transfer_cmd,
                     gsg->_graphics_queue_family_index,
                     VK_IMAGE_LAYOUT_GENERAL,
                     desc._stage_mask, access_mask);

      int view = gsg->get_current_tex_view_offset();
      if (desc._type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {
        write.pTexelBufferView = &tc->get_buffer_view(view);
      } else {
        VkDescriptorImageInfo &image_info = *image_infos++;
        image_info.sampler = VK_NULL_HANDLE;
        image_info.imageView = tc->get_image_view(view);
        image_info.imageLayout = tc->_layout;
        write.pImageInfo = &image_info;
      }
    }
    break;

  default:
    nassertr(false, false);
    return false;
  }

  return true;
}

/**
 * Updates the descriptor set containing all the texture attributes.
 */
bool VulkanShaderContext::
update_tattr_descriptor_set(VulkanGraphicsStateGuardian *gsg, VkDescriptorSet ds) {
  VkWriteDescriptorSet *writes = (VkWriteDescriptorSet *)alloca(_tex_stage_descriptors.size() * sizeof(VkWriteDescriptorSet));
  VkDescriptorImageInfo *image_infos = (VkDescriptorImageInfo *)alloca(_num_tex_stage_descriptor_elements * sizeof(VkDescriptorImageInfo));

  size_t wi = 0;
  size_t di = 0;
  for (const Descriptor &desc : _tex_stage_descriptors) {
    if (!fetch_descriptor(gsg, desc, writes[wi], image_infos)) {
      return false;
    }
    writes[wi].dstSet = ds;
    writes[wi].dstBinding = di++;
    ++wi;
  }
  gsg->_vkUpdateDescriptorSets(gsg->_device, wi, writes, 0, nullptr);
  return true;
}

/**
 * Updates the descriptor set containing all the ShaderAttrib textures.
 */
bool VulkanShaderContext::
update_sattr_descriptor_set(VulkanGraphicsStateGuardian *gsg, VkDescriptorSet ds) {
  // Allocate enough memory.
  size_t max_num_descriptors = 1 + _tex_input_descriptors.size();
  VkWriteDescriptorSet *writes = (VkWriteDescriptorSet *)alloca(max_num_descriptors * sizeof(VkWriteDescriptorSet));
  VkDescriptorImageInfo *image_infos = (VkDescriptorImageInfo *)alloca(_num_tex_input_descriptor_elements * sizeof(VkDescriptorImageInfo));

  // First the UBO, then the shader input textures.
  size_t di = 0;
  size_t wi = 0;

  VkDescriptorBufferInfo buffer_info;
  if (_shader_input_block._size > 0) {
    buffer_info.offset = update_sattr_uniforms(gsg, buffer_info.buffer);
    buffer_info.range = _shader_input_block._size;

    VkWriteDescriptorSet &write = writes[wi++];
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = ds;
    write.dstBinding = di;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.pImageInfo = nullptr;
    write.pBufferInfo = &buffer_info;
    write.pTexelBufferView = nullptr;

    ++di;
  }

  for (const Descriptor &desc : _tex_input_descriptors) {
    if (!fetch_descriptor(gsg, desc, writes[wi], image_infos)) {
      return false;
    }
    writes[wi].dstSet = ds;
    writes[wi].dstBinding = di++;
    ++wi;
  }
  gsg->_vkUpdateDescriptorSets(gsg->_device, wi, writes, 0, nullptr);
  return true;
}

/**
 * Updates the ShaderPtrSpec uniforms, which change with the ShaderAttrib.
 */
uint32_t VulkanShaderContext::
update_sattr_uniforms(VulkanGraphicsStateGuardian *gsg, VkBuffer &buffer) {
  if (_shader_input_block._size == 0) {
    return 0;
  }

  uint32_t ubo_offset;
  void *ptr = gsg->alloc_dynamic_uniform_buffer(_shader_input_block._size, buffer, ubo_offset);

  ShaderInputBinding::State state;
  state.gsg = gsg;
  state.matrix_cache = &_matrix_cache[0];

  for (const Block::Binding &binding : _shader_input_block._bindings) {
    binding._binding->fetch_data(state, (unsigned char *)ptr + binding._offset);
  }

  return ubo_offset;
}

/**
 * Updates the ShaderMatSpec uniforms, if they have changed.
 */
uint32_t VulkanShaderContext::
update_dynamic_uniforms(VulkanGraphicsStateGuardian *gsg, int altered) {
  if (_other_state_block._size == 0) {
    return 0;
  }

  if (altered & _other_state_block._deps) {
    if (altered & _matrix_cache_deps) {
      gsg->update_shader_matrix_cache(_shader, &_matrix_cache[0], altered);
    }

    VkBuffer ubo;
    void *ptr = gsg->alloc_dynamic_uniform_buffer(_other_state_block._size, ubo, _dynamic_uniform_offset);

    ShaderInputBinding::State state;
    state.gsg = gsg;
    state.matrix_cache = &_matrix_cache[0];

    for (const Block::Binding &binding : _other_state_block._bindings) {
      binding._binding->fetch_data(state, (unsigned char *)ptr + binding._offset);
    }

    if (ubo != _uniform_buffer) {
      // If the buffer has changed, we need to recreate this descriptor set.
      gsg->update_dynamic_uniform_descriptor_set(this);
      _uniform_buffer = ubo;
    }
  }

  return _dynamic_uniform_offset;
}

/**
 * Returns a VkPipeline for the given RenderState+GeomVertexFormat combination.
 */
VkPipeline VulkanShaderContext::
get_pipeline(VulkanGraphicsStateGuardian *gsg, const RenderState *state,
             const GeomVertexFormat *format, VkPrimitiveTopology topology,
             uint32_t patch_control_points, VkSampleCountFlagBits multisamples) {
  PipelineKey key;
  key._format = format;
  key._topology = topology;
  key._patch_control_points = patch_control_points;
  key._multisamples = multisamples;

  const ColorAttrib *color_attr;
  state->get_attrib_def(color_attr);
  key._color_type = color_attr->get_color_type();

  const RenderModeAttrib *render_mode;
  if (state->get_attrib(render_mode) &&
      (render_mode->get_mode() == RenderModeAttrib::M_wireframe ||
       render_mode->get_mode() == RenderModeAttrib::M_point)) {
    key._render_mode_attrib = render_mode;
  }

  const CullFaceAttrib *cull_face;
  state->get_attrib_def(cull_face);
  key._cull_face_mode = cull_face->get_effective_mode();

  const DepthWriteAttrib *depth_write;
  state->get_attrib_def(depth_write);
  key._depth_write_mode = depth_write->get_mode();

  const DepthTestAttrib *depth_test;
  state->get_attrib_def(depth_test);
  key._depth_test_mode = depth_test->get_mode();

  const ColorWriteAttrib *color_write;
  state->get_attrib_def(color_write);
  key._color_write_mask = color_write->get_channels();

  const LogicOpAttrib *logic_op;
  state->get_attrib_def(logic_op);
  key._logic_op = logic_op->get_operation();

  const ColorBlendAttrib *color_blend;
  if (state->get_attrib(color_blend) && color_blend->get_mode() == ColorBlendAttrib::M_none) {
    key._color_blend_attrib = color_blend;
    key._transparency_mode = TransparencyAttrib::M_none;
  } else {
    const TransparencyAttrib *transp;
    state->get_attrib_def(transp);
    key._transparency_mode = transp->get_mode();
  }

  PipelineMap::const_iterator it;
  it = _pipeline_map.find(key);
  if (it == _pipeline_map.end()) {
    VkPipeline pipeline = gsg->make_pipeline(this, key);
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
