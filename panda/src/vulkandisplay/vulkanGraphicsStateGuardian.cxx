/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanGraphicsStateGuardian.cxx
 * @author rdb
 * @date 2016-02-16
 */

#include "vulkanGraphicsStateGuardian.h"
#include "standardMunger.h"

TypeHandle VulkanGraphicsStateGuardian::_type_handle;

/**
 *
 */
VulkanGraphicsStateGuardian::
VulkanGraphicsStateGuardian(GraphicsEngine *engine, VulkanGraphicsPipe *pipe,
                            VulkanGraphicsStateGuardian *share_with) :
  GraphicsStateGuardian(CS_default, engine, pipe),
  _device(VK_NULL_HANDLE),
  _queue(VK_NULL_HANDLE),
  _cmd_pool(VK_NULL_HANDLE),
  _cmd(VK_NULL_HANDLE)
{
  const char *const layers[] = {
    "VK_LAYER_LUNARG_standard_validation",
  };

  const char *extensions[] = {
    "VK_KHR_swapchain",
  };

  const float queue_priorities[1] = {0.0f};
  VkDeviceQueueCreateInfo queue_info;
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.pNext = NULL;
  queue_info.flags = 0;
  queue_info.queueFamilyIndex = 0;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = queue_priorities;

  VkDeviceCreateInfo device_info;
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = NULL;
  device_info.flags = 0;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = &queue_info;
  device_info.enabledLayerCount = 1;
  device_info.ppEnabledLayerNames = layers;
  device_info.enabledExtensionCount = 1;
  device_info.ppEnabledExtensionNames = extensions;
  device_info.pEnabledFeatures = NULL;

  VkResult
  err = vkCreateDevice(pipe->_gpu, &device_info, NULL, &_device);
  if (err) {
    vulkan_error(err, "Failed to create device");
    return;
  }

  vkGetDeviceQueue(_device, 0, 0, &_queue);

  // Create a command pool to allocate command buffers from.
  VkCommandPoolCreateInfo pool_info;
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.pNext = NULL;
  pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  pool_info.queueFamilyIndex = 0;

  err = vkCreateCommandPool(_device, &pool_info, NULL, &_cmd_pool);
  if (err) {
    vulkan_error(err, "Failed to create command pool");
    return;
  }

  _needs_reset = false;
}

/**
 *
 */
VulkanGraphicsStateGuardian::
~VulkanGraphicsStateGuardian() {
}

/**
 * Creates a new GeomMunger object to munge vertices appropriate to this GSG
 * for the indicated state.
 */
PT(GeomMunger) VulkanGraphicsStateGuardian::
make_geom_munger(const RenderState *state, Thread *current_thread) {
  PT(StandardMunger) munger = new StandardMunger(this, state, 1, GeomEnums::NT_packed_dabc, GeomEnums::C_color);
  return GeomMunger::register_munger(munger, current_thread);
}

/**
 * Clears the framebuffer within the current DisplayRegion, according to the
 * flags indicated by the given DrawableRegion object.
 *
 * This does not set the DisplayRegion first.  You should call
 * prepare_display_region() to specify the region you wish the clear operation
 * to apply to.
 */
