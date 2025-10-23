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
#include "vulkanGraphicsBuffer.h"
#include "pandaVersion.h"
#include "displayInformation.h"
#include "small_vector.h"

/**
 * Callback called by the VK_EXT_debug_utils extension whenever one of the
 * validation layers has something to report.
 */
static VkBool32 debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT flags, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *data, void *) {
  // When updating this, be sure to also update the code in the GraphicsPipe
  // constructor that assigns the correct mask bits.
  NotifySeverity severity;
  if (flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    severity = NS_error;
  } else if (flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    severity = NS_warning;
  } else if (flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    // I'd label this as 'info', but the output is just way too spammy.
    severity = NS_spam;
  } else {
    severity = NS_spam;
  }

  vulkandisplay_cat.out(severity) << data->pMessage << "\n";

  // Return true here to fail validation, causing an error to be returned from
  // the validated function.
  return VK_FALSE;
}

TypeHandle VulkanGraphicsPipe::_type_handle;

/**
 * Creates a Vulkan instance and picks a GPU to use.
 */
VulkanGraphicsPipe::
VulkanGraphicsPipe() : _max_allocation_size(0) {
  _is_valid = false;
  _has_surface_ext = false;
  _vkSetDebugUtilsObjectName = nullptr;

  // Get the API version number for instance-level functionality.
  uint32_t inst_version = VK_API_VERSION_1_0;
  PFN_vkEnumerateInstanceVersion pvkEnumerateInstanceVersion =
    (PFN_vkEnumerateInstanceVersion)vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion");
  if (pvkEnumerateInstanceVersion != nullptr) {
    pvkEnumerateInstanceVersion(&inst_version);
  }

  // Query supported layers, and enable the ones we want.
  small_vector<const char *> layers;

  uint32_t num_inst_layers = 0;
  vkEnumerateInstanceLayerProperties(&num_inst_layers, nullptr);

  VkLayerProperties *inst_layers = nullptr;
  if (num_inst_layers > 0) {
    inst_layers = (VkLayerProperties *)
      alloca(sizeof(VkLayerProperties) * num_inst_layers);

    vkEnumerateInstanceLayerProperties(&num_inst_layers, inst_layers);
    for (uint32_t i = 0; i < num_inst_layers; ++i) {
      if (strcmp(inst_layers[i].layerName, "VK_LAYER_KHRONOS_validation") == 0) {
        layers.push_back("VK_LAYER_KHRONOS_validation");
      }
    }
  }

  // Query supported instance extensions.
  uint32_t num_inst_extensions = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &num_inst_extensions, nullptr);

  VkExtensionProperties *inst_extensions = nullptr;
  if (num_inst_extensions > 0) {
    inst_extensions = (VkExtensionProperties *)
      alloca(sizeof(VkExtensionProperties) * num_inst_extensions);

    vkEnumerateInstanceExtensionProperties(nullptr, &num_inst_extensions, inst_extensions);
    for (uint32_t i = 0; i < num_inst_extensions; ++i) {
      _instance_extensions[std::string(inst_extensions[i].extensionName)] = inst_extensions[i].specVersion;
    }
  }

  std::vector<const char *> extensions;

  bool has_debug = false;
  if (has_instance_extension("VK_EXT_debug_utils")) {
    extensions.push_back("VK_EXT_debug_utils");
    has_debug = true;
  }

#ifdef _WIN32
  if (has_instance_extension("VK_KHR_win32_surface")) {
    extensions.push_back("VK_KHR_surface");
    extensions.push_back("VK_KHR_win32_surface");
    _has_surface_ext = true;
  }
#elif defined(HAVE_COCOA)
  if (has_instance_extension("VK_EXT_metal_surface")) {
    extensions.push_back("VK_KHR_surface");
    extensions.push_back("VK_EXT_metal_surface");
    _has_surface_ext = true;
  }
#elif defined(HAVE_X11)
  if (has_instance_extension("VK_KHR_xlib_surface")) {
    extensions.push_back("VK_KHR_surface");
    extensions.push_back("VK_KHR_xlib_surface");
    _has_surface_ext = true;
  }
