/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlRenderInterface.h
 * @author rdb
 * @date 2011-11-04
 */

#ifndef RML_RENDER_INTERFACE_H
#define RML_RENDER_INTERFACE_H

#include "config_rmlui.h"
#include "cullTraverser.h"
#include "displayRegion.h"
#include "geom.h"
#include "graphicsOutput.h"
#include "graphicsStateGuardian.h"
#include "lmatrix.h"
#include "mutexHolder.h"
#include "pvector.h"
#include "renderState.h"
#include "shader.h"
#include "texture.h"
#include "thread.h"
#include "transformState.h"

#ifndef CPPPARSER
#include <RmlUi/Core/RenderInterface.h>
#endif

/**
 * Implements Rml::RenderInterface, dispatching draw calls into Panda3D's cull
 * traverser with full layer / filter / shader support.
 *
 * Layer lifecycle (PushLayer / CompositeLayers / PopLayer):
 *   PushLayer        — allocates a layer buffer, calls begin_layer() to bind
 *                      its FBO; subsequent RenderGeometry calls go there.
 *   CompositeLayers  — ends the source layer, applies the filter chain via
 *                      ping-pong scratch buffers, then blits the result into
 *                      the destination with a fullscreen quad.
 *   PopLayer         — ends and frees the current layer, rebinding the parent.
 *
 * All filter/shader programs are compiled from embedded GLSL strings via
 * Shader::make(SL_GLSL).  No raw GL calls — the implementation is fully
 * backend-agnostic.
 *
 * Thread safety: render() holds _lock for its entire duration.
 * CompileGeometry, CompileFilter, CompileShader and their Release counterparts
 * must be called from the same thread as render(), or the caller must
 * synchronise externally.
 */
class RmlRenderInterface
#ifndef CPPPARSER
  : public Rml::RenderInterface
#endif
{
public:
  ~RmlRenderInterface();

  // Called once by RmlRegion after the window is created.
  void init(GraphicsOutput *window);

  // Releases the layer-buffer pool and scratch buffers, requesting closure of
  // their offscreen GraphicsOutputs.  Must be called while the GraphicsEngine
  // is still valid (i.e. from ~RmlRegion, not during late static teardown).
  void shutdown();

  // Called once per frame from RmlRegion::do_cull.  trav, gsg, and
  // current_thread must remain valid for the duration of the call.
  void render(Rml::Context *context, CullTraverser *trav,
              GraphicsStateGuardian *gsg, Thread *current_thread);

  // -----------------------------------------------------------------------
  // Required Rml::RenderInterface
  // -----------------------------------------------------------------------
  Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices,
                                              Rml::Span<const int> indices) override;
  void RenderGeometry(Rml::CompiledGeometryHandle geometry,
                      Rml::Vector2f translation,
                      Rml::TextureHandle texture) override;
  void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

  Rml::TextureHandle LoadTexture(Rml::Vector2i &texture_dimensions,
                                 const Rml::String &source) override;
  Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source,
                                     Rml::Vector2i source_dimensions) override;
  void ReleaseTexture(Rml::TextureHandle texture) override;

  void EnableScissorRegion(bool enable) override;
  void SetScissorRegion(Rml::Rectanglei region) override;
  void SetTransform(const Rml::Matrix4f *transform) override;

  // Clip mask (non-rectangular clipping: border-radius overflow, clipping under
  // a CSS transform, inset/clipped box-shadow).  Implemented with the stencil
  // buffer via StencilAttrib.
  void EnableClipMask(bool enable) override;
  void RenderToClipMask(Rml::ClipMaskOperation operation,
                        Rml::CompiledGeometryHandle geometry,
                        Rml::Vector2f translation) override;

  // -----------------------------------------------------------------------
  // Optional Rml::RenderInterface — layers / filters / shaders
  // -----------------------------------------------------------------------
  Rml::LayerHandle PushLayer() override;
  void CompositeLayers(Rml::LayerHandle source, Rml::LayerHandle destination,
                       Rml::BlendMode blend_mode,
                       Rml::Span<const Rml::CompiledFilterHandle> filters) override;
  void PopLayer() override;

  Rml::TextureHandle SaveLayerAsTexture() override;
  Rml::CompiledFilterHandle SaveLayerAsMaskImage() override;

  Rml::CompiledFilterHandle CompileFilter(const Rml::String &name,
                                          const Rml::Dictionary &parameters) override;
  void ReleaseFilter(Rml::CompiledFilterHandle filter) override;

  Rml::CompiledShaderHandle CompileShader(const Rml::String &name,
                                          const Rml::Dictionary &parameters) override;
  void RenderShader(Rml::CompiledShaderHandle shader,
                    Rml::CompiledGeometryHandle geometry,
                    Rml::Vector2f translation,
                    Rml::TextureHandle texture) override;
  void ReleaseShader(Rml::CompiledShaderHandle shader) override;

