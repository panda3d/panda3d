/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanFramebuffer.cxx
 * @author rdb
 * @date 2026-06-02
 */

#include "vulkanFramebuffer.h"
#include "vulkanGraphicsStateGuardian.h"
#include "vulkanTextureContext.h"
#include "drawableRegion.h"

/**
 * Returns true if the given attachment is a depth and/or stencil attachment,
 * or false if it is a color attachment.
 */
static bool
is_depth_stencil(const VulkanFramebuffer::Attachment &attach) {
  return (attach._tc->_aspect_mask &
          (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0;
}

/**
 * Removes all attachments without destroying any owned resources.  This is used
 * by the window, which manages the lifetime of its swapchain images itself and
 * merely points the framebuffer at the image to render to in any given frame.
 */
void VulkanFramebuffer::
clear_attachments() {
  _attachments.clear();
}

/**
 * Adds a color or depth/stencil attachment.  Whether it is treated as a color
 * or a depth/stencil attachment is decided by the texture context's aspect
 * mask, which must be set before calling this.
 *
 * If a texture is passed, the texture context is assumed to be owned by the
 * prepared objects and will not be deleted by destroy().
 */
void VulkanFramebuffer::
add_attachment(VulkanTextureContext *tc, VkAttachmentStoreOp store_op,
               Texture *texture) {
  _attachments.push_back({tc, texture, store_op});
}

/**
 * Replaces the attachment at the given (already existing) index.  This is used
 * by the window to repoint its color attachment at the freshly acquired
 * swapchain image each frame, without disturbing the rest of the attachment
 * list.  Whether the attachment is a color or depth/stencil attachment is still
 * decided by the texture context's aspect mask.
 */
void VulkanFramebuffer::
set_attachment(size_t index, VulkanTextureContext *tc,
               VkAttachmentStoreOp store_op, Texture *texture) {
  nassertv(index < _attachments.size());
  _attachments[index] = {tc, texture, store_op};
}

/**
 * Destroys the attachment resources owned by this framebuffer.  Before calling
 * this, make sure that no commands are executing on any queue that uses these
 * resources.
 */
void VulkanFramebuffer::
destroy(VulkanGraphicsStateGuardian *vkgsg) {
  VkDevice device = vkgsg->_device;

  for (Attachment &attach : _attachments) {
    if (vkgsg->_last_frame_data != nullptr) {
      attach._tc->release(*vkgsg->_last_frame_data);
    } else {
      attach._tc->destroy_now(device);
    }
    if (attach._texture.is_null()) {
      delete attach._tc;
    }
  }
  _attachments.clear();
}

/**
 * Begins a dynamic rendering pass that renders into all of the attachments,
 * applying the clear settings from the given region.  Returns true on success.
 */
bool VulkanFramebuffer::
begin_rendering(VulkanGraphicsStateGuardian *vkgsg, DrawableRegion *region) {
  VulkanCommandBuffer &cmd = vkgsg->_render_cmd;
  nassertr((VkCommandBuffer)cmd != VK_NULL_HANDLE, false);

  // Count the color attachments so that we can size the array.
  size_t num_color = 0;
  for (const Attachment &attach : _attachments) {
    if (!is_depth_stencil(attach)) {
      ++num_color;
    }
  }

  VkRenderingAttachmentInfo *color_attachments = (VkRenderingAttachmentInfo *)
    alloca(num_color * sizeof(VkRenderingAttachmentInfo));
  VkRenderingAttachmentInfo depth_attachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
  VkRenderingAttachmentInfo stencil_attachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};

  VkRenderingInfo render_info = {VK_STRUCTURE_TYPE_RENDERING_INFO};
  render_info.layerCount = 1;
  render_info.renderArea.extent.width = _size[0];
  render_info.renderArea.extent.height = _size[1];
  render_info.colorAttachmentCount = 0;
  render_info.pColorAttachments = color_attachments;

  vkgsg->_fb_color_tc = nullptr;
  vkgsg->_fb_depth_tc = nullptr;

  for (Attachment &attach : _attachments) {
    VulkanTextureContext *tc = attach._tc;
    // Note: <=, not <, because a window deliberately bumps the swapchain image's
    // _read_seq up to the current command buffer to keep add_barrier() from
    // hoisting the barrier ahead of the semaphore wait.
    nassertr(tc->_read_seq <= cmd._seq, false);
    tc->set_active(true);

    if (is_depth_stencil(attach)) {
      VkImageView view = tc->get_image_view(0);
      VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

      if (tc->_aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) {
        if (region->get_clear_active(DrawableRegion::RTP_depth)) {
          depth_attachment.clearValue.depthStencil.depth = region->get_clear_depth();
          depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        } else {
          depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
        depth_attachment.storeOp = attach._store_op;
        depth_attachment.imageView = view;
        depth_attachment.imageLayout = layout;
        render_info.pDepthAttachment = &depth_attachment;
      }

      if (tc->_aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) {
        if (region->get_clear_active(DrawableRegion::RTP_stencil)) {
          stencil_attachment.clearValue.depthStencil.stencil = region->get_clear_stencil();
          stencil_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        } else {
          stencil_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
        stencil_attachment.storeOp = attach._store_op;
        stencil_attachment.imageView = view;
        stencil_attachment.imageLayout = layout;
        render_info.pStencilAttachment = &stencil_attachment;
      }

      cmd.add_barrier(tc, layout,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
        VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

      vkgsg->_fb_depth_tc = tc;
    }
    else {
      VkRenderingAttachmentInfo &color_attachment =
        color_attachments[render_info.colorAttachmentCount++];
      color_attachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};

      if (region->get_clear_active(DrawableRegion::RTP_color)) {
        LColor clear_color = region->get_clear_value(DrawableRegion::RTP_color);
        color_attachment.clearValue.color.float32[0] = clear_color[0];
        color_attachment.clearValue.color.float32[1] = clear_color[1];
        color_attachment.clearValue.color.float32[2] = clear_color[2];
        color_attachment.clearValue.color.float32[3] = clear_color[3];
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      } else {
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      }
      color_attachment.storeOp = attach._store_op;
      color_attachment.imageView = tc->get_image_view(0);
      color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

      cmd.add_barrier(tc, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

      vkgsg->_fb_color_tc = tc;
    }
  }

  cmd.flush_barriers();
  vkgsg->_vkCmdBeginRendering(cmd, &render_info);
  vkgsg->_fb_config = _config_id;
  return true;
}

/**
 * Ends the dynamic rendering pass started by begin_rendering() and updates the
 * synchronization state of the attachments to reflect the writes done during
 * rendering.
 */
void VulkanFramebuffer::
end_rendering(VulkanGraphicsStateGuardian *vkgsg) {
  VulkanCommandBuffer &cmd = vkgsg->_render_cmd;
  vkgsg->_vkCmdEndRendering(cmd);

  for (Attachment &attach : _attachments) {
    VulkanTextureContext *tc = attach._tc;
    if (is_depth_stencil(attach)) {
      tc->_write_stage_mask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT
                            | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
      tc->_write_access_mask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    } else {
      tc->_write_stage_mask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
      tc->_write_access_mask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    }
    tc->_read_stage_mask = 0;
    tc->_write_seq = cmd._seq;
  }
}