#endif

  bool has_props2_ext = false;
  if (inst_version < VK_MAKE_VERSION(1, 1, 0) &&
      has_instance_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    has_props2_ext = true;
  }

  bool has_portability_enum_ext = false;
  if (has_instance_extension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    has_portability_enum_ext = true;
  }

  VkApplicationInfo app_info;
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pNext = nullptr;
  app_info.pApplicationName = nullptr;
  app_info.applicationVersion = 0;
  app_info.pEngineName = "Panda3D";
  app_info.engineVersion = PANDA_NUMERIC_VERSION;
  app_info.apiVersion = VK_API_VERSION_1_0;

  if (inst_version >= VK_MAKE_VERSION(1, 3, 0)) {
    app_info.apiVersion = VK_MAKE_VERSION(1, 3, 0);
  }

  VkInstanceCreateInfo inst_info;
  inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  inst_info.pNext = nullptr;
  inst_info.flags = 0;
  inst_info.pApplicationInfo = &app_info;
  inst_info.enabledLayerCount = layers.size();
  inst_info.ppEnabledLayerNames = &layers[0];
  inst_info.enabledExtensionCount = extensions.size();
  inst_info.ppEnabledExtensionNames = &extensions[0];

  if (has_portability_enum_ext) {
    inst_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
  }

  VkResult err = vkCreateInstance(&inst_info, nullptr, &_instance);
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

  // Set up the debugging extensions.
  if (has_debug) {
    _vkSetDebugUtilsObjectName = (PFN_vkSetDebugUtilsObjectNameEXT)
      vkGetInstanceProcAddr(_instance, "vkSetDebugUtilsObjectNameEXT");

    PFN_vkCreateDebugUtilsMessengerEXT pvkCreateDebugUtilsMessenger =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");

    if (pvkCreateDebugUtilsMessenger) {
      VkDebugUtilsMessengerCreateInfoEXT info = {};
      info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                       | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                       | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      info.pfnUserCallback = &debug_callback;

      // Tell the extension which severities to report, based on the enabled
      // notify categories.
      if (vulkandisplay_cat.is_spam()) {
        info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
      }
      if (vulkandisplay_cat.is_warning()) {
        info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
      }
      if (vulkandisplay_cat.is_error()) {
        info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      }

      VkDebugUtilsMessengerEXT messenger;
      err = pvkCreateDebugUtilsMessenger(_instance, &info, nullptr, &messenger);
      if (err) {
        vulkan_error(err, "Failed to create debug report callback");
      }
    } else {
      vulkandisplay_cat.warning()
        << "Cannot find vkCreateDebugUtilsMessengerEXT function.  Debug "
           "reporting will not be enabled.\n";
    }
  }

  // Enumerate the available GPUs.
  uint32_t gpu_count;
  err = vkEnumeratePhysicalDevices(_instance, &gpu_count, nullptr);
  nassertv(!err && gpu_count > 0);

  VkPhysicalDevice *physical_devices =
    (VkPhysicalDevice *)alloca(sizeof(VkPhysicalDevice) * gpu_count);

  err = vkEnumeratePhysicalDevices(_instance, &gpu_count, physical_devices);
  if (err) {
    vulkan_error(err, "Failed to enumerate GPUs");
    return;
  }

#ifndef NDEBUG
  static const char *const device_types[] = {
    "OTHER", "INTEGRATED_GPU", "DISCRETE_GPU", "VIRTUAL_GPU", "CPU", ""
  };
  if (vulkandisplay_cat.is_debug()) {
    vulkandisplay_cat.debug()
      << gpu_count << " physical adapter" << (gpu_count != 1 ? "s" : "") << " found:\n";

    for (uint32_t i = 0; i < gpu_count; ++i) {
      VkPhysicalDeviceProperties gpu_props;
      vkGetPhysicalDeviceProperties(physical_devices[i], &gpu_props);

      vulkandisplay_cat.debug()
        << "  " << gpu_props.deviceName << " (" << device_types[gpu_props.deviceType] << ")\n";
    }
  }
