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
  _noncoherent_atom_size(1),
  _linear_tiling(false),
  _persistent_ptr(nullptr) {
}

/**
 *
 */
INLINE VulkanMemoryPage::
VulkanMemoryPage(VkDevice device, VkDeviceMemory memory, VkDeviceSize size,
                 uint32_t type_index, VkFlags flags,
                 VkDeviceSize noncoherent_atom_size, Mutex &lock) :
  SimpleAllocator((size_t)size, lock),
  _device(device),
  _memory(memory),
  _type_index(type_index),
  _flags(flags),
  _noncoherent_atom_size(noncoherent_atom_size),
  _linear_tiling(false),
  _persistent_ptr(nullptr) {

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
  _noncoherent_atom_size(from._noncoherent_atom_size),
  _linear_tiling(from._linear_tiling),
  _persistent_ptr(from._persistent_ptr) {

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
    _persistent_ptr = nullptr;
  }
}

/**
 * Returns true if this page is of the given memory type and can satisfy the
 * given requirements.
 */
INLINE bool VulkanMemoryPage::
meets_requirements(const VkMemoryRequirements &reqs,
                   uint32_t type_index, bool linear_tiling) {
  return (_type_index == type_index &&
          _linear_tiling == linear_tiling &&
          reqs.size <= (VkDeviceSize)_max_size);
}

/**
 * Maps this memory page persistently.
 */
INLINE void *VulkanMemoryPage::
map_persistent() {
  if (_persistent_ptr == nullptr) {
    VkResult err = vkMapMemory(_device, _memory, 0, VK_WHOLE_SIZE, 0, &_persistent_ptr);
    if (err) {
      vulkan_error(err, "Failed to map memory");
      _persistent_ptr = nullptr;
    }
  }
  ++_persistent_ptr_count;
  return _persistent_ptr;
}

/**
 * Decreases the number of blocks that use a persistent mapping.  If it
 * reaches zero, unmaps the page.
 */
