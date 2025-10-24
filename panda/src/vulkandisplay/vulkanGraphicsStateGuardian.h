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
#include "vulkanCommandBuffer.h"
#include "vulkanFrameData.h"
#include "vulkanMemoryPage.h"
#include "vulkanShaderContext.h"
#include "circularAllocator.h"
#include "completionToken.h"

class VulkanBufferContext;
class VulkanGraphicsPipe;
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
                                    VkPipelineStageFlags2 stage_mask,
                                    VkAccessFlags2 access_mask,
                                    bool discard=false);
  virtual TextureContext *prepare_texture(Texture *tex);
  bool create_texture(VulkanTextureContext *vtc);
  bool upload_texture(VulkanTextureContext *vtc,
                      CompletionToken token = CompletionToken());
  virtual bool update_texture(TextureContext *tc, bool force,
                              CompletionToken token = CompletionToken());
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

  VulkanBufferContext *use_shader_buffer(ShaderBuffer *buffer,
                                         VkPipelineStageFlags2 stage_mask,
                                         VkAccessFlags2 access_mask);
  virtual BufferContext *prepare_shader_buffer(ShaderBuffer *data);
  virtual void release_shader_buffer(BufferContext *bc);
  virtual bool extract_shader_buffer_data(ShaderBuffer *buffer, vector_uchar &data);

  virtual void issue_timer_query(int pstats_index);
  uint32_t get_next_timer_query(int pstats_index);
  void replace_timer_query_pool();

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
  bool begin_frame(Thread *current_thread, VkSemaphore wait_for);
  void end_frame(Thread *current_thread, VkSemaphore signal_done);
  void finish_frame(FrameData &frame_data);
  bool finish_one_frame();
  FrameData &get_next_frame_data(bool finish_frames = false);
  INLINE FrameData &get_frame_data();

  VkCommandBuffer create_command_buffer();
  VulkanCommandBuffer begin_command_buffer(VkSemaphore wait_for = VK_NULL_HANDLE);
  void end_command_buffer(VulkanCommandBuffer &&cmd,
                          VkSemaphore signal_done = VK_NULL_HANDLE);
  uint64_t flush();

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

  virtual GraphicsOutput *make_shadow_buffer(LightLensNode *light, Texture *tex,
                                             GraphicsOutput *host);

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
  bool do_extract_buffer(VulkanBufferContext *tc, vector_uchar &data);

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
  bool wait_semaphore(VkSemaphore semaphore, uint64_t value,
                      uint64_t timeout = UINT64_MAX);

  struct FbConfig;
  uint32_t choose_fb_config(FbConfig &out, FrameBufferProperties &props,
                            VkFormat preferred_format = VK_FORMAT_UNDEFINED);

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

  struct FbConfig {
    small_vector<VkFormat, 1> _color_formats;
    VkFormat _depth_format = VK_FORMAT_UNDEFINED;
    VkFormat _stencil_format = VK_FORMAT_UNDEFINED;
    VkSampleCountFlagBits _sample_count = VK_SAMPLE_COUNT_1_BIT;

    bool operator == (const FbConfig &other) const {
      if (_color_formats.size() != other._color_formats.size() ||
          _depth_format != other._depth_format ||
          _stencil_format != other._stencil_format ||
          _sample_count != other._sample_count) {
        return false;
      }
      for (size_t i = 0; i < _color_formats.size(); ++i) {
        if (_color_formats[i] != other._color_formats[i]) {
          return false;
        }
      }
      return true;
    }
  };

