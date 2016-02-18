/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanGraphicsPipe.cxx
 * @author rdb
 * @date 2016-02-16
 */

#include "vulkanGraphicsPipe.h"
#include "vulkanGraphicsWindow.h"
#include "pandaVersion.h"

/**
 * Callback called by the VK_EXT_debug_report extension whenever one of the
 * validation layers has something to report.
 */
static VkBool32 debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char *prefix, const char *message, void *) {
  NotifySeverity severity;
  if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
    severity = NS_error;
  } else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
    severity = NS_warning;
  } else if (flags & (VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)) {
    severity = NS_info;
  } else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
    severity = NS_debug;
  } else {
    severity = NS_spam;
  }

  vulkandisplay_cat.out(severity) << message << "\n";

  // Return true here to fail validation, causing an error to be returned from
  // the validated function.
  return VK_FALSE;
}

TypeHandle VulkanGraphicsPipe::_type_handle;

/**
 * Creates a Vulkan instance and picks a GPU to use.
 */
VulkanGraphicsPipe::
VulkanGraphicsPipe() {
  _is_valid = false;

  const char *const layers[] = {
    "VK_LAYER_LUNARG_standard_validation",
  };

  const char *extensions[] = {
    "VK_EXT_debug_report",
    "VK_KHR_surface",
    "VK_KHR_win32_surface"
  };

  VkApplicationInfo app_info;
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pNext = NULL;
  app_info.pApplicationName = NULL;
  app_info.applicationVersion = 0;
  app_info.pEngineName = "Panda3D";
  app_info.engineVersion = PANDA_NUMERIC_VERSION;
  app_info.apiVersion = VK_API_VERSION;

  VkInstanceCreateInfo inst_info;
  inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  inst_info.pNext = NULL;
  inst_info.pApplicationInfo = &app_info;
  inst_info.enabledLayerCount = 1;
  inst_info.ppEnabledLayerNames = layers;
  inst_info.enabledExtensionCount = 3;
  inst_info.ppEnabledExtensionNames = extensions;

  VkResult err = vkCreateInstance(&inst_info, NULL, &_instance);
  if (err == VK_ERROR_INCOMPATIBLE_DRIVER) {
    vulkandisplay_cat.error()
      << "Cannot find a compatible Vulkan installable client driver (ICD).\n";
    return;

  } else if (err == VK_ERROR_EXTENSION_NOT_PRESENT) {
    vulkandisplay_cat.error()
      << "Cannot find a specified extension library.\n";
    return;

  } else if (err) {
    vulkan_error(err, "Failure to create Vulkan instance");
    return;
  }

  // Set a debug callback.
  PFN_vkCreateDebugReportCallbackEXT pvkCreateDebugReportCallback =
    (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");

  if (pvkCreateDebugReportCallback) {
    VkDebugReportCallbackCreateInfoEXT cb_info;
    cb_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    cb_info.pNext = NULL;
    cb_info.flags = 0;
    cb_info.pfnCallback = &debug_callback;
    cb_info.pUserData = NULL;

    // Tell the extension which severities to report, based on the enabled
    // notify categories.
    if (vulkandisplay_cat.is_debug()) {
      cb_info.flags |= VK_DEBUG_REPORT_DEBUG_BIT_EXT;
    }
    if (vulkandisplay_cat.is_info()) {
      cb_info.flags |= VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                       VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    }
    if (vulkandisplay_cat.is_warning()) {
      cb_info.flags |= VK_DEBUG_REPORT_WARNING_BIT_EXT;
    }
    if (vulkandisplay_cat.is_error()) {
      cb_info.flags |= VK_DEBUG_REPORT_ERROR_BIT_EXT;
    }

    VkDebugReportCallbackEXT callback;
    err = pvkCreateDebugReportCallback(_instance, &cb_info, NULL, &callback);
    if (err) {
      vulkan_error(err, "Failed to create debug report callback");
    }
  } else {
    vulkandisplay_cat.warning()
      << "Cannot find vkCreateDebugReportCallbackEXT function.  Debug "
         "reporting will not be enabled.\n";
  }

  // Enumerate the available GPUs.
  uint32_t gpu_count;
  err = vkEnumeratePhysicalDevices(_instance, &gpu_count, NULL);
  nassertv(!err && gpu_count > 0);

  VkPhysicalDevice *physical_devices =
    (VkPhysicalDevice *)alloca(sizeof(VkPhysicalDevice) * gpu_count);

  err = vkEnumeratePhysicalDevices(_instance, &gpu_count, physical_devices);
  if (err) {
    vulkan_error(err, "Failed to enumerate GPUs");
    return;
  }

  // Just pick the first GPU for now.
  _gpu = physical_devices[0];

  // Query memory properties.
  vkGetPhysicalDeviceMemoryProperties(_gpu, &_memory_properties);

  // Query queue information.  Currently unused, but keeps validator happy.
  uint32_t num_families;
  vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &num_families, NULL);
  _queue_families.resize(num_families);
  vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &num_families, &_queue_families[0]);

  _is_valid = true;
}

