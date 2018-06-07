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

  bool create_buffer(VkDeviceSize size, VkBuffer &buffer, VkDeviceMemory &memory,
                     int usage_flags, VkMemoryPropertyFlagBits flags);

  /**
   * Stores whatever is used to key a cached pipeline into the pipeline map.
   * This allows us to map Panda states to Vulkan pipelines effectively.
   */
  struct PipelineKey {
    INLINE PipelineKey() = default;
    INLINE PipelineKey(const PipelineKey &copy);
    INLINE PipelineKey(PipelineKey &&from) noexcept;

    INLINE void operator = (const PipelineKey &copy);
    INLINE void operator = (PipelineKey &&from) noexcept;

    INLINE bool operator ==(const PipelineKey &other) const;
    INLINE bool operator < (const PipelineKey &other) const;

    CPT(RenderState) _state;
    CPT(GeomVertexFormat) _format;
    VkPrimitiveTopology _topology;
  };

  VkPipeline get_pipeline(const RenderState *state,
                          const GeomVertexFormat *format,
                          VkPrimitiveTopology topology);
  VkPipeline make_pipeline(const RenderState *state,
                           const GeomVertexFormat *format,
                           VkPrimitiveTopology topology);

  /**
   * Stores whatever is used to key a cached descriptor set into the
   * descriptor set map.
   */
  struct DescriptorSetKey {
    INLINE DescriptorSetKey() = default;
    INLINE DescriptorSetKey(const DescriptorSetKey &copy);
    INLINE DescriptorSetKey(DescriptorSetKey &&from) noexcept;

    INLINE void operator = (const DescriptorSetKey &copy);
    INLINE void operator = (DescriptorSetKey &&from) noexcept;

    INLINE bool operator ==(const DescriptorSetKey &other) const;
    INLINE bool operator < (const DescriptorSetKey &other) const;

    CPT(TextureAttrib) _tex_attrib;
    CPT(ShaderAttrib) _shader_attrib;
  };

  VkDescriptorSet get_descriptor_set(const RenderState *state);
  VkDescriptorSet make_descriptor_set(const RenderState *state);

  VkFormat get_image_format(const Texture *texture) const;

private:
  VkDevice _device;
  VkQueue _queue;
  VkQueue _dma_queue;
  uint32_t _graphics_queue_family_index;
  VkFence _fence;
  VkCommandPool _cmd_pool;
  VkCommandBuffer _cmd;
  VkCommandBuffer _transfer_cmd;
  pvector<VkRect2D> _viewports;
  VkPipelineCache _pipeline_cache;
  VkPipelineLayout _pipeline_layout;
  VkDescriptorSetLayout _descriptor_set_layout;
  VkDescriptorPool _descriptor_pool;
  VulkanShaderContext *_default_sc;
  CPT(GeomVertexFormat) _format;
  PT(Texture) _white_texture;

  // Stores current framebuffer info.
  VkRenderPass _render_pass;
  VulkanTextureContext *_fb_color_tc;
  VulkanTextureContext *_fb_depth_tc;
  VkSemaphore _wait_semaphore;
  VkSemaphore _signal_semaphore;

  // Palette for flat colors.
  VkBuffer _color_vertex_buffer;
  int _next_palette_index;
  typedef pmap<LColorf, uint32_t> ColorPaletteIndices;
  ColorPaletteIndices _color_palette;

  typedef pmap<PipelineKey, VkPipeline> PipelineMap;
  PipelineMap _pipeline_map;

  typedef pmap<DescriptorSetKey, VkDescriptorSet> DescriptorSetMap;
  DescriptorSetMap _descriptor_set_map;

  // Queued buffer-to-RAM transfer.
  struct QueuedDownload {
    VkBuffer _buffer;
    VkDeviceMemory _memory;
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