private:
  VkQueue _queue = VK_NULL_HANDLE;
  VkQueue _dma_queue = VK_NULL_HANDLE;
  VkCommandPool _cmd_pool = VK_NULL_HANDLE;
  pvector<VkRect2D> _viewports;
  VkPipelineCache _pipeline_cache = VK_NULL_HANDLE;
  VkDescriptorPool _descriptor_pool = VK_NULL_HANDLE;
  VulkanShaderContext *_default_sc = nullptr;
  Shader *_current_shader = nullptr;
  VulkanShaderContext *_current_sc = nullptr;
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
  uint32_t _fb_config = 0;
  pvector<FbConfig> _fb_configs;

  // Static "null" vertex buffer if nullDescriptor is not supported.
  VkBuffer _null_vertex_buffer = VK_NULL_HANDLE;
  VulkanMemoryBlock _null_vertex_memory;
  bool _needs_write_null_vertex_data = false;

  // Descriptor set layouts used for the LightAttrib descriptor sets.
  // The others are shader-dependent and stored in VulkanShaderContext.
  VkSampler _shadow_sampler;
  VkDescriptorSetLayout _lattr_descriptor_set_layout;
  VkDescriptorSet _empty_lattr_descriptor_set;

  // Keep track of all the individual allocations.
  Mutex _allocator_lock;
  pdeque<VulkanMemoryPage> _memory_pages;
  VkDeviceSize _total_allocated = 0u;

  // We store references to two command buffers.  The transfer cmd is used for
  // anything that needs to happen outside a render pass (including transfers),
  // the render cmd is used for anything inside.  The render cmd is only
  // present between begin_frame() and end_frame() and MUST have a higher seq.
  VulkanCommandBuffer _transfer_cmd;
  VulkanCommandBuffer _render_cmd;

  uint64_t _next_begin_command_buffer_seq = 0;
  uint64_t _next_end_command_buffer_seq = 0;
  pvector<VkCommandBuffer> _free_command_buffers; // new and unused
  pvector<VkCommandBuffer> _pending_command_buffers; // ready to submit
  pvector<VkBufferMemoryBarrier2> _pending_buffer_barriers;
  pvector<VkImageMemoryBarrier2> _pending_image_barriers;
  uint32_t _first_pending_command_buffer_seq = 0;
  uint32_t _last_pending_command_buffer_seq = 0;
  struct PendingSubmission {
    VkSemaphore _wait_semaphore;
    VkSemaphore _signal_semaphore;
    uint32_t _first_command_buffer; // Indexes into _pending_command_buffers
    uint32_t _num_command_buffers;
  };
  pvector<PendingSubmission> _pending_submissions;
  uint64_t _last_submitted_watermark = 0;

  static const size_t _frame_data_capacity = 5;
  FrameData _frame_data_pool[_frame_data_capacity];
  size_t _frame_data_head = _frame_data_capacity;
  size_t _frame_data_tail = 0;
  FrameData *_frame_data = nullptr;
  FrameData *_last_frame_data = nullptr;
  VkSemaphore _timeline_semaphore = VK_NULL_HANDLE;
  uint32_t _transfer_end_query = 0;
  VkQueryPool _transfer_end_query_pool = VK_NULL_HANDLE;

  uint64_t _frame_counter = 0;
  uint64_t _last_finished_frame = 0;
  int _current_clock_frame_number = -1;

  VkQueryPool _timer_query_pool = VK_NULL_HANDLE;
  uint32_t _timer_query_pool_size = 7; // always power of 2 minus one
  uint32_t _timer_query_head = 0;
  uint32_t _timer_query_tail = 0;
  PStatFrameData _pstats_frame_data;
  int _pstats_frame_number = 0;
  double _pstats_frame_end_time = 0.0;
  uint64_t _gpu_sync_time = 0;
  double _cpu_sync_time = 0;
  double _timer_query_factor = 0.0;

  // Feature checks.
  bool _supports_dynamic_rendering = false;
  bool _supports_custom_border_colors = false;
  bool _supports_vertex_attrib_divisor = false;
  bool _supports_vertex_attrib_zero_divisor = false;
  bool _supports_extended_dynamic_state2 = false;
  bool _supports_extended_dynamic_state2_patch_control_points = false;

  // Limits.
  uint32_t _max_inline_uniform_block_size = 0;

  // Function pointers.
  PFN_vkCmdBeginRendering _vkCmdBeginRendering;
  PFN_vkCmdBindIndexBuffer _vkCmdBindIndexBuffer;
  PFN_vkCmdBindPipeline _vkCmdBindPipeline;
  PFN_vkCmdBindVertexBuffers _vkCmdBindVertexBuffers;
  PFN_vkCmdDraw _vkCmdDraw;
  PFN_vkCmdDrawIndexed _vkCmdDrawIndexed;
  PFN_vkCmdEndRendering _vkCmdEndRendering;
  PFN_vkCmdPushConstants _vkCmdPushConstants;
  PFN_vkCmdSetPatchControlPointsEXT _vkCmdSetPatchControlPointsEXT;
  PFN_vkCmdSetPrimitiveRestartEnable _vkCmdSetPrimitiveRestartEnable;
  PFN_vkCmdSetPrimitiveTopology _vkCmdSetPrimitiveTopology;
  PFN_vkCmdWriteTimestamp2 _vkCmdWriteTimestamp2;
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