/**
 * Cleans up the Vulkan instance.
 */
VulkanGraphicsPipe::
~VulkanGraphicsPipe() {
}

/**
 * Finds the index of the memory type that fits the given requirements.
 */
bool VulkanGraphicsPipe::
find_memory_type(uint32_t &type_index, uint32_t type_bits, VkFlags required_flags) const {
  for (int i = 0; type_bits > 0; ++i) {
    if ((type_bits & 1) == 1) {
      // Type is available.  Does it match the required properties?
      if ((_memory_properties.memoryTypes[i].propertyFlags & required_flags) == required_flags) {
        type_index = i;
        return true;
      }
    }
    type_bits >>= 1;
  }

  // Not found.
  return false;
}

/**
 * Returns the name of the rendering interface associated with this
 * GraphicsPipe.  This is used to present to the user to allow him/her to
 * choose between several possible GraphicsPipes available on a particular
 * platform, so the name should be meaningful and unique for a given platform.
 */
string VulkanGraphicsPipe::
get_interface_name() const {
  return "Vulkan";
}

/**
 * This function is passed to the GraphicsPipeSelection object to allow the
 * user to make a default VulkanGraphicsPipe.
 */
PT(GraphicsPipe) VulkanGraphicsPipe::
pipe_constructor() {
  return new VulkanGraphicsPipe;
}

/**
 * Creates a new window or buffer on the pipe, if possible.  This routine is
 * only called from GraphicsEngine::make_output.
 */
PT(GraphicsOutput) VulkanGraphicsPipe::
make_output(const string &name,
            const FrameBufferProperties &fb_prop,
            const WindowProperties &win_prop,
            int flags,
            GraphicsEngine *engine,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {

  if (!_is_valid) {
    return NULL;
  }

  VulkanGraphicsStateGuardian *vkgsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(vkgsg, gsg, NULL);
  }

  // First thing to try: a VulkanGraphicsWindow

  if (retry == 0) {
    if ((flags & BF_require_parasite) != 0 ||
        (flags & BF_refuse_window) != 0 ||
        (flags & BF_resizeable) != 0 ||
        (flags & BF_size_track_host) != 0 ||
        (flags & BF_rtt_cumulative) != 0 ||
        (flags & BF_can_bind_color) != 0 ||
        (flags & BF_can_bind_every) != 0 ||
        (flags & BF_can_bind_layered) != 0) {
      return NULL;
    }
    return new VulkanGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                    flags, gsg, host);
  }


  // Nothing else left to try.
  return NULL;
}

/**
 * This is called when make_output() is used to create a
 * CallbackGraphicsWindow.  If the GraphicsPipe can construct a GSG that's not
 * associated with any particular window object, do so now, assuming the
 * correct graphics context has been set up externally.
 */
PT(GraphicsStateGuardian) VulkanGraphicsPipe::
make_callback_gsg(GraphicsEngine *engine) {
  return new VulkanGraphicsStateGuardian(engine, this, NULL);
}
