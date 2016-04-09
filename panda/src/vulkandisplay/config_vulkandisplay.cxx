/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_vulkandisplay.cxx
 * @author rdb
 * @date 2016-02-16
 */

#include "config_vulkandisplay.h"
#include "vulkanGraphicsPipe.h"
#include "vulkanGraphicsStateGuardian.h"
#include "vulkanGraphicsWindow.h"
#include "vulkanIndexBufferContext.h"
#include "vulkanSamplerContext.h"
#include "vulkanShaderContext.h"
#include "vulkanTextureContext.h"
#include "vulkanVertexBufferContext.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"
#include "pandaSystem.h"

Configure(config_vulkandisplay);
NotifyCategoryDef(vulkandisplay, "display");

ConfigureFn(config_vulkandisplay) {
  init_libvulkandisplay();
}

ConfigVariableInt vulkan_color_palette_size
("vulkan-color-palette-size", 256,
 PRC_DESC("This value indicates how many unique ColorAttrib values the "
          "Vulkan renderer should be prepared to encounter.  These values "
          "are stored in a palette that is 16 bytes per entry."));

#define VK_ERROR_INVALID_SHADER_NV -1000012000

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libvulkandisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  VulkanGraphicsPipe::init_type();
  VulkanGraphicsStateGuardian::init_type();
  VulkanGraphicsWindow::init_type();
  VulkanIndexBufferContext::init_type();
  VulkanSamplerContext::init_type();
  VulkanShaderContext::init_type();
  VulkanTextureContext::init_type();
  VulkanVertexBufferContext::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(VulkanGraphicsPipe::get_class_type(),
                           VulkanGraphicsPipe::pipe_constructor);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->set_system_tag("Vulkan", "native_window_system", "Win");
}

/**
 * Returns the TypeHandle index of the recommended graphics pipe type defined
 * by this module.  Which is VulkanGraphicsPipe of course, duh.
 */
int
get_pipe_type_p3vulkandisplay() {
  return VulkanGraphicsPipe::get_class_type().get_index();
}

/**
 * Returns a string describing a VkResult code, or NULL if it is unrecognized.
 */
static const char *const string_result(VkResult result) {
  switch (result) {
  case VK_SUCCESS:
    return "Success.";
  case VK_NOT_READY:
    return "Fence or query has not yet completed.";
  case VK_TIMEOUT:
    return "Wait operation has not completed in the specified time.";
  case VK_EVENT_SET:
    return "Event is signalled.";
  case VK_EVENT_RESET:
    return "Event is unsignalled.";
  case VK_INCOMPLETE:
    return "Return array too small for result.";
  case VK_ERROR_OUT_OF_HOST_MEMORY:
    return "Out of host memory.";
  case VK_ERROR_OUT_OF_DEVICE_MEMORY:
    return "Out of device memory.";
  case VK_ERROR_INITIALIZATION_FAILED:
    return "Initialization failed.";
  case VK_ERROR_DEVICE_LOST:
    return "Device lost.";
  case VK_ERROR_MEMORY_MAP_FAILED:
    return "Mapping of a memory object has failed.";
  case VK_ERROR_LAYER_NOT_PRESENT:
    return "Specified layer does not exist.";
  case VK_ERROR_EXTENSION_NOT_PRESENT:
    return "Specified extension does not exist.";
  case VK_ERROR_FEATURE_NOT_PRESENT:
    return "Requested feature not available.";
  case VK_ERROR_INCOMPATIBLE_DRIVER:
    return "Incompatible Vulkan driver.";
  case VK_ERROR_TOO_MANY_OBJECTS:
    return "Exhausted object limit.";
  case VK_ERROR_FORMAT_NOT_SUPPORTED:
    return "Requested format is not supported.";
  case VK_ERROR_SURFACE_LOST_KHR:
    return "Surface lost.";
  case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
    return "Native window is in use.";
  case VK_SUBOPTIMAL_KHR:
    return "Suboptimal.";
  case VK_ERROR_OUT_OF_DATE_KHR:
    return "Out of date.";
  case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
    return "Incompatible display.";
  case VK_ERROR_VALIDATION_FAILED_EXT:
    return "Validation failed.";
  case VK_ERROR_INVALID_SHADER_NV:
    return "Shader(s) failed to compile or link.";
  default:
    break;
  }
  return NULL;
}

/**
 * Formats a Vulkan error message.
 */
void vulkan_error(VkResult result, const char *message) {
  const char *const str = string_result(result);
  if (str == NULL) {
    // Unrecognized code.  Display error number.
    if (result < 0) {
      vulkandisplay_cat.error()
        << message << ": Unknown error " << result << "\n";
    } else {
      vulkandisplay_cat.error()
        << message << ": Unknown result " << result << "\n";
    }
  } else {
    vulkandisplay_cat.error()
      << message << ": " << str << "\n";
  }
}
