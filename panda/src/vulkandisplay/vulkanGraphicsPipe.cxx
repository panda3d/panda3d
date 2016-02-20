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
#include "displayInformation.h"

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
  } else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
    severity = NS_debug;
  } else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
    // I'd label this as 'info', but the output is just way too spammy.
    severity = NS_spam;
  } else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
    severity = NS_spam;
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

  // Query device limits and memory properties.
  vkGetPhysicalDeviceFeatures(_gpu, &_gpu_features);
  vkGetPhysicalDeviceProperties(_gpu, &_gpu_properties);
  vkGetPhysicalDeviceMemoryProperties(_gpu, &_memory_properties);

  // Query queue information, used by find_queue_family_for_surface.
  uint32_t num_families;
  vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &num_families, NULL);
  _queue_families.resize(num_families);
  vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &num_families, &_queue_families[0]);

  // Fill in DisplayInformation.
  _display_information->_vendor_id = _gpu_properties.vendorID;
  _display_information->_device_id = _gpu_properties.deviceID;

  if (vulkandisplay_cat.is_debug()) {
    // Output some properties about this device.
    vulkandisplay_cat.debug() << "apiVersion: "
      << ((_gpu_properties.apiVersion >> 22) & 0x3ff) << '.'
      << ((_gpu_properties.apiVersion >> 12) & 0x3ff) << '.'
      << (_gpu_properties.apiVersion & 0xfff) << '\n';

    if (_gpu_properties.vendorID == 0x10DE) {
      // This is how NVIDIA encodes their driver version.
      vulkandisplay_cat.debug()
        << "driverVersion: " << _gpu_properties.driverVersion << " ("
        << ((_gpu_properties.driverVersion >> 22) & 0x3ff) << '.'
        << ((_gpu_properties.driverVersion >> 14) & 0x0ff);

      if (_gpu_properties.driverVersion & 0x3fc0) {
        vulkandisplay_cat.debug(false) << '.'
          << ((_gpu_properties.driverVersion >>  6) & 0x0ff) << ")\n";
      } else {
        vulkandisplay_cat.debug(false) << ")\n";
      }
    } else {
      vulkandisplay_cat.debug()
        << "driverVersion: " << _gpu_properties.driverVersion << "\n";
    }

    char vendor_id[5], device_id[5];
    sprintf(vendor_id, "%04X", _gpu_properties.vendorID);
    sprintf(device_id, "%04X", _gpu_properties.deviceID);

    // Display the vendor name, if the vendor ID is recognized.
    const char *vendor_name = get_vendor_name();
    if (vendor_name != NULL) {
      vulkandisplay_cat.debug() << "vendorID: 0x" << vendor_id
                                << " (" << vendor_name << ")\n";
    } else {
      vulkandisplay_cat.debug() << "vendorID: 0x" << vendor_id << "\n";
    }
    vulkandisplay_cat.debug() << "deviceID: 0x" << device_id << "\n";

    static const char *const device_types[] = {
      "OTHER", "INTEGRATED_GPU", "DISCRETE_GPU", "VIRTUAL_GPU", "CPU", ""
    };
    vulkandisplay_cat.debug()
      << "deviceType: VK_PHYSICAL_DEVICE_TYPE_"
      << device_types[_gpu_properties.deviceType] << "\n";

    vulkandisplay_cat.debug() << "deviceName: " << _gpu_properties.deviceName << "\n";

    // Enumerate supported extensions.
    uint32_t num_inst_extensions = 0, num_dev_extensions = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &num_inst_extensions, NULL);
    vkEnumerateDeviceExtensionProperties(_gpu, NULL, &num_dev_extensions, NULL);

    VkExtensionProperties *extensions = (VkExtensionProperties *)
      alloca(sizeof(VkExtensionProperties) * max(num_inst_extensions, num_dev_extensions));

    vulkandisplay_cat.debug() << "Supported instance extensions:\n";
    vkEnumerateInstanceExtensionProperties(NULL, &num_inst_extensions, extensions);
    for (uint32_t i = 0; i < num_inst_extensions; ++i) {
      vulkandisplay_cat.debug() << "  " << extensions[i].extensionName << "\n";
    }

    vulkandisplay_cat.debug() << "Supported device extensions:\n";
    vkEnumerateDeviceExtensionProperties(_gpu, NULL, &num_dev_extensions, extensions);
    for (uint32_t i = 0; i < num_dev_extensions; ++i) {
      vulkandisplay_cat.debug() << "  " << extensions[i].extensionName << "\n";
    }

    // Get a list of supported texture formats.
    vulkandisplay_cat.debug() << "Supported texture formats:\n";

    static const char *const format_strings[] = {
      "UNDEFINED",
      "R4G4_UNORM_PACK8",
      "R4G4B4A4_UNORM_PACK16",
      "B4G4R4A4_UNORM_PACK16",
      "R5G6B5_UNORM_PACK16",
      "B5G6R5_UNORM_PACK16",
      "R5G5B5A1_UNORM_PACK16",
      "B5G5R5A1_UNORM_PACK16",
      "A1R5G5B5_UNORM_PACK16",
      "R8_UNORM",
      "R8_SNORM",
      "R8_USCALED",
      "R8_SSCALED",
      "R8_UINT",
      "R8_SINT",
      "R8_SRGB",
      "R8G8_UNORM",
      "R8G8_SNORM",
      "R8G8_USCALED",
      "R8G8_SSCALED",
      "R8G8_UINT",
      "R8G8_SINT",
      "R8G8_SRGB",
      "R8G8B8_UNORM",
      "R8G8B8_SNORM",
      "R8G8B8_USCALED",
      "R8G8B8_SSCALED",
      "R8G8B8_UINT",
      "R8G8B8_SINT",
      "R8G8B8_SRGB",
      "B8G8R8_UNORM",
      "B8G8R8_SNORM",
      "B8G8R8_USCALED",
      "B8G8R8_SSCALED",
      "B8G8R8_UINT",
      "B8G8R8_SINT",
      "B8G8R8_SRGB",
      "R8G8B8A8_UNORM",
      "R8G8B8A8_SNORM",
      "R8G8B8A8_USCALED",
      "R8G8B8A8_SSCALED",
      "R8G8B8A8_UINT",
      "R8G8B8A8_SINT",
      "R8G8B8A8_SRGB",
      "B8G8R8A8_UNORM",
      "B8G8R8A8_SNORM",
      "B8G8R8A8_USCALED",
      "B8G8R8A8_SSCALED",
      "B8G8R8A8_UINT",
      "B8G8R8A8_SINT",
      "B8G8R8A8_SRGB",
      "A8B8G8R8_UNORM_PACK32",
      "A8B8G8R8_SNORM_PACK32",
      "A8B8G8R8_USCALED_PACK32",
      "A8B8G8R8_SSCALED_PACK32",
      "A8B8G8R8_UINT_PACK32",
      "A8B8G8R8_SINT_PACK32",
      "A8B8G8R8_SRGB_PACK32",
      "A2R10G10B10_UNORM_PACK32",
      "A2R10G10B10_SNORM_PACK32",
      "A2R10G10B10_USCALED_PACK32",
      "A2R10G10B10_SSCALED_PACK32",
      "A2R10G10B10_UINT_PACK32",
      "A2R10G10B10_SINT_PACK32",
      "A2B10G10R10_UNORM_PACK32",
      "A2B10G10R10_SNORM_PACK32",
      "A2B10G10R10_USCALED_PACK32",
      "A2B10G10R10_SSCALED_PACK32",
      "A2B10G10R10_UINT_PACK32",
      "A2B10G10R10_SINT_PACK32",
      "R16_UNORM",
      "R16_SNORM",
      "R16_USCALED",
      "R16_SSCALED",
      "R16_UINT",
      "R16_SINT",
      "R16_SFLOAT",
      "R16G16_UNORM",
      "R16G16_SNORM",
      "R16G16_USCALED",
      "R16G16_SSCALED",
      "R16G16_UINT",
      "R16G16_SINT",
      "R16G16_SFLOAT",
      "R16G16B16_UNORM",
      "R16G16B16_SNORM",
      "R16G16B16_USCALED",
      "R16G16B16_SSCALED",
      "R16G16B16_UINT",
      "R16G16B16_SINT",
      "R16G16B16_SFLOAT",
      "R16G16B16A16_UNORM",
      "R16G16B16A16_SNORM",
      "R16G16B16A16_USCALED",
      "R16G16B16A16_SSCALED",
      "R16G16B16A16_UINT",
      "R16G16B16A16_SINT",
      "R16G16B16A16_SFLOAT",
      "R32_UINT",
      "R32_SINT",
      "R32_SFLOAT",
      "R32G32_UINT",
      "R32G32_SINT",
      "R32G32_SFLOAT",
      "R32G32B32_UINT",
      "R32G32B32_SINT",
      "R32G32B32_SFLOAT",
      "R32G32B32A32_UINT",
      "R32G32B32A32_SINT",
      "R32G32B32A32_SFLOAT",
      "R64_UINT",
      "R64_SINT",
      "R64_SFLOAT",
      "R64G64_UINT",
      "R64G64_SINT",
      "R64G64_SFLOAT",
      "R64G64B64_UINT",
      "R64G64B64_SINT",
      "R64G64B64_SFLOAT",
      "R64G64B64A64_UINT",
      "R64G64B64A64_SINT",
      "R64G64B64A64_SFLOAT",
      "B10G11R11_UFLOAT_PACK32",
      "E5B9G9R9_UFLOAT_PACK32",
      "D16_UNORM",
      "X8_D24_UNORM_PACK32",
      "D32_SFLOAT",
      "S8_UINT",
      "D16_UNORM_S8_UINT",
      "D24_UNORM_S8_UINT",
      "D32_SFLOAT_S8_UINT",
      "BC1_RGB_UNORM_BLOCK",
      "BC1_RGB_SRGB_BLOCK",
      "BC1_RGBA_UNORM_BLOCK",
      "BC1_RGBA_SRGB_BLOCK",
      "BC2_UNORM_BLOCK",
      "BC2_SRGB_BLOCK",
      "BC3_UNORM_BLOCK",
      "BC3_SRGB_BLOCK",
      "BC4_UNORM_BLOCK",
      "BC4_SNORM_BLOCK",
      "BC5_UNORM_BLOCK",
      "BC5_SNORM_BLOCK",
      "BC6H_UFLOAT_BLOCK",
      "BC6H_SFLOAT_BLOCK",
      "BC7_UNORM_BLOCK",
      "BC7_SRGB_BLOCK",
      "ETC2_R8G8B8_UNORM_BLOCK",
      "ETC2_R8G8B8_SRGB_BLOCK",
      "ETC2_R8G8B8A1_UNORM_BLOCK",
      "ETC2_R8G8B8A1_SRGB_BLOCK",
      "ETC2_R8G8B8A8_UNORM_BLOCK",
      "ETC2_R8G8B8A8_SRGB_BLOCK",
      "EAC_R11_UNORM_BLOCK",
      "EAC_R11_SNORM_BLOCK",
      "EAC_R11G11_UNORM_BLOCK",
      "EAC_R11G11_SNORM_BLOCK",
      "ASTC_4x4_UNORM_BLOCK",
      "ASTC_4x4_SRGB_BLOCK",
      "ASTC_5x4_UNORM_BLOCK",
      "ASTC_5x4_SRGB_BLOCK",
      "ASTC_5x5_UNORM_BLOCK",
      "ASTC_5x5_SRGB_BLOCK",
      "ASTC_6x5_UNORM_BLOCK",
      "ASTC_6x5_SRGB_BLOCK",
      "ASTC_6x6_UNORM_BLOCK",
      "ASTC_6x6_SRGB_BLOCK",
      "ASTC_8x5_UNORM_BLOCK",
      "ASTC_8x5_SRGB_BLOCK",
      "ASTC_8x6_UNORM_BLOCK",
      "ASTC_8x6_SRGB_BLOCK",
      "ASTC_8x8_UNORM_BLOCK",
      "ASTC_8x8_SRGB_BLOCK",
      "ASTC_10x5_UNORM_BLOCK",
      "ASTC_10x5_SRGB_BLOCK",
      "ASTC_10x6_UNORM_BLOCK",
      "ASTC_10x6_SRGB_BLOCK",
      "ASTC_10x8_UNORM_BLOCK",
      "ASTC_10x8_SRGB_BLOCK",
      "ASTC_10x10_UNORM_BLOCK",
      "ASTC_10x10_SRGB_BLOCK",
      "ASTC_12x10_UNORM_BLOCK",
      "ASTC_12x10_SRGB_BLOCK",
      "ASTC_12x12_UNORM_BLOCK",
      "ASTC_12x12_SRGB_BLOCK",
    };

    for (int i = 1; i < VK_FORMAT_RANGE_SIZE; ++i) {
      VkFormatProperties fmt_props;
      vkGetPhysicalDeviceFormatProperties(_gpu, (VkFormat)i, &fmt_props);

      if (fmt_props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
        vulkandisplay_cat.debug()
          << "  VK_FORMAT_" << format_strings[i] << "\n";
      }
    }

    // Print some more limits.
    vulkandisplay_cat.debug() << "maxImageDimension1D = "
      << _gpu_properties.limits.maxImageDimension1D << "\n";
    vulkandisplay_cat.debug() << "maxImageDimension2D = "
      << _gpu_properties.limits.maxImageDimension2D << "\n";
    vulkandisplay_cat.debug() << "maxImageDimension3D = "
      << _gpu_properties.limits.maxImageDimension3D << "\n";
    vulkandisplay_cat.debug() << "maxImageDimensionCube = "
      << _gpu_properties.limits.maxImageDimensionCube << "\n";
    vulkandisplay_cat.debug() << "maxImageArrayLayers = "
      << _gpu_properties.limits.maxImageArrayLayers << "\n";
    vulkandisplay_cat.debug() << "maxTexelBufferElements = "
      << _gpu_properties.limits.maxTexelBufferElements << "\n";
    vulkandisplay_cat.debug() << "maxPushConstantsSize = "
      << _gpu_properties.limits.maxPushConstantsSize << "\n";
    vulkandisplay_cat.debug() << "maxMemoryAllocationCount = "
      << _gpu_properties.limits.maxMemoryAllocationCount << "\n";
    vulkandisplay_cat.debug() << "maxSamplerAllocationCount = "
      << _gpu_properties.limits.maxSamplerAllocationCount << "\n";
    vulkandisplay_cat.debug() << "maxColorAttachments = "
      << _gpu_properties.limits.maxColorAttachments << "\n";
    vulkandisplay_cat.debug() << "maxDrawIndexedIndexValue = "
      << _gpu_properties.limits.maxDrawIndexedIndexValue << "\n";
  }

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
 * @return true if a matching memory type was found
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
 * Finds the index of the queue family capable of presenting to the given
 * surface that fits the given requirements.
 * @return true if a matching queue family was found
 */