INLINE void VulkanMemoryPage::
unmap_persistent() {
  if (--_persistent_ptr_count == 0) {
    vkUnmapMemory(_device, _memory);
  }
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
 * Returns true if this block was allocated from host-visible memory.  Under
 * a retained allocation policy this is not guaranteed either way; use this
 * to determine whether the block may be written to directly or requires a
 * staging copy.
 */
INLINE bool VulkanMemoryBlock::
is_host_visible() const {
  nassertr(get_allocator() != nullptr, false);
  return (((VulkanMemoryPage *)get_allocator())->_flags &
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
}

/**
 * Returns true if this block was allocated from host-cached memory.  Reads
 * through a mapping of memory that is not host-cached (such as BAR memory)
 * are very slow; prefer a copy through MP_readback_staging memory instead.
 */
INLINE bool VulkanMemoryBlock::
is_host_cached() const {
  nassertr(get_allocator() != nullptr, false);
  return (((VulkanMemoryPage *)get_allocator())->_flags &
          VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0;
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
 * Only one block in the page can be mapped at any given time, except if the
 * entire memory page is enabled for persistent mapping.
 */
INLINE VulkanMemoryMapping VulkanMemoryBlock::
map(size_t start, size_t size) {
  VulkanMemoryPage *page = (VulkanMemoryPage *)get_allocator();
  VulkanMemoryMapping ptr(page);
  if (page->_persistent_ptr != nullptr) {
    ptr._data = (char *)page->_persistent_ptr + get_start() + start;
    return ptr;
  }
  ptr._device = page->_device;
  ptr._memory = page->_memory;
  VkDeviceSize begin = get_start() + start;
  VkDeviceSize end = begin + std::min(size, get_size() - start);
  VkDeviceSize slop = 0;
  if ((page->_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
    // Expand the mapping to nonCoherentAtomSize boundaries, so that a
    // subsequent flush() or invalidate() of the same range, which must round
    // the same way, stays within the mapped range.
    VkDeviceSize atom = page->_noncoherent_atom_size;
    slop = begin % atom;
    begin -= slop;
    end += (atom - end % atom) % atom;
    end = std::min(end, (VkDeviceSize)page->_max_size);
  }
  VkResult err = vkMapMemory(page->_device, page->_memory, begin,
                             end - begin, 0, &ptr._data);
  if (err) {
    vulkan_error(err, "Failed to map memory");
    ptr._data = nullptr;
  } else {
    ptr._data = (char *)ptr._data + slop;
  }
  return ptr;
}

/**
 * Maps the entire page this block is in persistently and return a pointer to
 * this block within the mapping.  Many blocks can be mapped at the same time.
 */
INLINE void *VulkanMemoryBlock::
map_persistent() {
  VulkanMemoryPage *page = (VulkanMemoryPage *)get_allocator();
  void *ptr = page->map_persistent();
  nassertr(ptr != nullptr, nullptr);
  return (char *)ptr + get_start();
}

/**
 * Opposite of map_persistent().
 */
INLINE void VulkanMemoryBlock::
unmap_persistent() {
  VulkanMemoryPage *page = (VulkanMemoryPage *)get_allocator();
  page->unmap_persistent();
}

/**
 * Computes the page range covered by the given range within this block,
 * expanded to nonCoherentAtomSize boundaries as vkFlushMappedMemoryRanges
 * and vkInvalidateMappedMemoryRanges require.  Returns false if the memory
 * is host-coherent, meaning no flush or invalidation is necessary.
 */
INLINE bool VulkanMemoryBlock::
get_noncoherent_range(VkMappedMemoryRange &range, size_t start, size_t size) const {
  VulkanMemoryPage *page = (VulkanMemoryPage *)get_allocator();
  if (page->_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
    return false;
  }
  // This must round exactly like map(), so that the range is guaranteed to
  // fall within a mapping made with the same start and size.  Blocks in
  // non-coherent pages are allocated at atom-aligned offsets, so the rounded
  // range cannot reach into a neighbouring block.
  VkDeviceSize atom = page->_noncoherent_atom_size;
  VkDeviceSize begin = get_start() + start;
  VkDeviceSize end = begin + std::min(size, get_size() - start);
  begin -= begin % atom;
  end += (atom - end % atom) % atom;
  // The end of the page is a valid range end even if not atom-aligned.
  end = std::min(end, (VkDeviceSize)page->_max_size);
  range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  range.pNext = nullptr;
  range.memory = page->_memory;
  range.offset = begin;
  range.size = end - begin;
  return true;
}

/**
 * Flushes any writes to mapped memory in the given range of this block.
 * Does nothing if the memory is host-coherent.  The range must currently be
 * mapped, via map() with the same start and size or a persistent mapping.
 */
INLINE void VulkanMemoryBlock::
flush(size_t start, size_t size) {
  VkMappedMemoryRange range;
  if (get_noncoherent_range(range, start, size)) {
    VulkanMemoryPage *page = (VulkanMemoryPage *)get_allocator();
    vkFlushMappedMemoryRanges(page->_device, 1, &range);
  }
}

/**
 * Invalidates the given range of mapped memory in this block.  Does nothing
 * if the memory is host-coherent.  The range must currently be mapped, via
 * map() with the same start and size or a persistent mapping.
 */
INLINE void VulkanMemoryBlock::
invalidate(size_t start, size_t size) {
  VkMappedMemoryRange range;
  if (get_noncoherent_range(range, start, size)) {
    VulkanMemoryPage *page = (VulkanMemoryPage *)get_allocator();
    vkInvalidateMappedMemoryRanges(page->_device, 1, &range);
  }
}

/**
 * Just grabs the page lock.
 */
INLINE VulkanMemoryMapping::
VulkanMemoryMapping(VulkanMemoryPage *page) :
  _holder(page->_lock) {
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
  from._memory = VK_NULL_HANDLE;
}

/**
 * Destructor, auto-unmaps if necessary.
 */
INLINE VulkanMemoryMapping::
~VulkanMemoryMapping() {
  if (_data != nullptr) {
    unmap();
  }
}

/**
 * Move assignment operator.
 */
INLINE VulkanMemoryMapping &VulkanMemoryMapping::
operator =(VulkanMemoryMapping &&from) noexcept {
  if (_data != nullptr) {
    unmap();
  }
  _holder = std::move(from._holder);
  _device = from._device;
  _memory = from._memory;
  _data = from._data;
  from._data = nullptr;
  from._memory = VK_NULL_HANDLE;
  return *this;
}

/**
 * Unmaps the memory.  Does nothing if the page is persistently mapped.
 */
INLINE void VulkanMemoryMapping::
unmap() {
  nassertv_always(_data != nullptr);
  if (_memory != VK_NULL_HANDLE) {
    vkUnmapMemory(_device, _memory);
    _memory = VK_NULL_HANDLE;
  }
  _data = nullptr;
  _holder.unlock();
}