void VulkanGraphicsStateGuardian::
clear(DrawableRegion *clearable) {
  nassertv(_cmd != VK_NULL_HANDLE);
  nassertv(clearable->is_any_clear_active());

  VkClearAttachment attachments[2];
  int ai = 0;

  if (clearable->get_clear_color_active()) {
    LColor color = clearable->get_clear_color();
    VkClearAttachment &attachment = attachments[ai++];
    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    attachment.colorAttachment = 0;
    attachment.clearValue.color.float32[0] = color[0];
    attachment.clearValue.color.float32[1] = color[1];
    attachment.clearValue.color.float32[2] = color[2];
    attachment.clearValue.color.float32[3] = color[3];
  }

  if (clearable->get_clear_depth_active() ||
      clearable->get_clear_stencil_active()) {
    VkClearAttachment &attachment = attachments[ai++];
    attachment.aspectMask = 0;
    if (clearable->get_clear_depth_active()) {
      attachment.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (clearable->get_clear_stencil_active()) {
      attachment.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    attachment.colorAttachment = 0; // unused
    attachment.clearValue.depthStencil.depth = clearable->get_clear_depth();
    attachment.clearValue.depthStencil.stencil = clearable->get_clear_stencil();
  }

  // Collect the rectangles we want to clear.
  VkClearRect *rects = (VkClearRect *)alloca(sizeof(VkClearRect) * _viewports.size());
  for (size_t i = 0; i < _viewports.size(); ++i) {
    rects[i].rect = _viewports[i];
    rects[i].baseArrayLayer = 0;
    rects[i].layerCount = 1;
  }

  vkCmdClearAttachments(_cmd, ai, attachments, _viewports.size(), rects);
}

/**
 * Prepare a display region for rendering (set up scissor region and viewport)
 */
void VulkanGraphicsStateGuardian::
prepare_display_region(DisplayRegionPipelineReader *dr) {
  nassertv(dr != (DisplayRegionPipelineReader *)NULL);
  GraphicsStateGuardian::prepare_display_region(dr);

  int count = dr->get_num_regions();
  VkViewport *viewports = (VkViewport *)alloca(sizeof(VkViewport) * count);
  _viewports.resize(count);

  for (int i = 0; i < count; ++i) {
    int x, y, w, h;
    dr->get_region_pixels(i, x, y, w, h);
    viewports[i].x = x;
    viewports[i].y = y;
    viewports[i].width = w;
    viewports[i].height = h;

    // Also save this in the _viewports array for later use.
    _viewports[i].offset.x = x;
    _viewports[i].offset.y = y;
    _viewports[i].extent.width = w;
    _viewports[i].extent.height = h;
  }

  vkCmdSetViewport(_cmd, 0, count, viewports);
  vkCmdSetScissor(_cmd, 0, count, &_viewports[0]);
}

/**
 * Called before each frame is rendered, to allow the GSG a chance to do any
 * internal cleanup before beginning the frame.
 *
 * The return value is true if successful (in which case the frame will be
 * drawn and end_frame() will be called later), or false if unsuccessful (in
 * which case nothing will be drawn and end_frame() will not be called).
 */
bool VulkanGraphicsStateGuardian::
begin_frame(Thread *current_thread) {
  if (!GraphicsStateGuardian::begin_frame(current_thread)) {
    return false;
  }

  // Create a command buffer.
  VkCommandBufferAllocateInfo alloc_info;
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.pNext = NULL;
  alloc_info.commandPool = _cmd_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = 1;

  VkResult err;
  err = vkAllocateCommandBuffers(_device, &alloc_info, &_cmd);
  nassertr(!err, false);
  nassertr(_cmd != VK_NULL_HANDLE, false);

  VkCommandBufferBeginInfo begin_info;
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.pNext = NULL;
  begin_info.flags = 0;
  begin_info.pInheritanceInfo = NULL;

  err = vkBeginCommandBuffer(_cmd, &begin_info);
  if (err) {
    vulkan_error(err, "Can't begin command buffer");
    return false;
  }

  return true;
}

/**
 * Called between begin_frame() and end_frame() to mark the beginning of
 * drawing commands for a "scene" (usually a particular DisplayRegion) within
 * a frame.  All 3-D drawing commands, except the clear operation, must be
 * enclosed within begin_scene() .. end_scene(). This must be called in the
 * draw thread.
 *
 * The return value is true if successful (in which case the scene will be
 * drawn and end_scene() will be called later), or false if unsuccessful (in
 * which case nothing will be drawn and end_scene() will not be called).
 */
bool VulkanGraphicsStateGuardian::
begin_scene() {
  return GraphicsStateGuardian::begin_scene();
}

/**
 * Called between begin_frame() and end_frame() to mark the end of drawing
 * commands for a "scene" (usually a particular DisplayRegion) within a frame.
 * All 3-D drawing commands, except the clear operation, must be enclosed
 * within begin_scene() .. end_scene().
 */
void VulkanGraphicsStateGuardian::
end_scene() {
  GraphicsStateGuardian::end_scene();
}

/**
 * Called after each frame is rendered, to allow the GSG a chance to do any
 * internal cleanup after rendering the frame, and before the window flips.
 */
void VulkanGraphicsStateGuardian::
end_frame(Thread *current_thread) {
  GraphicsStateGuardian::end_frame(current_thread);

  nassertv(_cmd != VK_NULL_HANDLE);
  vkEndCommandBuffer(_cmd);

  //TODO: delete command buffer, schedule for deletion, or recycle.
}

/**
 * Resets all internal state as if the gsg were newly created.
 */
void VulkanGraphicsStateGuardian::
reset() {
}
