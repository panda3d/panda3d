/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanTextureContext.h
 * @author rdb
 * @date 2016-02-19
 */

#ifndef VULKANTEXTURECONTEXT_H
#define VULKANTEXTURECONTEXT_H

#include "config_vulkandisplay.h"
#include "textureContext.h"

/**
 * Manages a Vulkan image and device memory.
 */
class EXPCL_VULKANDISPLAY VulkanTextureContext : public TextureContext {
public:
  INLINE VulkanTextureContext(PreparedGraphicsObjects *pgo, Texture *texture, int view);
  INLINE VulkanTextureContext(PreparedGraphicsObjects *pgo, VkImage image, VkFormat format);
  ~VulkanTextureContext() {};

  ALLOC_DELETED_CHAIN(VulkanTextureContext);

  INLINE void access(VkPipelineStageFlags stage_mask, VkAccessFlags access_mask);
  INLINE void discard();

  void clear_color_image(VkCommandBuffer cmd, const VkClearColorValue &value);
  void clear_depth_stencil_image(VkCommandBuffer cmd, const VkClearDepthStencilValue &value);

  void transition(VkCommandBuffer cmd, uint32_t queue_family, VkImageLayout layout,
                  VkPipelineStageFlags dst_stage_mask, VkAccessFlags dst_access_mask);

public:
  VkFormat _format;
  VkExtent3D _extent;
  int _mipmap_begin, _mipmap_end;
  uint32_t _mip_levels;
  uint32_t _array_layers;
  VkImageAspectFlags _aspect_mask;
  bool _generate_mipmaps;
  bool _pack_bgr8;

  VkImage _image;
  VkDeviceMemory _memory;
  VkImageView _image_view;

  VkImageLayout _layout;
  VkAccessFlags _access_mask;
  VkPipelineStageFlags _stage_mask;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextureContext::init_type();
    register_type(_type_handle, "VulkanTextureContext",
                  TextureContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "vulkanTextureContext.I"

#endif  // VULKANTEXTURECONTEXT_H
