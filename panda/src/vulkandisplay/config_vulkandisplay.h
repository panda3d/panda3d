/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_vulkandisplay.h
 * @author rdb
 * @date 2016-02-16
 */

#ifndef CONFIG_VULKANDISPLAY_H
#define CONFIG_VULKANDISPLAY_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableInt64.h"

NotifyCategoryDecl(vulkandisplay, EXPCL_VULKANDISPLAY, EXPTP_VULKANDISPLAY);

extern ConfigVariableInt64 vulkan_memory_page_size;
extern ConfigVariableInt64 vulkan_global_uniform_buffer_size;
extern ConfigVariableInt64 vulkan_staging_buffer_size;
extern ConfigVariableBool vulkan_support_custom_border_color;

extern EXPCL_VULKANDISPLAY void init_libvulkandisplay();
extern "C" EXPCL_VULKANDISPLAY int get_pipe_type_p3vulkandisplay();

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR

#elif defined(HAVE_COCOA)
#define VK_USE_PLATFORM_METAL_EXT

#elif defined(HAVE_X11)
// Make sure Eigen is included first to avoid conflict
#include "lsimpleMatrix.h"
#include "pre_x11_include.h"
#define VK_USE_PLATFORM_XLIB_KHR
#endif

// Necessary for VK_KHR_portability_subset (MoltenVK)
#define VK_ENABLE_BETA_EXTENSIONS

#include <vulkan/vulkan.h>

#ifdef HAVE_X11
#include "post_x11_include.h"
#endif

void vulkan_error(VkResult result, const char *message);

/**
 * Determines how memory for an allocation is chosen.  This is not a hint:
 * every policy defines both an algorithm for selecting a memory type and a
 * binding contract about what the caller may assume and must do.
 */
enum VulkanMemoryPolicy {
  /**
   * Retained on the GPU and read there.  Guaranteed device-local, not
   * guaranteed to be host-visible (but may be so, especially in UMA).
   * Safe for data that is rewritten from time to time; it only forgoes
   * claiming scarce BAR memory, which is reserved for data that is rewritten
   * (nearly) every frame.
   */
  MP_retained,

  /**
   * Same contract as MP_retained, but for data rewritten by the CPU (nearly)
   * every frame: may claim host-visible (BAR) memory even if this is scarce.
   */
  MP_stream_retained,

  /**
   * Rewritten continuously by the CPU while the GPU reads it in place.
   * Guaranteed writable by the CPU (host-visible and host-coherent), with a
   * strong preference for placement in device-local BAR if possible.
   */
  MP_stream,

  /**
   * CPU-written transfer source, guaranteed host-visible and host-coherent,
   * preferably *not* device-local.
   */
  MP_upload_staging,

  /**
   * GPU-written, CPU-readable transfer destination, guaranteed host-visible,
   * with a preference for host-cached and a mild preference for host-coherent.
   */
  MP_readback_staging,
};

#endif
