/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanGraphicsStateGuardian.h
 * @author rdb
 * @date 2016-02-16
 */

#ifndef VULKANGRAPHICSSTATEGUARDIAN_H
#define VULKANGRAPHICSSTATEGUARDIAN_H

#include "config_vulkandisplay.h"
#include "vulkanFrameData.h"
#include "vulkanMemoryPage.h"
#include "vulkanShaderContext.h"
#include "circularAllocator.h"

class VulkanBufferContext;
class VulkanIndexBufferContext;
class VulkanSamplerContext;
class VulkanShaderContext;
class VulkanTextureContext;
class VulkanVertexBufferContext;

/**
 * Manages a Vulkan device, and manages sending render commands to this
 * device.
 */
class VulkanGraphicsStateGuardian final : public GraphicsStateGuardian {
private:
  typedef VulkanFrameData FrameData;

public:
  VulkanGraphicsStateGuardian(GraphicsEngine *engine, VulkanGraphicsPipe *pipe,
                              VulkanGraphicsStateGuardian *share_with,
                              uint32_t queue_family_index);
  virtual ~VulkanGraphicsStateGuardian();

  virtual void reset();

  void destroy_device();
  virtual void close_gsg();

  virtual std::string get_driver_vendor();
  virtual std::string get_driver_renderer();
  virtual std::string get_driver_version();

  bool allocate_memory(VulkanMemoryBlock &block, const VkMemoryRequirements &reqs,
                       VkFlags required_flags, bool linear);

  VulkanTextureContext *use_texture(Texture *texture, VkImageLayout layout,
                                    VkPipelineStageFlags stage_mask,
                                    VkAccessFlags access_mask,
                                    bool discard=false);
  virtual TextureContext *prepare_texture(Texture *tex);
  bool create_texture(VulkanTextureContext *vtc);
  bool upload_texture(VulkanTextureContext *vtc);
  virtual bool update_texture(TextureContext *tc, bool force);
  virtual void release_texture(TextureContext *tc);
  virtual bool extract_texture_data(Texture *tex);

  virtual SamplerContext *prepare_sampler(const SamplerState &sampler);
  virtual void release_sampler(SamplerContext *sc);

  virtual GeomContext *prepare_geom(Geom *geom);
  virtual void release_geom(GeomContext *gc);

  virtual ShaderContext *prepare_shader(Shader *shader);
  virtual void release_shader(ShaderContext *sc);

  virtual VertexBufferContext *prepare_vertex_buffer(GeomVertexArrayData *data);
  bool update_vertex_buffer(VulkanVertexBufferContext *vbc,
                            const GeomVertexArrayDataHandle *reader,
                            bool force);
  virtual void release_vertex_buffer(VertexBufferContext *vbc);

  virtual IndexBufferContext *prepare_index_buffer(GeomPrimitive *data);
  bool update_index_buffer(VulkanIndexBufferContext *ibc,
                           const GeomPrimitivePipelineReader *reader,
                           bool force);
  virtual void release_index_buffer(IndexBufferContext *ibc);

  virtual BufferContext *prepare_shader_buffer(ShaderBuffer *data);
  virtual void release_shader_buffer(BufferContext *bc);

  virtual void dispatch_compute(int size_x, int size_y, int size_z);

  virtual PT(GeomMunger) make_geom_munger(const RenderState *state,
                                          Thread *current_thread);

  virtual void set_state_and_transform(const RenderState *state,
                                       const TransformState *transform);

  virtual void clear(DrawableRegion *clearable);
  virtual void prepare_display_region(DisplayRegionPipelineReader *dr);
  virtual CPT(TransformState) calc_projection_mat(const Lens *lens);
  virtual bool prepare_lens();

  virtual bool begin_frame(Thread *current_thread);
  virtual bool begin_scene();
  virtual void end_scene();
  virtual void end_frame(Thread *current_thread);
  void end_frame(Thread *current_thread, VkSemaphore wait_for, VkSemaphore signal_done);
  void finish_frame(FrameData &frame_data);
  INLINE FrameData &get_frame_data();