#endif  // !NDEBUG

  // Time to pick a GPU.  For now, we simply check for a discrete GPU, but
  // this should probably be more complex in the future.
  _gpu = physical_devices[0];

  for (uint32_t i = 0; i < gpu_count; ++i) {
    VkPhysicalDeviceProperties gpu_props;
    vkGetPhysicalDeviceProperties(physical_devices[i], &gpu_props);

    if (gpu_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      // Great, we found a discrete GPU, use that.
      _gpu = physical_devices[i];
      break;
    }

    if (gpu_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
      // Great, we'll use this as fallback, but keep looking.
      _gpu = physical_devices[i];
    }
  }

  // Query queue information, used by find_queue_family_for_surface.
  uint32_t num_families;
  vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &num_families, nullptr);
  _queue_families.resize(num_families);
  vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &num_families, &_queue_families[0]);

  // Query supported device extensions.
  uint32_t num_dev_extensions;
  vkEnumerateDeviceExtensionProperties(_gpu, nullptr, &num_dev_extensions, nullptr);

  VkExtensionProperties *dev_extensions = (VkExtensionProperties *)
    alloca(sizeof(VkExtensionProperties) * num_dev_extensions);

  vkEnumerateDeviceExtensionProperties(_gpu, nullptr, &num_dev_extensions, dev_extensions);
  for (uint32_t i = 0; i < num_dev_extensions; ++i) {
    _device_extensions[std::string(dev_extensions[i].extensionName)] = dev_extensions[i].specVersion;
  }

  // Query device limits and memory properties.
  vkGetPhysicalDeviceProperties(_gpu, &_gpu_properties);
  vkGetPhysicalDeviceMemoryProperties(_gpu, &_memory_properties);

  PFN_vkGetPhysicalDeviceFeatures2 pVkGetPhysicalDeviceFeatures2 = nullptr;

  if (_gpu_properties.apiVersion >= VK_MAKE_VERSION(1, 1, 0)) {
    pVkGetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2)
      vkGetInstanceProcAddr(_instance, "vkGetPhysicalDeviceFeatures2");
  }
  else if (has_props2_ext) {
    pVkGetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2)
      vkGetInstanceProcAddr(_instance, "vkGetPhysicalDeviceFeatures2KHR");
  }

  if (pVkGetPhysicalDeviceFeatures2 != nullptr) {
    VkPhysicalDeviceFeatures2 features2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

    VkPhysicalDeviceDynamicRenderingFeatures dr_features = {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
      features2.pNext,
    };
    if (_gpu_properties.apiVersion >= VK_MAKE_VERSION(1, 3, 0) ||
        has_device_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)) {
      features2.pNext = &dr_features;
    }

    VkPhysicalDeviceCustomBorderColorFeaturesEXT cbc_features = {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT,
      features2.pNext,
    };
    if (has_device_extension(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME)) {
      features2.pNext = &cbc_features;
    }

    VkPhysicalDeviceRobustness2FeaturesKHR ro2_features = {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_KHR,
      features2.pNext,
    };
    if (has_device_extension(VK_KHR_ROBUSTNESS_2_EXTENSION_NAME) ||
        has_device_extension(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME)) {
      features2.pNext = &ro2_features;
    }

    VkPhysicalDeviceVertexAttributeDivisorFeaturesKHR div_features = {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_KHR,
      features2.pNext,
    };
    if (has_device_extension(VK_KHR_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME) ||
        has_device_extension(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME)) {
      features2.pNext = &div_features;
    }

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT eds_features = {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
      features2.pNext,
    };
    if (has_device_extension(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME)) {
      features2.pNext = &eds_features;
    }

    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT eds2_features = {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
      features2.pNext,
    };
    if (has_device_extension(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME)) {
      features2.pNext = &eds2_features;
    }

    VkPhysicalDeviceInlineUniformBlockFeatures iub_features = {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES,
      features2.pNext,
    };
    if (_gpu_properties.apiVersion >= VK_MAKE_VERSION(1, 3, 0) ||
        has_device_extension(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME)) {
      features2.pNext = &iub_features;
    }

    pVkGetPhysicalDeviceFeatures2(_gpu, &features2);
    _gpu_features = features2.features;
    _gpu_supports_dynamic_rendering = dr_features.dynamicRendering;
    _gpu_supports_custom_border_colors = cbc_features.customBorderColors
                                      && cbc_features.customBorderColorWithoutFormat;
    _gpu_supports_null_descriptor = ro2_features.nullDescriptor;
    _gpu_supports_vertex_attrib_divisor = div_features.vertexAttributeInstanceRateDivisor;
    _gpu_supports_vertex_attrib_zero_divisor = div_features.vertexAttributeInstanceRateZeroDivisor;
    if (_gpu_properties.apiVersion >= VK_MAKE_VERSION(1, 3, 0)) {
      _gpu_supports_extended_dynamic_state = true;
      _gpu_supports_extended_dynamic_state2 = true;
    } else {
      _gpu_supports_extended_dynamic_state = eds_features.extendedDynamicState;
      _gpu_supports_extended_dynamic_state2 = eds2_features.extendedDynamicState2;
    }
    _gpu_supports_extended_dynamic_state2_patch_control_points = eds2_features.extendedDynamicState2PatchControlPoints;
    _gpu_supports_inline_uniform_block = iub_features.inlineUniformBlock;
  }
  else {
    vkGetPhysicalDeviceFeatures(_gpu, &_gpu_features);
    _gpu_supports_dynamic_rendering = false;
    _gpu_supports_custom_border_colors = false;
    _gpu_supports_null_descriptor = false;
    _gpu_supports_vertex_attrib_divisor = false;
    _gpu_supports_vertex_attrib_zero_divisor = false;
    _gpu_supports_extended_dynamic_state = false;
    _gpu_supports_extended_dynamic_state2 = false;
    _gpu_supports_extended_dynamic_state2_patch_control_points = false;
    _gpu_supports_inline_uniform_block = false;
  }

  // Default the maximum allocation size to the largest of the heaps.
  for (uint32_t i = 0; i < _memory_properties.memoryHeapCount; ++i) {
    VkMemoryHeap &heap = _memory_properties.memoryHeaps[i];
    _max_allocation_size = std::max(_max_allocation_size, heap.size);
  }

  VkPhysicalDeviceDriverProperties driver_props = {
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES, nullptr,
  };
  if (_gpu_properties.apiVersion > VK_MAKE_VERSION(1, 1, 0) || has_props2_ext) {
    VkPhysicalDeviceProperties2 props2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, &driver_props};
    VkPhysicalDeviceMaintenance3Properties maint3_props;
    maint3_props.maxMemoryAllocationSize = _max_allocation_size;

    // Query more specific memory limits if available.
    if (_gpu_properties.apiVersion >= VK_MAKE_VERSION(1, 1, 0) ||
        has_device_extension("VK_KHR_maintenance3")) {
      maint3_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES;
      maint3_props.pNext = nullptr;
      maint3_props.maxPerSetDescriptors = 0;
      maint3_props.maxMemoryAllocationSize = _max_allocation_size;
      driver_props.pNext = &maint3_props;
    }

    VkPhysicalDeviceInlineUniformBlockProperties iub_props = {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES,
      props2.pNext,
    };
    if (_gpu_supports_inline_uniform_block) {
      props2.pNext = &iub_props;
    }

    if (_gpu_properties.apiVersion >= VK_MAKE_VERSION(1, 1, 0)) {
      PFN_vkGetPhysicalDeviceProperties2 pVkGetPhysicalDeviceProperties2 =
        (PFN_vkGetPhysicalDeviceProperties2)vkGetInstanceProcAddr(_instance, "vkGetPhysicalDeviceProperties2");

      if (pVkGetPhysicalDeviceProperties2 != nullptr) {
        pVkGetPhysicalDeviceProperties2(_gpu, &props2);
      }
    } else {
      PFN_vkGetPhysicalDeviceProperties2KHR pVkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vkGetInstanceProcAddr(_instance, "vkGetPhysicalDeviceProperties2KHR");

      if (pVkGetPhysicalDeviceProperties2KHR != nullptr) {
        pVkGetPhysicalDeviceProperties2KHR(_gpu, &props2);
      }
    }

    _max_allocation_size = maint3_props.maxMemoryAllocationSize;
    _max_inline_uniform_block_size = iub_props.maxInlineUniformBlockSize;
  }

  // Fill in DisplayInformation.
  _display_information->_vendor_id = _gpu_properties.vendorID;
  _display_information->_device_id = _gpu_properties.deviceID;