public:
  // One pre-allocated RGBA offscreen buffer for a single UI layer.
  // Public so the file-scope make_layer_buffer helper can name the type.
  struct LayerBuffer {
    PT(GraphicsOutput) _buf;
    PT(DisplayRegion)  _dr;         // plain DR on _buf, used for clear/prepare
    PT(Texture)        _tex;
    bool               _frame_open = false;
    bool               _in_use     = false;
  };

protected:
  struct CompiledGeometry {
    CPT(Geom) _geom;
  };

  // One entry on the active layer stack.
  struct LayerEntry {
    Rml::LayerHandle _handle;  // 0 = main window; else (LayerBuffer*)+1
    Rml::Rectanglei  _scissor; // scissor rect saved at push time
    // True if this layer's buffer failed to bind (begin_frame failed): its
    // geometry must be suppressed rather than drawn into the parent target.
    bool             _suppressed = false;
  };

  // -----------------------------------------------------------------------
  // Filter / shader data structs
  // -----------------------------------------------------------------------
  enum class FilterType {
    INVALID, PASSTHROUGH, BLUR, DROP_SHADOW, COLOR_MATRIX, MASK_IMAGE
  };

  struct CompiledFilterData {
    FilterType  _type         = FilterType::INVALID;
    float       _blend_factor = 1.0f; // opacity passthrough
    float       _sigma        = 0.0f; // blur / drop-shadow
    LVecBase2f  _offset;              // drop-shadow pixel offset
    LColor      _color;               // drop-shadow tint
    LMatrix4f   _color_matrix;        // color-matrix filters
    PT(Texture) _mask_tex;            // mask-image
  };

  enum class ShaderType { INVALID, GRADIENT, CREATION };
  enum class GradientFunc {
    LINEAR, RADIAL, CONIC,
    REPEATING_LINEAR, REPEATING_RADIAL, REPEATING_CONIC
  };

  static constexpr int MAX_STOPS = 16;
  struct CompiledShaderData {
    ShaderType   _type          = ShaderType::INVALID;
    GradientFunc _gradient_func = GradientFunc::LINEAR;
    LVecBase2f   _p, _v;
    int          _num_stops     = 0;
    float        _stop_positions[MAX_STOPS];
    LVecBase4f   _stop_colors[MAX_STOPS];
    LVecBase2f   _dimensions;    // creation shader
  };

  // -----------------------------------------------------------------------
  // Internal helpers
  // -----------------------------------------------------------------------
  PT(Geom) make_geom(Rml::Span<const Rml::Vertex> vertices,
                     Rml::Span<const int> indices,
                     GeomEnums::UsageHint uh);

  // Submit a geom+state to the active cull handler.
  void render_geom(const Geom *geom, const RenderState *state,
                   Rml::Vector2f translation);

  // Submit a fullscreen NDC quad to the active cull handler.
  void composite_quad(CPT(RenderState) state, PT(Texture) tex);

  // Compose the active scissor rectangle onto state as a ScissorAttrib
  // (relative to the current render target's display region).  Returns state
  // unchanged when no scissor is active.
  CPT(RenderState) apply_scissor(CPT(RenderState) state) const;

  // Compile filter shaders lazily on first use.
  void ensure_shaders();

  // Layer pool management.
  void         grow_pool(size_t target_size); // call only outside Render()
  LayerBuffer *alloc_layer();                 // nullptr if pool exhausted
  void         free_layer(LayerBuffer *lb);
  LayerBuffer *get_layer(Rml::LayerHandle handle); // nullptr if handle==0

  // Composites src_tex into a brand-new independent RGBA texture (via a pool
  // buffer that is restored to a clean, reusable state afterwards) and returns
  // it.  Used by SaveLayerAsTexture / SaveLayerAsMaskImage so the returned
  // texture never aliases a live layer render target.  Returns nullptr on
  // failure.  The returned PT is the sole owner; the caller decides refcounting.
  PT(Texture) snapshot_texture(Texture *src_tex);

  // Bind a layer buffer as the current render target.  clear=true starts it as
  // transparent black; clear=false preserves its existing content (compositing).
  // Implemented by nesting begin_frame() on the buffer (GL allows this within an
  // open window frame).
  void bind_target(LayerBuffer *lb, bool clear);

  // Bind a layer buffer's FBO as the current render target, clearing to
  // transparent black.  Asserts lb != nullptr.  (= bind_target(lb, true).)
  void begin_layer(LayerBuffer *lb);

  // End a layer's frame, flushing its texture.  Rebinds dest (nullptr = main
  // window).
  void end_layer(LayerBuffer *lb, LayerBuffer *dest);

  // Apply filters in ping-pong between _scratch[0] and _scratch[1].
  // Returns the texture holding the final result.
  PT(Texture) apply_filters(PT(Texture) src,
                             Rml::Span<const Rml::CompiledFilterHandle> filters);

