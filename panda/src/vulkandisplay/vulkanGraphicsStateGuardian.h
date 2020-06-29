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
#include "vulkanMemoryPage.h"

class VulkanIndexBufferContext;
class VulkanShaderContext;
class VulkanTextureContext;
class VulkanVertexBufferContext;

/**
 * Manages a Vulkan device, and manages sending render commands to this
 * device.
 */
class VulkanGraphicsStateGuardian final : public GraphicsStateGuardian {
public:
  VulkanGraphicsStateGuardian(GraphicsEngine *engine, VulkanGraphicsPipe *pipe,
                              VulkanGraphicsStateGuardian *share_with,
                              uint32_t queue_family_index);
  virtual ~VulkanGraphicsStateGuardian();

  virtual void close_gsg();

  virtual std::string get_driver_vendor();
  virtual std::string get_driver_renderer();
  virtual std::string get_driver_version();

  bool allocate_memory(VulkanMemoryBlock &block, const VkMemoryRequirements &reqs,
                       VkFlags required_flags, bool linear);

  virtual TextureContext *prepare_texture(Texture *tex, int view);
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

  virtual bool begin_draw_primitives(const GeomPipelineReader *geom_reader,
                                     const GeomVertexDataPipelineReader *data_reader,
                                     bool force);
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

  virtual void reset();

  virtual bool framebuffer_copy_to_texture(Texture *tex, int view, int z,
                                           const DisplayRegion *dr,
                                           const RenderBuffer &rb);
  virtual bool framebuffer_copy_to_ram(Texture *tex, int view, int z,
                                       const DisplayRegion *dr,
                                       const RenderBuffer &rb);

private:
  bool do_extract_image(VulkanTextureContext *tc, Texture *tex, int view, int z=-1);

  bool do_draw_primitive(const GeomPrimitivePipelineReader *reader, bool force,
                         VkPrimitiveTopology topology);

public:
  bool create_buffer(VkDeviceSize size, VkBuffer &buffer, VulkanMemoryBlock &block,
                     int usage_flags, VkMemoryPropertyFlagBits flags);
  VulkanTextureContext *create_image(VkImageType type, VkFormat format,
                                     const VkExtent3D &extent, uint32_t levels,
                                     uint32_t layers, VkSampleCountFlagBits samples,
                                     VkImageUsageFlags usage);

  VkSemaphore create_semaphore();

  VkPipeline make_pipeline(VulkanShaderContext *sc, const RenderState *state,
                           const GeomVertexFormat *format,
                           VkPrimitiveTopology topology);
  VkPipeline make_compute_pipeline(VulkanShaderContext *sc);

  // Built-in descriptor set indices, ordered by frequency.  Static descriptor
  // sets come first.  We separate descriptor sets by the RenderAttrib that
  // contains those inputs, so we can swap out only the set that has changed.
  enum DescriptorSetIndex {
    DS_texture_attrib = 0,
    DS_ATTRIB_COUNT = 1,
    DS_dynamic_uniforms = 1,
    DS_SET_COUNT = 2,
  };

  bool get_attrib_descriptor_set(VkDescriptorSet &out, VkDescriptorSetLayout layout,
                                 const RenderAttrib *attrib);

  VkDeviceSize update_dynamic_uniform_buffer(void *data, VkDeviceSize size);
  uint32_t get_color_palette_offset(const LColor &color);

  VkFormat get_image_format(const Texture *texture) const;
  static bool lookup_image_format(VkFormat vk_format, Texture::Format &format,
                                  Texture::ComponentType &type);

public:
  VkDevice _device;
  VkCommandBuffer _cmd;
  VkCommandBuffer _transfer_cmd;
  uint32_t _graphics_queue_family_index;
  PT(Texture) _white_texture;

private:
  uint64_t _frame_counter = 0;
  VkQueue _queue;
  VkQueue _dma_queue;
  VkFence _fence;
  VkCommandPool _cmd_pool;
  pvector<VkRect2D> _viewports;
  VkPipelineCache _pipeline_cache;
  VkDescriptorPool _descriptor_pool;
  VulkanShaderContext *_default_sc;
  VulkanShaderContext *_current_shader;
  const ShaderType::Struct *_push_constant_block_type = nullptr;
  CPT(GeomVertexFormat) _format;

  // Single large uniform buffer used for everything in a frame.
  VkBuffer _uniform_buffer;
  VulkanMemoryBlock _uniform_buffer_memory;
  VkDeviceSize _uniform_buffer_size = 0;
  VkDeviceSize _uniform_buffer_offset = 0;
  VkDeviceSize _uniform_buffer_offset_alignment;
  VkDescriptorSet _uniform_descriptor_set;

  // Stores current framebuffer info.
  VkRenderPass _render_pass;
  VulkanTextureContext *_fb_color_tc;
  VulkanTextureContext *_fb_depth_tc;
  VkSampleCountFlagBits _fb_ms_count = VK_SAMPLE_COUNT_1_BIT;
  VkSemaphore _wait_semaphore;
  VkSemaphore _signal_semaphore;

  // Remembers semaphores created on this device.
  pvector<VkSemaphore> _semaphores;

  // Palette for flat colors.
  VkBuffer _color_vertex_buffer;
  VulkanMemoryBlock _color_vertex_memory;
  int _next_palette_index;
  typedef pmap<LColorf, uint32_t> ColorPaletteIndices;
  ColorPaletteIndices _color_palette;

  // Keep track of a created descriptor set and the last frame in which it was
  // bound (since we can only update it once per frame).
  struct DescriptorSet {
    VkDescriptorSet _handle = VK_NULL_HANDLE;
    uint64_t _last_update_frame = 0;
    WeakReferenceList *_weak_ref = nullptr;
  };

  // We need only one map rather than one per descriptor set since each
  // different type of RenderAttrib corresponds to a different descriptor set.
  typedef pmap<const RenderAttrib *, DescriptorSet> AttribDescriptorSetMap;
  AttribDescriptorSetMap _attrib_descriptor_set_map;

  // Keep track of all the individual allocations.
  Mutex _allocator_lock;
  pvector<VulkanMemoryPage> _memory_pages;
  VkDeviceSize _total_allocated;

  // Keep track of blocks that should be deleted at the next fence.
  pvector<VulkanMemoryBlock> _pending_free;
  pvector<VkBuffer> _pending_delete_buffers;
  pvector<VkDescriptorSet> _pending_delete_descriptor_sets;

  // Queued buffer-to-RAM transfer.
  struct QueuedDownload {
    VkBuffer _buffer;
    VulkanMemoryBlock _block;
    PT(Texture) _texture;
    int _view;
  };
  typedef pvector<QueuedDownload> DownloadQueue;
  DownloadQueue _download_queue;

  friend class VulkanGraphicsBuffer;
  friend class VulkanGraphicsWindow;

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
