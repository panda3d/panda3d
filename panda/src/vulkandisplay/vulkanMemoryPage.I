/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanMemoryPage.I
 * @author rdb
 * @date 2018-06-30
 */


/**
 * Initializes the page to an invalid state, for later move-assignment.
 */
INLINE VulkanMemoryPage::
VulkanMemoryPage(Mutex &lock) :
  SimpleAllocator(0, lock),
  _device(VK_NULL_HANDLE),
  _memory(VK_NULL_HANDLE),
  _type_index(0),
  _flags(0),
  _linear_tiling(false) {
}

/**
 *
 */
INLINE VulkanMemoryPage::
VulkanMemoryPage(VkDevice device, VkDeviceMemory memory, VkDeviceSize size,
                 uint32_t type_index, VkFlags flags, Mutex &lock) :
  SimpleAllocator((size_t)size, lock),
  _device(device),
  _memory(memory),
  _type_index(type_index),
  _flags(flags),
  _linear_tiling(false) {

  nassertv(_next == this && _prev == this);
}

/**
 * Move constructor.
 */
INLINE VulkanMemoryPage::
VulkanMemoryPage(VulkanMemoryPage &&from) :
  SimpleAllocator(std::move(from)),
  _device(from._device),
  _memory(from._memory),
  _type_index(from._type_index),
  _flags(from._flags),
  _linear_tiling(from._linear_tiling) {

  // Prevent double free.
  from._memory = VK_NULL_HANDLE;
}

/**
 * Releases the memory held by this page.
 */
INLINE VulkanMemoryPage::
~VulkanMemoryPage() {
  if (_memory != VK_NULL_HANDLE) {
    vkFreeMemory(_device, _memory, nullptr);
    _memory = VK_NULL_HANDLE;
  }
}

/**
 * Returns true if memory in this page satisfies the given requirements.
 */
INLINE bool VulkanMemoryPage::
meets_requirements(const VkMemoryRequirements &reqs, VkFlags required_flags,
                   bool linear_tiling) {
  return ((reqs.memoryTypeBits & (1 << _type_index)) != 0 &&
          (required_flags & _flags) == required_flags &&
          _linear_tiling == linear_tiling &&
          reqs.size <= (VkDeviceSize)get_max_size());
}

/**
 * Returns the VkDeviceMemory of the page this block is located in.
 */
INLINE VkDeviceMemory VulkanMemoryBlock::
get_memory() {
  nassertr(get_allocator() != nullptr, VK_NULL_HANDLE);
  return ((VulkanMemoryPage *)get_allocator())->_memory;
}

/**
 * Binds this memory to the given image.
 */
INLINE bool VulkanMemoryBlock::
bind_image(VkImage image) {
  VulkanMemoryPage *page = (VulkanMemoryPage *)get_allocator();
  nassertr(page != nullptr, false);
  VkResult err = vkBindImageMemory(page->_device, image, page->_memory, get_start());
  if (err) {
    vulkan_error(err, "Failed to bind memory to image");
    return false;
  }
  return true;
}

/**
 * Binds this memory to the given buffer.
 */
INLINE bool VulkanMemoryBlock::
bind_buffer(VkBuffer buffer) {
  VulkanMemoryPage *page = (VulkanMemoryPage *)get_allocator();
  nassertr(page != nullptr, false);
  nassertr(page->_linear_tiling, false);
  VkResult err = vkBindBufferMemory(page->_device, buffer, page->_memory, get_start());
  if (err) {
    vulkan_error(err, "Failed to bind memory to buffer");
    return false;
  }
  return true;
}

/**
 * Maps this block and returns the mapped memory address in a RAII wrapper.
 * Only one block in the page can be mapped at any given time.
 */
INLINE VulkanMemoryMapping VulkanMemoryBlock::
map() {
  VulkanMemoryPage *page = (VulkanMemoryPage *)get_allocator();
  VulkanMemoryMapping ptr(page);
  assert(!ptr);
  VkResult err = vkMapMemory(page->_device, page->_memory, get_start(), get_size(), 0, &ptr._data);
  if (err) {
    vulkan_error(err, "Failed to map memory");
    ptr._data = nullptr;
  }
  return std::move(ptr);
}

/**
 * Move constructor.
 */
INLINE VulkanMemoryMapping::
VulkanMemoryMapping(VulkanMemoryMapping &&from) noexcept :
  _holder(std::move(from._holder)),
  _device(from._device),
  _memory(from._memory),
  _data(from._data) {

  // Prevent double unmap
  from._data = nullptr;
}