private:
#ifndef CPPPARSER
  Mutex _lock;

  // The parent window (set by init()).
  GraphicsOutput *_window = nullptr;

  // Layer buffer pool.  Pre-allocated in init(); grown only between frames
  // (in render()) when a prior frame ran out, never mid-frame.
  pvector<LayerBuffer *> _layer_pool;

  // Concurrent-use accounting, used to size pool growth between frames.
  int  _layers_in_use      = 0;
  int  _peak_layers_in_use = 0;
  bool _pool_exhausted     = false;

  // Two scratch buffers for ping-pong compositing within apply_filters().
  LayerBuffer *_scratch[2] = { nullptr, nullptr };

  // Current layer stack.  Bottom entry is the base layer (handle=0).
  pvector<LayerEntry> _layer_stack;

  // Scissor state.
  bool            _scissor_active = false;
  Rml::Rectanglei _scissor_rect;

  // Clip-mask (stencil) state.  When _clip_mask_active, geometry is drawn with
  // a stencil test that passes only where stencil == _clip_mask_ref.  Intersect
  // raises the reference value so successive masks compound.
  bool         _clip_mask_active = false;
  unsigned int _clip_mask_ref    = 1;
  bool         _warned_no_stencil = false;  // one-shot base-layer stencil warning

  // CSS transform: set by SetTransform(); nullptr = identity.
  CPT(TransformState) _css_transform;

  // Per-frame state, filled by render() and cleared after Context::Render().
  CullTraverser          *_trav         = nullptr;
  GraphicsStateGuardian  *_gsg          = nullptr;
  Thread                 *_thread       = nullptr;
  // The RmlRegion's own DisplayRegion, re-prepared when a layer ends back on
  // the main window so the viewport is restored for non-fullscreen regions.
  DisplayRegion          *_base_dr      = nullptr;
  CPT(TransformState)     _net_transform;
  CPT(RenderState)        _net_state;
  Rml::Vector2i           _dimensions;

  // Cached fullscreen quad (built once on first composite_quad call).
  CPT(Geom) _fsq;

  // Shader programs (compiled once on first use).
  bool _shaders_ready = false;

  CPT(RenderState) _shader_passthrough;
  CPT(RenderState) _shader_color_matrix;
  CPT(RenderState) _shader_blend_mask;
  CPT(RenderState) _shader_blur;
  CPT(RenderState) _shader_drop_shadow;
  CPT(RenderState) _shader_gradient;
  CPT(RenderState) _shader_creation;
#endif
};

#endif