  virtual bool begin_draw_primitives(const GeomPipelineReader *geom_reader,
                                     const GeomVertexDataPipelineReader *data_reader,
                                     size_t num_instances, bool force);
  virtual bool draw_triangles(const GeomPrimitivePipelineReader *reader,
                              bool force);
  virtual bool draw_triangles_adj(const GeomPrimitivePipelineReader *reader,
                                  bool force);
  virtual bool draw_tristrips(const GeomPrimitivePipelineReader *reader,
                              bool force);
  virtual bool draw_tristrips_adj(const GeomPrimitivePipelineReader *reader,
                                  bool force);
  virtual bool draw_trifans(const GeomPrimitivePipelineReader *reader,
                            bool force);
  virtual bool draw_patches(const GeomPrimitivePipelineReader *reader,
                            bool force);
  virtual bool draw_lines(const GeomPrimitivePipelineReader *reader,
                          bool force);
  virtual bool draw_lines_adj(const GeomPrimitivePipelineReader *reader,
                              bool force);
  virtual bool draw_linestrips(const GeomPrimitivePipelineReader *reader,
                               bool force);
  virtual bool draw_linestrips_adj(const GeomPrimitivePipelineReader *reader,
                                   bool force);
  virtual bool draw_points(const GeomPrimitivePipelineReader *reader,
                           bool force);
  virtual void end_draw_primitives();

  virtual bool framebuffer_copy_to_texture(Texture *tex, int view, int z,
                                           const DisplayRegion *dr,
                                           const RenderBuffer &rb);
  virtual bool framebuffer_copy_to_ram(Texture *tex, int view, int z,
                                       const DisplayRegion *dr,
                                       const RenderBuffer &rb,
                                       ScreenshotRequest *request = nullptr);

private:
  bool do_extract_image(VulkanTextureContext *tc, Texture *tex, int view, int z=-1,
                        ScreenshotRequest *request = nullptr);

  bool do_draw_primitive_with_topology(const GeomPrimitivePipelineReader *reader,
                                      bool force, VkPrimitiveTopology topology,
                                      bool primitive_restart_enable);
  bool do_draw_primitive(const GeomPrimitivePipelineReader *reader, bool force);

public:
  bool create_buffer(VkDeviceSize size, VkBuffer &buffer, VulkanMemoryBlock &block,
                     int usage_flags, VkMemoryPropertyFlagBits flags);
  bool create_image(VulkanTextureContext *tc, VkImageType type, VkFormat format,
                    const VkExtent3D &extent, uint32_t levels, uint32_t layers,
                    VkSampleCountFlagBits samples, VkImageUsageFlags usage,
                    VkImageCreateFlags flags = 0);

  VkSemaphore create_semaphore();

  VkPipeline make_pipeline(VulkanShaderContext *sc,
                           const VulkanShaderContext::PipelineKey &key);
  VkPipeline make_compute_pipeline(VulkanShaderContext *sc);

  // Built-in descriptor set indices, ordered by frequency.  Static descriptor
  // sets come first.  We separate descriptor sets by the RenderAttrib that
  // contains those inputs, so we can swap out only the set that has changed.
  // Note that Vulkan only guarantees that 4 sets can be bound simultaneously,
  // though most implementations support at least 8.
  enum DescriptorSetIndex : uint32_t {
    DS_light_attrib = 0,
    DS_texture_attrib = 1,
    DS_shader_attrib = 2,

    DS_ATTRIB_COUNT = 3,

    // This one is used for other shader inputs and uses dynamic offsets.
    DS_dynamic_uniforms = 3,

    DS_SET_COUNT = 4,
  };

  bool get_attrib_descriptor_set(VkDescriptorSet &out, VkDescriptorSetLayout layout,
                                 const RenderAttrib *attrib);

  bool update_lattr_descriptor_set(VkDescriptorSet ds, const LightAttrib *attr);
  bool update_dynamic_uniform_descriptor_set(VulkanShaderContext *sc);
  void *alloc_dynamic_uniform_buffer(VkDeviceSize size, VkBuffer &buffer, uint32_t &offset);

  void *alloc_staging_buffer(VkDeviceSize size, VkBuffer &buffer, uint32_t &offset);

  VkFormat get_image_format(const Texture *texture) const;
  static bool lookup_image_format(VkFormat vk_format, Texture::Format &format,
                                  Texture::ComponentType &type);

public:
  VkDevice _device = VK_NULL_HANDLE;
  uint32_t _graphics_queue_family_index;
  PT(Texture) _white_texture;

private:
  VkQueue _queue = VK_NULL_HANDLE;
  VkQueue _dma_queue = VK_NULL_HANDLE;
  VkCommandPool _cmd_pool = VK_NULL_HANDLE;
  pvector<VkRect2D> _viewports;
  VkPipelineCache _pipeline_cache = VK_NULL_HANDLE;
  VkDescriptorPool _descriptor_pool = VK_NULL_HANDLE;
  VulkanShaderContext *_default_sc = nullptr;
  VulkanShaderContext *_current_shader = nullptr;
  const ShaderType::Struct *_push_constant_block_type = nullptr;
  CPT(GeomVertexFormat) _format;
  uint32_t _instance_count = 0;