#ifndef NDEBUG
  if (vulkandisplay_cat.is_debug()) {
    // Output some properties about this device.
    vulkandisplay_cat.debug() << "apiVersion: "
      << ((_gpu_properties.apiVersion >> 22) & 0x3ff) << '.'
      << ((_gpu_properties.apiVersion >> 12) & 0x3ff) << '.'
      << (_gpu_properties.apiVersion & 0xfff) << '\n';

    if (driver_props.driverID == VK_DRIVER_ID_NVIDIA_PROPRIETARY ||
        (driver_props.driverID == 0 && _gpu_properties.vendorID == 0x10DE)) {
      // This is how NVIDIA encodes their driver version.
      vulkandisplay_cat.debug()
        << "driverVersion: " << _gpu_properties.driverVersion << " ("
        << ((_gpu_properties.driverVersion >> 22) & 0x3ff) << '.'
        << ((_gpu_properties.driverVersion >> 14) & 0x0ff);

      if (_gpu_properties.driverVersion & 0x3fff) {
        vulkandisplay_cat.debug(false) << '.'
          << ((_gpu_properties.driverVersion >> 6) & 0xff);

        if (_gpu_properties.driverVersion & 0x3f) {
          vulkandisplay_cat.debug(false) << '.'
            << (_gpu_properties.driverVersion & 0x3f);
        }
      }
      vulkandisplay_cat.debug(false) << ")\n";

    } else {
      vulkandisplay_cat.debug()
        << "driverVersion: " << _gpu_properties.driverVersion << "\n";
    }

    if (driver_props.driverID != 0) {
      vulkandisplay_cat.debug()
        << "driverID: " << driver_props.driverID << "\n";
    }
    if (driver_props.driverName[0] != 0) {
      vulkandisplay_cat.debug()
        << "driverName: " << driver_props.driverName << "\n";
    }
    if (driver_props.driverInfo[0] != 0) {
      vulkandisplay_cat.debug()
        << "driverInfo: " << driver_props.driverInfo << "\n";
    }

    char vendor_id[5], device_id[5];
    sprintf(vendor_id, "%04X", _gpu_properties.vendorID);
    sprintf(device_id, "%04X", _gpu_properties.deviceID);

    // Display the vendor name, if the vendor ID is recognized.
    const char *vendor_name = get_vendor_name();
    if (vendor_name != nullptr) {
      vulkandisplay_cat.debug() << "vendorID: 0x" << vendor_id
                                << " (" << vendor_name << ")\n";
    } else {
      vulkandisplay_cat.debug() << "vendorID: 0x" << vendor_id << "\n";
    }
    vulkandisplay_cat.debug() << "deviceID: 0x" << device_id << "\n";

    vulkandisplay_cat.debug()
      << "deviceType: VK_PHYSICAL_DEVICE_TYPE_"
      << device_types[_gpu_properties.deviceType] << "\n";

    vulkandisplay_cat.debug() << "deviceName: " << _gpu_properties.deviceName << "\n";
    size_t size_mb = _max_allocation_size >> 20u;
    vulkandisplay_cat.debug() << "maxMemoryAllocationSize: " << (size_mb) << " MiB\n";

    // Show supported queue families
    vulkandisplay_cat.debug() << "Supported queue families:\n";
    uint32_t i = 0;
    for (const VkQueueFamilyProperties &props : _queue_families) {
      vulkandisplay_cat.debug() << "  " << (i++) << ": ";
      if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        vulkandisplay_cat.debug(false) << "GRAPHICS ";
      }
      if (props.queueFlags & VK_QUEUE_COMPUTE_BIT) {
        vulkandisplay_cat.debug(false) << "COMPUTE ";
      }
      if (props.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
        vulkandisplay_cat.debug(false) << "SPARSE_BINDING ";
      }
      if (props.queueFlags & VK_QUEUE_PROTECTED_BIT) {
        vulkandisplay_cat.debug(false) << "PROTECTED ";
      }
      vulkandisplay_cat.debug(false) << "(" << props.queueCount
        << " queue" << ((props.queueCount != 1) ? "s" : "") << ")\n";
    }

    // Show memory heaps.
    vulkandisplay_cat.debug() << "Available memory heaps:\n";
    for (uint32_t i = 0; i < _memory_properties.memoryHeapCount; ++i) {
      VkMemoryHeap &heap = _memory_properties.memoryHeaps[i];
      size_t size_mb = heap.size >> 20u;
      vulkandisplay_cat.debug() << "  " << i << ": " << (size_mb) << " MiB";

      if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
        vulkandisplay_cat.debug(false) << ", device-local";
      }
      if (heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) {
        vulkandisplay_cat.debug(false) << ", multi-instance";
      }
      vulkandisplay_cat.debug(false) << "\n";

      for (size_t ti = 0; ti < _memory_properties.memoryTypeCount; ++ti) {
        const VkMemoryType &type = _memory_properties.memoryTypes[ti];
        if (type.heapIndex == i) {
          std::ostream &out = vulkandisplay_cat.debug();
          out << "    Type " << ti << ":";
          if (type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            out << " DEVICE_LOCAL";
          }
          if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            out << " HOST_VISIBLE";
          }
          if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
            out << " HOST_COHERENT";
          }
          if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
            out << " HOST_CACHED";
          }
          if (type.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
            out << " LAZILY_ALLOCATED";
          }
          if (type.propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) {
            out << " PROTECTED";
          }
          if (type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) {
            out << " DEVICE_COHERENT";
          }
          if (type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD) {
            out << " DEVICE_UNCACHED";
          }
          if (type.propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV) {
            out << " RDMA_CAPABLE";
          }
          out << "\n";
        }
      }
    }

    // Enumerate supported layers and extensions.
    vulkandisplay_cat.debug() << "Supported instance layers:\n";
    for (uint32_t i = 0; i < num_inst_layers; ++i) {
      vulkandisplay_cat.debug() << "  " << inst_layers[i].layerName << "\n";
    }

    vulkandisplay_cat.debug() << "Supported instance extensions:\n";
    for (uint32_t i = 0; i < num_inst_extensions; ++i) {
      vulkandisplay_cat.debug() << "  " << inst_extensions[i].extensionName << "\n";
    }

    vulkandisplay_cat.debug() << "Supported device extensions:\n";
    for (uint32_t i = 0; i < num_dev_extensions; ++i) {
      vulkandisplay_cat.debug() << "  " << dev_extensions[i].extensionName << "\n";
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

    for (int i = 1; i <= VK_FORMAT_ASTC_12x12_SRGB_BLOCK; ++i) {
      VkFormatProperties fmt_props;
      vkGetPhysicalDeviceFormatProperties(_gpu, (VkFormat)i, &fmt_props);

      if ((fmt_props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) == 0) {
        continue;
      }

      VkImageFormatProperties props;
      vkGetPhysicalDeviceImageFormatProperties(_gpu, (VkFormat)i, VK_IMAGE_TYPE_2D,
                                               VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_SAMPLED_BIT,
                                               0, &props);

      if (props.maxExtent.width == 0) {
        continue;
      }

      if (props.maxExtent.width != _gpu_properties.limits.maxImageDimension2D ||
          props.maxExtent.height != _gpu_properties.limits.maxImageDimension2D) {
        // This format is supported, but has custom limits.
        vulkandisplay_cat.debug()
          << "  VK_FORMAT_" << format_strings[i] << " (max "
          << props.maxExtent.width << 'x' << props.maxExtent.height << ")\n";
      } else {
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
    vulkandisplay_cat.debug() << "bufferImageGranularity = "
      << _gpu_properties.limits.bufferImageGranularity << "\n";
    vulkandisplay_cat.debug() << "minUniformBufferOffsetAlignment = "
      << _gpu_properties.limits.minUniformBufferOffsetAlignment << "\n";
    vulkandisplay_cat.debug() << "maxColorAttachments = "
      << _gpu_properties.limits.maxColorAttachments << "\n";
    vulkandisplay_cat.debug() << "maxDrawIndexedIndexValue = "
      << _gpu_properties.limits.maxDrawIndexedIndexValue << "\n";
  }
#endif  // !NDEBUG

  _is_valid = true;

#ifdef HAVE_COCOA
  _supported_types = OT_window | OT_buffer | OT_texture_buffer;
#endif
}

/**
 * Cleans up the Vulkan instance.
 */
VulkanGraphicsPipe::
~VulkanGraphicsPipe() {
}

/**
 * Sets the name of a Vulkan object.  Useful for debugging.
 */
void VulkanGraphicsPipe::
set_object_name(VkDevice device, VkObjectType type, void *object, const char *name) {
  if (_vkSetDebugUtilsObjectName != nullptr) {
    VkDebugUtilsObjectNameInfoEXT info = {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = type;
    info.objectHandle = (uint64_t)object;
    info.pObjectName = name;
    _vkSetDebugUtilsObjectName(device, &info);
  }
}

/**
 * Returns true if the given instance extension is supported.
 */
bool VulkanGraphicsPipe::
has_instance_extension(const char *ext_name, uint32_t min_version) {
  auto it = _instance_extensions.find(std::string(ext_name));
  if (it != _instance_extensions.end()) {
    return (it->second >= min_version);
  } else {
    return false;
  }
}

/**
 * Returns true if the given device extension is supported.
 */
bool VulkanGraphicsPipe::
has_device_extension(const char *ext_name, uint32_t min_version) {
  auto it = _device_extensions.find(std::string(ext_name));
  if (it != _device_extensions.end()) {
    return (it->second >= min_version);
  } else {
    return false;
  }
}

/**
 * Finds the index of the memory type that fits the given requirements.
 * @return true if a matching memory type was found
 */
bool VulkanGraphicsPipe::
find_memory_type(uint32_t &type_index, const VkMemoryRequirements &reqs, VkFlags required_flags) const {
  // Does this fit within the maximum allocation size?
  if (reqs.size > _max_allocation_size) {
    return false;
  }

  uint32_t type_bits = reqs.memoryTypeBits;
  for (int i = 0; type_bits > 0; ++i) {
    if ((type_bits & 1) == 1) {
      // Type is available.  Does it match the required properties?
      const VkMemoryType &type = _memory_properties.memoryTypes[i];
      if ((type.propertyFlags & required_flags) == required_flags) {
        type_index = i;

        // Is the heap large enough to fit the allocation?
        if (reqs.size <= _memory_properties.memoryHeaps[type.heapIndex].size) {
          return true;
        }
      }
    }
    type_bits >>= 1;
  }

  // Not found.
  return false;
}

/**
 * Finds the index of the queue family matching the given requirements.
 * @return true if a matching queue family was found
 */
bool VulkanGraphicsPipe::
find_queue_family(uint32_t &queue_family_index, VkFlags required_flags) const {
  for (uint32_t i = 0; i < _queue_families.size(); ++i) {
    if ((_queue_families[i].queueFlags & required_flags) == required_flags) {
      queue_family_index = i;
      return true;
    }
  }

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
  // These are PCI vendor IDs.
  case 0x1002: return "ATI Technologies Inc.";
  case 0x10DE: return "NVIDIA Corporation";
  case 0x5143: return "Qualcomm";
  case 0x8086: return "Intel";

  // Khronos vendor IDs for vendors without PCI.  See vk.xml.
  case 0x10001: return "Vivante Corporation";
  case 0x10002: return "VeriSilicon";
  case 0x10003: return "Kazan";
  case 0x10004: return "Codeplay Software Ltd.";
  case 0x10005: return "Mesa";
  case 0x10006: return "PoCL";
  case 0x10007: return "Mobileye";
  default:
    return nullptr;
  }
}

/**
 * Returns the name of the rendering interface associated with this
 * GraphicsPipe.  This is used to present to the user to allow him/her to
 * choose between several possible GraphicsPipes available on a particular
 * platform, so the name should be meaningful and unique for a given platform.
 */
std::string VulkanGraphicsPipe::
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
make_output(const std::string &name,
            const FrameBufferProperties &fb_prop,
            const WindowProperties &win_prop,
            int flags,
            GraphicsEngine *engine,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {

  if (!_is_valid) {
    return nullptr;
  }

  VulkanGraphicsStateGuardian *vkgsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(vkgsg, gsg, nullptr);
  }

  // First thing to try: a VulkanGraphicsWindow

  if (retry == 0) {
    if (!_has_surface_ext ||
        (flags & BF_require_parasite) != 0 ||
        (flags & BF_refuse_window) != 0 ||
        (flags & BF_resizeable) != 0 ||
        (flags & BF_size_track_host) != 0 ||
        (flags & BF_rtt_cumulative) != 0 ||
        (flags & BF_can_bind_color) != 0 ||
        (flags & BF_can_bind_every) != 0 ||
        (flags & BF_can_bind_layered) != 0) {
      return nullptr;
    }
    return new VulkanGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                    flags, gsg, host);
  }

  // Second thing: a VulkanGraphicsBuffer

  if (retry == 1) {
    if ((flags & BF_require_parasite) != 0 ||
        (flags & BF_require_window) != 0 ||
        (flags & BF_rtt_cumulative) != 0) {
      return nullptr;
    }
    return new VulkanGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                    flags, gsg, host);
  }

  // Nothing else left to try.
  return nullptr;
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
    return nullptr;
  }
  return new VulkanGraphicsStateGuardian(engine, this, nullptr, i);
}