bool VulkanGraphicsPipe::
find_queue_family_for_surface(uint32_t &queue_family_index, VkSurfaceKHR surface, VkFlags required_flags) const {
  // Iterate over each queue to learn whether it supports presenting.
  for (uint32_t i = 0; i < _queue_families.size(); ++i) {
    VkBool32 supports_present;
    vkGetPhysicalDeviceSurfaceSupportKHR(_gpu, i, surface, &supports_present);

    if ((_queue_families[i].queueFlags & required_flags) == required_flags) {
      queue_family_index = i;
      return true;
    }
  }

  return false;
}

/**
 * Returns the name of the vendor of the physical device.
 */
const char *VulkanGraphicsPipe::
get_vendor_name() const {
  // Match OpenGL vendor for consistency.
  switch (_gpu_properties.vendorID) {
  case 0x1002: return "ATI Technologies Inc.";
  case 0x10DE: return "NVIDIA Corporation";
  case 0x5143: return "Qualcomm";
  case 0x8086: return "Intel";
  default:
    return NULL;
  }
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
  // For now, grab the first queue family that can supports graphics commands.
  uint32_t i;
  for (i = 0; i < _queue_families.size(); ++i) {
    if (_queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      break;
    }
  }
  if (i == _queue_families.size()) {
    return NULL;
  }
  return new VulkanGraphicsStateGuardian(engine, this, NULL, i);
}
