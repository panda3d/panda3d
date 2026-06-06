/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanFramebuffer.h
 * @author rdb
 * @date 2026-06-02
 */

#ifndef VULKANFRAMEBUFFER_H
#define VULKANFRAMEBUFFER_H

#include "config_vulkandisplay.h"
#include "texture.h"
#include "small_vector.h"
#include "lvecBase2.h"
#include "pointerTo.h"

class VulkanGraphicsStateGuardian;
class VulkanTextureContext;
class DrawableRegion;

/**
 * Describes a set of render target attachments and encapsulates the logic
 * shared by VulkanGraphicsWindow and VulkanGraphicsBuffer for beginning and
 * ending a dynamic rendering pass.
 *
 * The framebuffer does not own the lifetime of the swapchain images managed by
 * a window; for an offscreen buffer it owns the attachment images it creates.
 */
class EXPCL_VULKANDISPLAY VulkanFramebuffer {
public:
  /**
   * Describes the formats and sample count of a framebuffer.  This serves as a
   * compatibility key for the pipeline cache, so two framebuffers with an equal
   * Config may share render pipelines.
   */
  struct Config {
    small_vector<VkFormat, 1> _color_formats;
    VkFormat _depth_format = VK_FORMAT_UNDEFINED;
    VkFormat _stencil_format = VK_FORMAT_UNDEFINED;
    VkSampleCountFlagBits _sample_count = VK_SAMPLE_COUNT_1_BIT;

    bool operator == (const Config &other) const {
      if (_color_formats.size() != other._color_formats.size() ||
          _depth_format != other._depth_format ||
          _stencil_format != other._stencil_format ||
          _sample_count != other._sample_count) {
        return false;
      }
      for (size_t i = 0; i < _color_formats.size(); ++i) {
        if (_color_formats[i] != other._color_formats[i]) {
          return false;
        }
      }
      return true;
    }
  };

  /**
   * A single color or depth/stencil attachment.  Whether it is a color or a
   * depth/stencil attachment is determined by the texture context's aspect
   * mask.
   *
   * If the render target is multisampled and a resolve target is given, the
   * multisampled samples are resolved into the (single-sampled) resolve target
   * at the end of the rendering pass.
   */
  struct Attachment {
    VulkanTextureContext *_tc = nullptr;
    VulkanTextureContext *_resolve_tc = nullptr;

    // Keeps a render-to-texture target alive.  If null, the texture context is
    // owned by this framebuffer and destroyed along with it.  The same applies
    // to _resolve_texture and _resolve_tc.
    PT(Texture) _texture;
    PT(Texture) _resolve_texture;

    VkAttachmentStoreOp _store_op = VK_ATTACHMENT_STORE_OP_STORE;
  };

  void clear_attachments();
  void add_attachment(VulkanTextureContext *tc,
                      VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_STORE,
                      Texture *texture = nullptr,
                      VulkanTextureContext *resolve_tc = nullptr,
                      Texture *resolve_texture = nullptr);
  void set_attachment(size_t index, VulkanTextureContext *tc,
                      VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_STORE,
                      Texture *texture = nullptr,
                      VulkanTextureContext *resolve_tc = nullptr,
                      Texture *resolve_texture = nullptr);
  void destroy(VulkanGraphicsStateGuardian *vkgsg);

  bool begin_rendering(VulkanGraphicsStateGuardian *vkgsg, DrawableRegion *region);
  void end_rendering(VulkanGraphicsStateGuardian *vkgsg);

public:
  LVecBase2i _size;
  Config _config;
  uint32_t _config_id = 0;

  small_vector<Attachment, 2> _attachments;
};

#include "vulkanFramebuffer.I"

#endif  // VULKANFRAMEBUFFER_H
