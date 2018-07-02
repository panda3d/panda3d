/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanMemoryPage.h
 * @author rdb
 * @date 2018-06-30
 */

#include "config_vulkandisplay.h"
#include "simpleAllocator.h"

#include <mutex>

class VulkanMemoryMapping;

/**
 * A single page of allocated Vulkan memory.
 */
class VulkanMemoryPage final : public SimpleAllocator {
public:
  INLINE VulkanMemoryPage(Mutex &lock);
  INLINE VulkanMemoryPage(VkDevice device, VkDeviceMemory memory, VkDeviceSize size,
                          uint32_t type_index, VkFlags flags, Mutex &lock);
  INLINE VulkanMemoryPage(VulkanMemoryPage &&from);
  INLINE ~VulkanMemoryPage();

  //INLINE VulkanMemoryPage &operator = (VulkanMemoryPage &&from);

  INLINE bool meets_requirements(const VkMemoryRequirements &reqs,
                                 VkFlags required_flags = 0,
                                 bool linear_tiling = false);

private:
  VkDevice _device;
  VkDeviceMemory _memory;
  uint32_t _type_index;
  VkFlags _flags;
  bool _linear_tiling;

  friend class VulkanGraphicsStateGuardian;
  friend class VulkanMemoryBlock;
  friend class VulkanMemoryMapping;
};

/**
 * A single block of memory sub-allocated from a VulkanMemoryPage.
 */
class VulkanMemoryBlock final : public SimpleAllocatorBlock {
public:
  VulkanMemoryBlock() = default;
  VulkanMemoryBlock(VulkanMemoryPage *page) :
    SimpleAllocatorBlock(page, 0, page->get_max_size()) {}
  VulkanMemoryBlock(SimpleAllocatorBlock &&block) :
    SimpleAllocatorBlock(std::move(block)) {}

  INLINE VkDeviceMemory get_memory();

  INLINE bool bind_image(VkImage image);
  INLINE bool bind_buffer(VkBuffer buffer);
  INLINE VulkanMemoryMapping map();

  friend class VulkanMemoryPage;
};

/**
 * RAII container for a mapped pointer that automatically unmaps it when it
 * goes out of scope.  Also holds the page lock.
 */
class VulkanMemoryMapping {
public:
  VulkanMemoryMapping(VulkanMemoryPage *page) :
    _holder(page->_lock),
    _device(page->_device),
    _memory(page->_memory) {
  }

  VulkanMemoryMapping(const VulkanMemoryMapping &copy) = delete;
  INLINE VulkanMemoryMapping(VulkanMemoryMapping &&from) noexcept;

  ~VulkanMemoryMapping() {
    if (_data != nullptr) {
      unmap();
    }
  }

  void unmap() {
    nassertv_always(_data != nullptr);
    vkUnmapMemory(_device, _memory);
    _data = nullptr;
    _holder.unlock();
  }

  explicit operator bool() {
    return _data != nullptr;
  }

  operator void *() {
    return _data;
  }

  operator uint8_t *() {
    return (uint8_t *)_data;
  }

  operator uint16_t *() {
    return (uint16_t *)_data;
  }

  operator uint32_t *() {
    return (uint32_t *)_data;
  }

  std::unique_lock<Mutex> _holder;
  VkDevice _device;
  VkDeviceMemory _memory;
  void *_data = nullptr;
};

#include "vulkanMemoryPage.I"
