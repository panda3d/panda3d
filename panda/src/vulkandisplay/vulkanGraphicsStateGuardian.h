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

class VulkanShaderContext;

/**
 * Manages a Vulkan device, and manages sending render commands to this
 * device.
 */
class VulkanGraphicsStateGuardian FINAL : public GraphicsStateGuardian {
public:
  VulkanGraphicsStateGuardian(GraphicsEngine *engine, VulkanGraphicsPipe *pipe,
                              VulkanGraphicsStateGuardian *share_with,
                              uint32_t queue_family_index);
  virtual ~VulkanGraphicsStateGuardian();

  virtual string get_driver_renderer();

  virtual TextureContext *prepare_texture(Texture *tex, int view);
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
  virtual void release_vertex_buffer(VertexBufferContext *vbc);

  virtual IndexBufferContext *prepare_index_buffer(GeomPrimitive *data);
  virtual void release_index_buffer(IndexBufferContext *ibc);

  virtual void dispatch_compute(int size_x, int size_y, int size_z);

  virtual PT(GeomMunger) make_geom_munger(const RenderState *state,
                                          Thread *current_thread);

  virtual void clear(DrawableRegion *clearable);
  virtual void prepare_display_region(DisplayRegionPipelineReader *dr);
  virtual bool prepare_lens();

  virtual bool begin_frame(Thread *current_thread);
  virtual bool begin_scene();
  virtual void end_scene();
  virtual void end_frame(Thread *current_thread);

  virtual bool begin_draw_primitives(const GeomPipelineReader *geom_reader,
                                     const GeomMunger *munger,
                                     const GeomVertexDataPipelineReader *data_reader,
                                     bool force);
  virtual bool draw_triangles(const GeomPrimitivePipelineReader *reader,
                              bool force);
  virtual bool draw_tristrips(const GeomPrimitivePipelineReader *reader,
                              bool force);
  virtual bool draw_trifans(const GeomPrimitivePipelineReader *reader,
                            bool force);
  virtual bool draw_patches(const GeomPrimitivePipelineReader *reader,
                            bool force);
  virtual bool draw_lines(const GeomPrimitivePipelineReader *reader,
                          bool force);
  virtual bool draw_linestrips(const GeomPrimitivePipelineReader *reader,
                               bool force);
  virtual bool draw_points(const GeomPrimitivePipelineReader *reader,
                           bool force);
  virtual void end_draw_primitives();

  virtual void reset();

private:
  VkDevice _device;
  VkQueue _queue;
  VkCommandPool _cmd_pool;
  VkCommandBuffer _cmd;
  pvector<VkRect2D> _viewports;
  VkRenderPass _render_pass;
  VkPipelineCache _pipeline_cache;
  VkPipelineLayout _pipeline_layout;
  VkPipeline _pipeline;
  VulkanShaderContext *_default_sc;

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