  // Single large uniform buffer used for everything in a frame.
  VkBuffer _uniform_buffer = VK_NULL_HANDLE;
  VulkanMemoryBlock _uniform_buffer_memory;
  CircularAllocator _uniform_buffer_allocator;
  void *_uniform_buffer_ptr = nullptr;
  VkDescriptorSet _uniform_descriptor_set;
  VkDeviceSize _uniform_buffer_max_used = 0;
  uint32_t _uniform_buffer_white_offset = 0;
  VkBuffer _current_color_buffer = VK_NULL_HANDLE;
  uint32_t _current_color_offset = 0;

  // Staging buffer for CPU-to-GPU uploads.
  VkBuffer _staging_buffer = VK_NULL_HANDLE;
  VulkanMemoryBlock _staging_buffer_memory;
  CircularAllocator _staging_buffer_allocator;
  void *_staging_buffer_ptr = nullptr;
  bool _has_unified_memory = false;

  // Stores current framebuffer info.
  VkRenderPass _render_pass = VK_NULL_HANDLE;
  VulkanTextureContext *_fb_color_tc = nullptr;
  VulkanTextureContext *_fb_depth_tc = nullptr;
  VkSampleCountFlagBits _fb_ms_count = VK_SAMPLE_COUNT_1_BIT;

  // Static "null" vertex buffer if nullDescriptor is not supported.
  VkBuffer _null_vertex_buffer = VK_NULL_HANDLE;
  VulkanMemoryBlock _null_vertex_memory;
  bool _needs_write_null_vertex_data = false;

  // Descriptor set layouts used for the LightAttrib descriptor sets.
  // The others are shader-dependent and stored in VulkanShaderContext.
  VkSampler _shadow_sampler;
  VkDescriptorSetLayout _lattr_descriptor_set_layout;

  // Keep track of all the individual allocations.
  Mutex _allocator_lock;
  pdeque<VulkanMemoryPage> _memory_pages;
  VkDeviceSize _total_allocated = 0u;

  static const size_t _frame_data_capacity = 5;
  FrameData _frame_data_pool[_frame_data_capacity];
  size_t _frame_data_head = _frame_data_capacity;
  size_t _frame_data_tail = 0;
  FrameData *_frame_data = nullptr;
  FrameData *_last_frame_data = nullptr;

  uint64_t _frame_counter = 0;
  uint64_t _last_finished_frame = 0;

  // Feature checks.
  bool _supports_custom_border_colors = false;
  bool _supports_vertex_attrib_divisor = false;
  bool _supports_vertex_attrib_zero_divisor = false;
  bool _supports_extended_dynamic_state2 = false;
  bool _supports_extended_dynamic_state2_patch_control_points = false;

  // Function pointers.
  PFN_vkCmdBindIndexBuffer _vkCmdBindIndexBuffer;
  PFN_vkCmdBindPipeline _vkCmdBindPipeline;
  PFN_vkCmdBindVertexBuffers _vkCmdBindVertexBuffers;
  PFN_vkCmdDraw _vkCmdDraw;
  PFN_vkCmdDrawIndexed _vkCmdDrawIndexed;
  PFN_vkCmdPushConstants _vkCmdPushConstants;
  PFN_vkCmdSetPatchControlPointsEXT _vkCmdSetPatchControlPointsEXT;
  PFN_vkCmdSetPrimitiveRestartEnableEXT _vkCmdSetPrimitiveRestartEnableEXT;
  PFN_vkCmdSetPrimitiveTopologyEXT _vkCmdSetPrimitiveTopologyEXT;
  PFN_vkUpdateDescriptorSets _vkUpdateDescriptorSets;

  friend class VulkanGraphicsBuffer;
  friend class VulkanGraphicsWindow;
  friend class VulkanShaderContext;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsStateGuardian::init_type();
    register_type(_type_handle, "VulkanGraphicsStateGuardian",
                  GraphicsStateGuardian::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class wglGraphicsBuffer;
};

#include "vulkanGraphicsStateGuardian.I"

#endif
