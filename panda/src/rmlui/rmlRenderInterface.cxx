/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlRenderInterface.cxx
 * @author rdb
 * @date 2011-11-04
 */

#include "rmlRenderInterface.h"
#include "colorAttrib.h"
#include "colorBlendAttrib.h"
#include "cullBinAttrib.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "displayRegion.h"
#include "frameBufferProperties.h"
#include "geomTriangles.h"
#include "geomVertexArrayData.h"
#include "geomVertexData.h"
#include "geomVertexWriter.h"
#include "internalName.h"
#include "pta_LMatrix4.h"
#include "pta_LVecBase4.h"
#include "pta_float.h"
#include "colorWriteAttrib.h"
#include "scissorAttrib.h"
#include "shader.h"
#include "shaderAttrib.h"
#include "stencilAttrib.h"
#include "texture.h"
#include "textureAttrib.h"
#include "texturePool.h"
#include "textureStage.h"

#ifndef CPPPARSER
#include <RmlUi/Core/DecorationTypes.h>
#include <RmlUi/Core/Dictionary.h>
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/Core.h>
#endif

// ===========================================================================
// Embedded GLSL shader sources
//
// All shaders target GLSL 1.50 (OpenGL 3.2 core).  No layout(binding=N) on
// samplers — macOS OpenGL 4.1 doesn't support ARB_shading_language_420pack.
// All samplers use custom names (u_source_tex, u_mask_tex, …) bound via
// set_shader_input.  p3d_-prefixed uniform names are reserved for Panda's
// state-driven binding (a sampler named p3d_Texture0 is fed from the state's
// TextureAttrib and ignores shader inputs entirely), so they must not be used
// for inputs supplied with set_shader_input.
// ===========================================================================

// ---------------------------------------------------------------------------
// Passthrough — blit one texture to the current FBO, optionally scaled
// ---------------------------------------------------------------------------
static const char *s_vert_passthrough = R"GLSL(
#version 150
in vec3 p3d_Vertex;
in vec2 p3d_MultiTexCoord0;
uniform vec2 u_uv_offset;  // default (0,0)
uniform vec2 u_uv_scale;   // default (1,1)
out vec2 v_uv;
void main() {
    // Sample a sub-rectangle of the source: uv = offset + texcoord * scale.
    // With offset=0, scale=1 this is the identity (full-texture blit).
    v_uv = u_uv_offset + p3d_MultiTexCoord0 * u_uv_scale;
    gl_Position = vec4(p3d_Vertex.xy, 0.0, 1.0);
}
)GLSL";

static const char *s_frag_passthrough = R"GLSL(
#version 150
uniform sampler2D u_source_tex;
uniform float u_blend_factor;
in  vec2 v_uv;
out vec4 o_color;
void main() {
    o_color = texture(u_source_tex, v_uv) * u_blend_factor;
}
)GLSL";

// ---------------------------------------------------------------------------
// Color matrix — brightness / contrast / grayscale / sepia / hue-rotate / etc.
// ---------------------------------------------------------------------------
static const char *s_frag_color_matrix = R"GLSL(
#version 150
uniform sampler2D u_source_tex;
uniform mat4 u_color_matrix;
in  vec2 v_uv;
out vec4 o_color;
void main() {
    vec4 c = texture(u_source_tex, v_uv);
    // The matrix is authored in mathematical (column-vector) row order and
    // uploaded untransposed from Panda's row-major storage, so GLSL sees its
    // transpose; c * M therefore computes the intended M * c.  The constant
    // term (fourth math column) is multiplied by c.a, which is correct in
    // premultiplied-alpha space.
    o_color = vec4(vec3(c * u_color_matrix), c.a);
}
)GLSL";

// ---------------------------------------------------------------------------
// Blend mask — multiply source rgb by mask alpha  (CSS mask-image)
// ---------------------------------------------------------------------------
static const char *s_frag_blend_mask = R"GLSL(
#version 150
uniform sampler2D u_source_tex;
uniform sampler2D u_mask_tex;
in  vec2 v_uv;
out vec4 o_color;
void main() {
    vec4 c  = texture(u_source_tex, v_uv);
    float m = texture(u_mask_tex, v_uv).a;
    o_color = c * m;
}
)GLSL";

// ---------------------------------------------------------------------------
// Drop shadow — offset tinted shadow only; caller composites original on top.
// ---------------------------------------------------------------------------
static const char *s_frag_drop_shadow = R"GLSL(
#version 150
uniform sampler2D u_source_tex;
uniform vec4  u_shadow_color;
uniform vec2  u_offset;
uniform vec2  u_inv_size;
in  vec2 v_uv;
out vec4 o_color;
void main() {
    vec2 suv = v_uv + u_offset * u_inv_size;
    float a  = texture(u_source_tex, suv).a;
    o_color  = u_shadow_color * a;
}
)GLSL";

// ---------------------------------------------------------------------------
// Separable Gaussian blur (single axis per pass, 7 taps)
//
// CPU weight array: index 0 = centre tap, index 3 = outermost tap.
// GLSL weight index: wi = abs(i - 3), so i==3 → wi=0 (centre).
// The two conventions are consistent: both treat index 0 as the
// highest-weight (centre) sample.
// ---------------------------------------------------------------------------
#define BLUR_TAPS    7
#define BLUR_WEIGHTS 4   // ceil(BLUR_TAPS / 2)

static const char *s_vert_blur = R"GLSL(
#version 150
in vec3 p3d_Vertex;
in vec2 p3d_MultiTexCoord0;
uniform vec2  u_axis;
uniform vec2  u_inv_size;
out vec2 v_uv[7];
void main() {
    for (int i = 0; i < 7; i++) {
        float d = float(i - 3);
        v_uv[i] = p3d_MultiTexCoord0 + u_axis * d * u_inv_size;
    }
    gl_Position = vec4(p3d_Vertex.xy, 0.0, 1.0);
}
)GLSL";

static const char *s_frag_blur = R"GLSL(
#version 150
uniform sampler2D u_source_tex;
uniform float u_weights[4];
uniform vec2  u_uv_min;
uniform vec2  u_uv_max;
in  vec2 v_uv[7];
out vec4 o_color;
void main() {
    o_color = vec4(0.0);
    for (int i = 0; i < 7; i++) {
        vec2 in_r = step(u_uv_min, v_uv[i]) * step(v_uv[i], u_uv_max);
        int  wi   = (i < 3) ? (3 - i) : (i - 3);
        o_color  += texture(u_source_tex, v_uv[i])
                    * in_r.x * in_r.y * u_weights[wi];
    }
}
)GLSL";

// ---------------------------------------------------------------------------
// Gradient decorator (linear / radial / conic, repeating variants, 16 stops)
// ---------------------------------------------------------------------------
static const char *s_vert_ui = R"GLSL(
#version 150
in vec2 p3d_Vertex;
in vec4 p3d_Color;
in vec2 p3d_MultiTexCoord0;
uniform mat4 p3d_ModelViewProjectionMatrix;
out vec2 v_uv;
out vec4 v_color;
void main() {
    v_uv    = p3d_MultiTexCoord0;
    v_color = p3d_Color;
    gl_Position = p3d_ModelViewProjectionMatrix * vec4(p3d_Vertex, 0.0, 1.0);
}
)GLSL";

static const char *s_frag_gradient = R"GLSL(
#version 150
#define LINEAR           0
#define RADIAL           1
#define CONIC            2
#define REPEATING_LINEAR 3
#define REPEATING_RADIAL 4
#define REPEATING_CONIC  5
#define PI 3.14159265
#define MAX_STOPS 16
uniform int   u_func;
uniform vec2  u_p;
uniform vec2  u_v;
uniform vec4  u_stop_colors[MAX_STOPS];
uniform float u_stop_positions[MAX_STOPS];
uniform int   u_num_stops;
in  vec2 v_uv;
in  vec4 v_color;
out vec4 o_color;
vec4 mix_stops(float t) {
    vec4 c = u_stop_colors[0];
    for (int i = 1; i < u_num_stops; i++)
        c = mix(c, u_stop_colors[i],
                smoothstep(u_stop_positions[i-1], u_stop_positions[i], t));
    return c;
}
void main() {
    float t = 0.0;
    if (u_func == LINEAR || u_func == REPEATING_LINEAR) {
        float d2 = dot(u_v, u_v);
        t = dot(u_v, v_uv - u_p) / d2;
    } else if (u_func == RADIAL || u_func == REPEATING_RADIAL) {
        t = length(u_v * (v_uv - u_p));
    } else {
        mat2 R = mat2(u_v.x, -u_v.y, u_v.y, u_v.x);
        vec2 V = R * (v_uv - u_p);
        t = 0.5 + atan(-V.x, V.y) / (2.0 * PI);
    }
    if (u_func == REPEATING_LINEAR || u_func == REPEATING_RADIAL ||
        u_func == REPEATING_CONIC) {
        float t0 = u_stop_positions[0];
        float t1 = u_stop_positions[u_num_stops - 1];
        t = t0 + mod(t - t0, t1 - t0);
    }
    o_color = v_color * mix_stops(t);
}
)GLSL";

// ---------------------------------------------------------------------------
// Creation procedural shader (Danilo Guanabara / shadertoy)
// ---------------------------------------------------------------------------
static const char *s_frag_creation = R"GLSL(
#version 150
uniform float u_time;
uniform vec2  u_dimensions;
in  vec2 v_uv;
in  vec4 v_color;
out vec4 o_color;
void main() {
    float t = u_time;
    vec3 c;
    float l;
    for (int i = 0; i < 3; i++) {
        vec2 p  = v_uv;
        vec2 uv = p;
        p -= 0.5;
        p.x *= u_dimensions.x / u_dimensions.y;
        float z = t + float(i) * 0.07;
        l  = length(p);
        uv += p / l * (sin(z) + 1.0) * abs(sin(l * 9.0 - z - z));
        c[i] = 0.01 / length(mod(uv, vec2(1.0)) - vec2(0.5));
    }
    o_color = vec4(c / l, v_color.a);
}
)GLSL";

// ===========================================================================
// Internal helpers
// ===========================================================================

/**
 * Computes normalised Gaussian weights for a 7-tap half-kernel.
 * out[0] is the centre weight; out[3] is the outermost tap weight.
 */
static void compute_blur_weights(float sigma, float out[BLUR_WEIGHTS]) {
  double sum = 0.0;
  for (int i = 0; i < BLUR_WEIGHTS; ++i) {
    out[i] = (float)std::exp(-0.5 * (double)(i * i) / (sigma * sigma));
    sum += out[i] * (i == 0 ? 1.0 : 2.0);
  }
  for (int i = 0; i < BLUR_WEIGHTS; ++i) {
    out[i] = (float)(out[i] / sum);
  }
}

/**
 * Builds a fullscreen NDC quad (positions -1..1, UVs 0..1).
 */
static CPT(Geom) build_fullscreen_quad() {
  struct V { float x, y, u, v; };
  static const V verts[4] = {
    {-1.f, -1.f, 0.f, 0.f},
    { 1.f, -1.f, 1.f, 0.f},
    { 1.f,  1.f, 1.f, 1.f},
    {-1.f,  1.f, 0.f, 1.f},
  };
  static const int idx[6] = {0, 1, 2, 0, 2, 3};

  PT(GeomVertexData) vd = new GeomVertexData(
    "fsq", GeomVertexFormat::get_v3c4t2(), GeomEnums::UH_static);
  vd->unclean_set_num_rows(4);
  GeomVertexWriter vw(vd, InternalName::get_vertex());
  GeomVertexWriter cw(vd, InternalName::get_color());
  GeomVertexWriter tw(vd, InternalName::get_texcoord());
  for (const auto &v : verts) {
    vw.add_data3f(v.x, v.y, 0.f);
    cw.add_data4i(255, 255, 255, 255);
    tw.add_data2f(v.u, v.v);
  }
  PT(GeomTriangles) tris = new GeomTriangles(GeomEnums::UH_static);
  PT(GeomVertexArrayData) idata = tris->modify_vertices();
  idata->unclean_set_num_rows(6);
  GeomVertexWriter iw(idata, 0);
  for (int i : idx) iw.add_data1i(i);

  PT(Geom) g = new Geom(vd);
  g->add_primitive(tris);
  return g;
}

/**
 * Creates a single RGBA GraphicsBuffer sharing the GSG of parent_window.
 */
static RmlRenderInterface::LayerBuffer *
make_layer_buffer(GraphicsOutput *parent_window, const std::string &name) {
  FrameBufferProperties fbp;
  fbp.set_rgb_color(1);
  fbp.set_rgba_bits(8, 8, 8, 8);
  fbp.set_depth_bits(0);
  // 8 stencil bits for clip-mask (border-radius / transform) clipping.
  fbp.set_stencil_bits(8);

  int w = parent_window->get_x_size();
  int h = parent_window->get_y_size();

  PT(Texture) tex = new Texture(name);
  tex->setup_2d_texture(w, h, Texture::T_unsigned_byte, Texture::F_rgba);
  tex->set_wrap_u(SamplerState::WM_clamp);
  tex->set_wrap_v(SamplerState::WM_clamp);
  tex->set_minfilter(SamplerState::FT_linear);
  tex->set_magfilter(SamplerState::FT_linear);

  PT(GraphicsOutput) buf = parent_window->make_texture_buffer(
    name, w, h, tex, false, &fbp);
  if (buf == nullptr) {
    rmlui_cat.error()
      << "Failed to allocate RmlUi layer buffer '" << name << "'\n";
    return nullptr;
  }
  buf->set_active(false);

  PT(DisplayRegion) dr = buf->make_display_region();
  dr->set_clear_color_active(true);
  dr->set_clear_color(LColor(0, 0, 0, 0));

  auto *lb = new RmlRenderInterface::LayerBuffer;
  lb->_buf       = buf;
  lb->_dr        = dr;
  lb->_tex       = tex;
  lb->_in_use    = false;
  lb->_frame_open = false;
  return lb;
}

/**
 * Closes a LayerBuffer's offscreen GraphicsOutput and frees the struct.  The
 * GraphicsEngine retains a reference to every buffer it creates, so the FBO
 * outlives the LayerBuffer struct unless explicitly closed here.
 */
static void
destroy_layer_buffer(RmlRenderInterface::LayerBuffer *lb) {
  if (lb == nullptr) return;
  if (lb->_buf != nullptr) {
    lb->_buf->request_close();
  }
  delete lb;
}

// ===========================================================================
// Destructor / shutdown — release the layer pool
// ===========================================================================

/**
 *
 */
RmlRenderInterface::
~RmlRenderInterface() {
  shutdown();
}

/**
 * Releases the layer-buffer pool and scratch buffers.  Idempotent.  Called by
 * the destructor and (preferably) explicitly by RmlRegion before the engine
 * tears down, so the offscreen buffers are closed while the GraphicsEngine is
 * still valid.
 */
void RmlRenderInterface::
shutdown() {
  for (LayerBuffer *lb : _layer_pool) {
    destroy_layer_buffer(lb);
  }
  _layer_pool.clear();

  for (int i = 0; i < 2; ++i) {
    destroy_layer_buffer(_scratch[i]);
    _scratch[i] = nullptr;
  }

  _layer_stack.clear();
  _window = nullptr;
}

// ===========================================================================
// init() — allocate layer pool
// ===========================================================================

/**
 * Called once by RmlRegion after the window is created.  Pre-allocates the
 * layer buffer pool and scratch buffers.
 */
void RmlRenderInterface::
init(GraphicsOutput *window) {
  nassertv(window != nullptr);
  _window = window;

  grow_pool(rmlui_layer_pool_size);

  for (int i = 0; i < 2; ++i) {
    _scratch[i] = make_layer_buffer(window,
      std::string("rmlui-scratch-") + std::to_string(i));
    // Mark permanently in-use so scratch buffers are never returned to the pool.
    if (_scratch[i]) _scratch[i]->_in_use = true;
  }
}

/**
 * Grows the layer-buffer pool to at least target_size entries.  MUST be called
 * outside of context->Render() (i.e. while no layer FBO frame is open), because
 * GraphicsOutput::make_texture_buffer re-enters GraphicsEngine::open_windows;
 * doing that mid-frame corrupts GL state and produces a white frame.
 */
void RmlRenderInterface::
grow_pool(size_t target_size) {
  while (_layer_pool.size() < target_size) {
    LayerBuffer *lb = make_layer_buffer(_window,
      std::string("rmlui-layer-") + std::to_string(_layer_pool.size()));
    if (lb == nullptr) break;
    _layer_pool.push_back(lb);
  }
}

// ===========================================================================
// Lazy shader compilation
// ===========================================================================

/**
 * Compiles all embedded GLSL filter/shader programs on first use.
 */
void RmlRenderInterface::
ensure_shaders() {
  if (_shaders_ready) return;
  _shaders_ready = true;

  CPT(RenderState) base = RenderState::make(
    CullBinAttrib::make("unsorted", 0),
    DepthTestAttrib::make(RenderAttrib::M_none),
    DepthWriteAttrib::make(DepthWriteAttrib::M_off),
    ColorAttrib::make_vertex()
  );

  // Premultiplied-alpha "over": layer buffers hold premultiplied content (drawn
  // with the O_one net_state blend), so composite them with src factor O_one.
  auto blend_over = [&]() {
    return base->add_attrib(ColorBlendAttrib::make(
      ColorBlendAttrib::M_add,
      ColorBlendAttrib::O_one,
      ColorBlendAttrib::O_one_minus_incoming_alpha));
  };
  auto blend_replace = [&]() {
    return base->add_attrib(ColorBlendAttrib::make(
      ColorBlendAttrib::M_add,
      ColorBlendAttrib::O_one,
      ColorBlendAttrib::O_zero));
  };

  auto make_state = [&](const char *vert, const char *frag,
                        CPT(RenderState) blend) -> CPT(RenderState) {
    PT(Shader) sh = Shader::make(Shader::SL_GLSL, vert, frag);
    CPT(RenderState) st = blend->add_attrib(ShaderAttrib::make(sh));
    // Identity UV remap default for the passthrough vertex shader (sampling the
    // full source); snapshot_texture overrides these to crop a sub-rectangle.
    if (vert == s_vert_passthrough) {
      CPT(RenderAttrib) sa = st->get_attrib(ShaderAttrib::get_class_slot());
      sa = DCAST(ShaderAttrib, sa)->set_shader_input(
        InternalName::make("u_uv_offset"), LVecBase2f(0.f, 0.f));
      sa = DCAST(ShaderAttrib, sa)->set_shader_input(
        InternalName::make("u_uv_scale"), LVecBase2f(1.f, 1.f));
      st = st->add_attrib(sa);
    }
    return st;
  };

  _shader_passthrough  = make_state(s_vert_passthrough, s_frag_passthrough,  blend_over());
  _shader_color_matrix = make_state(s_vert_passthrough, s_frag_color_matrix, blend_replace());
  _shader_blend_mask   = make_state(s_vert_passthrough, s_frag_blend_mask,   blend_replace());
  _shader_drop_shadow  = make_state(s_vert_passthrough, s_frag_drop_shadow,  blend_replace());
  _shader_blur         = make_state(s_vert_blur,        s_frag_blur,         blend_replace());
  _shader_gradient     = make_state(s_vert_ui,          s_frag_gradient,     blend_over());
  // _shader_creation compiled lazily in RenderShader on first use.
}

// ===========================================================================
// Layer pool management
// ===========================================================================

/**
 * Returns an unused LayerBuffer from the pool, or nullptr if the pool is
 * exhausted.
 *
 * The pool is NEVER grown here: that would allocate a GraphicsBuffer mid-frame
 * (re-entering GraphicsEngine::open_windows while the main window's frame is
 * open), which corrupts GL state and produces a white frame.  Instead, an
 * exhausted allocation is recorded and the pool is grown before the NEXT frame
 * in render().  Callers (PushLayer / apply_filters) handle nullptr gracefully.
 */
RmlRenderInterface::LayerBuffer *RmlRenderInterface::
alloc_layer() {
  for (LayerBuffer *lb : _layer_pool) {
    if (!lb->_in_use && lb->_tex != nullptr) {
      lb->_in_use = true;
      ++_layers_in_use;
      _peak_layers_in_use = std::max(_peak_layers_in_use, _layers_in_use);
      return lb;
    }
  }
  // Pool exhausted this frame.  Remember the shortfall so render() can grow the
  // pool before the next frame, and degrade gracefully (one missing effect)
  // rather than allocating a buffer mid-frame.
  _peak_layers_in_use = std::max(_peak_layers_in_use, _layers_in_use + 1);
  if (!_pool_exhausted) {
    _pool_exhausted = true;
    rmlui_cat.warning()
      << "RmlUi layer pool exhausted (" << _layer_pool.size()
      << " buffers); growing before next frame.  Increase rmlui-layer-pool-size "
      << "to avoid a dropped effect on this frame.\n";
  }
  return nullptr;
}

/**
 * Returns a layer buffer to the pool.
 */
void RmlRenderInterface::
free_layer(LayerBuffer *lb) {
  nassertv(lb != nullptr);
  if (lb->_in_use && _layers_in_use > 0) {
    --_layers_in_use;
  }
  lb->_in_use     = false;
  lb->_frame_open = false;
}

/**
 * Decodes a LayerHandle back to a LayerBuffer pointer.  Returns nullptr for
 * the main-window handle (0).
 */
RmlRenderInterface::LayerBuffer *RmlRenderInterface::
get_layer(Rml::LayerHandle handle) {
  if (handle == 0) return nullptr;
  return reinterpret_cast<LayerBuffer *>(handle - 1);
}

// ===========================================================================
// Frame management: begin / end a layer buffer
// ===========================================================================

/**
 * Binds lb's FBO as the current render target and clears to transparent black.
 *
 * Implemented by nesting a begin_frame() on the layer buffer, which GL allows
 * within an already-open window frame.  (Backends that permit only one open
 * frame at a time would need a within-frame render-target switch instead; no
 * such backend is supported here.)
 */
void RmlRenderInterface::
bind_target(LayerBuffer *lb, bool clear) {
  nassertv(lb != nullptr && _gsg != nullptr && _thread != nullptr);
  if (lb->_frame_open) return;

  if (!lb->_buf->begin_frame(GraphicsOutput::FM_render, _thread)) {
    rmlui_cat.error() << "begin_frame failed on layer buffer\n";
    return;
  }
  lb->_frame_open = true;

  if (clear) {
    lb->_buf->clear(_thread);
  }
  DisplayRegionPipelineReader dr_reader(lb->_dr, _thread);
  _gsg->prepare_display_region(&dr_reader);
}

void RmlRenderInterface::
begin_layer(LayerBuffer *lb) {
  bind_target(lb, /*clear=*/true);
}

/**
 * Ends lb's render pass (flushing its texture), then makes dest the active
 * target.  dest may be nullptr to return to the previously-active target (the
 * parent layer or the main window).
 */
void RmlRenderInterface::
end_layer(LayerBuffer *lb, LayerBuffer *dest) {
  if (lb != nullptr && lb->_frame_open) {
    lb->_buf->end_frame(GraphicsOutput::FM_render, _thread);
    lb->_frame_open = false;
  }

  if (dest != nullptr) {
    if (!dest->_frame_open) {
      if (!dest->_buf->begin_frame(GraphicsOutput::FM_render, _thread)) {
        rmlui_cat.error() << "begin_frame failed on destination layer\n";
        return;
      }
      dest->_frame_open = true;
    }
    DisplayRegionPipelineReader dr_reader(dest->_dr, _thread);
    _gsg->prepare_display_region(&dr_reader);
  } else {
    // Rebind the parent output without clearing it (the engine's own frame is
    // still open on it).  FM_refresh re-binds the output's framebuffer; this
    // matters when the parent is itself an offscreen GraphicsBuffer, whose FBO
    // is only rebound in FM_render/FM_refresh mode (FM_parasite would leave
    // the default framebuffer bound, which offscreen contexts may not have).
    _window->begin_frame(GraphicsOutput::FM_refresh, _thread);

    // Restore the region's own viewport/scissor.  The last prepared display
    // region belongs to a (window-sized) layer buffer, which is wrong whenever
    // this RmlRegion does not cover the whole window.
    if (_base_dr != nullptr) {
      DisplayRegionPipelineReader dr_reader(_base_dr, _thread);
      _gsg->prepare_display_region(&dr_reader);
    }
  }
}

// ===========================================================================
// render() — drive one frame
// ===========================================================================

/**
 * Called once per frame by RmlRegion::do_cull.  Drives context->Render(),
 * which dispatches into the RenderGeometry / PushLayer / CompositeLayers etc.
 * callbacks below.  trav, gsg, and current_thread must remain valid for the
 * duration of the call.
 */
void RmlRenderInterface::
render(Rml::Context *context, CullTraverser *trav,
       GraphicsStateGuardian *gsg, Thread *current_thread) {
  nassertv(context != nullptr);
  MutexHolder holder(_lock);
  _trav    = trav;
  _gsg     = gsg;
  _thread  = current_thread;
  _base_dr = trav->get_scene()->get_display_region();

  // Grow the pool here (between frames, with no layer FBO frame open) if a
  // previous frame ran out of buffers.  Allocating a GraphicsBuffer mid-frame
  // re-enters GraphicsEngine::open_windows and corrupts GL state, so it must
  // never happen during context->Render() below.  Headroom = peak concurrent
  // use plus the two implicit borrows snapshot_texture / drop-shadow may add.
  if (_pool_exhausted) {
    size_t target = std::max((size_t)rmlui_layer_pool_size,
                             (size_t)_peak_layers_in_use + 2);
    grow_pool(target);
    _pool_exhausted = false;
  }
  _layers_in_use = 0;
  _peak_layers_in_use = 0;

  _net_transform = trav->get_world_transform();
  // RmlUi v6 outputs premultiplied alpha (vertex colours are ColourbPremultiplied
  // and font/image textures are premultiplied), so blend with src factor O_one,
  // matching the reference backends' glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA).
  // Using O_incoming_alpha here would multiply RGB by alpha a second time,
  // darkening anti-aliased glyph edges.
  _net_state = RenderState::make(
    CullBinAttrib::make("unsorted", 0),
    DepthTestAttrib::make(RenderAttrib::M_none),
    DepthWriteAttrib::make(DepthWriteAttrib::M_off),
    ColorBlendAttrib::make(
      ColorBlendAttrib::M_add,
      ColorBlendAttrib::O_one,
      ColorBlendAttrib::O_one_minus_incoming_alpha),
    ColorAttrib::make_vertex()
  );
  ensure_shaders();

  _dimensions = context->GetDimensions();
  _css_transform = nullptr;

  _layer_stack.clear();
  LayerEntry base;
  base._handle  = 0;
  base._scissor = Rml::Rectanglei::FromSize({_dimensions.x, _dimensions.y});
  _layer_stack.push_back(base);

  context->Render();

  // Unclosed layers indicate an RmlUi bug; clean up defensively.
  while (_layer_stack.size() > 1) {
    LayerEntry top = _layer_stack.back();
    _layer_stack.pop_back();
    if (top._handle != 0) {
      LayerBuffer *lb = get_layer(top._handle);
      if (lb && lb->_frame_open) {
        end_layer(lb, nullptr);  // push-aware
      }
      free_layer(lb);
    }
  }
  _layer_stack.clear();

  _trav          = nullptr;
  _gsg           = nullptr;
  _thread        = nullptr;
  _base_dr       = nullptr;
  _net_transform = nullptr;
  _net_state     = nullptr;
}

// ===========================================================================
// Internal geometry helpers
// ===========================================================================

/**
 * Builds a Panda3D Geom from an RmlUi vertex/index span.
 */
PT(Geom) RmlRenderInterface::
make_geom(Rml::Span<const Rml::Vertex> vertices,
          Rml::Span<const int> indices,
          GeomEnums::UsageHint uh) {

  PT(GeomVertexData) vd = new GeomVertexData(
    "", GeomVertexFormat::get_v3c4t2(), uh);
  vd->unclean_set_num_rows((int)vertices.size());

  GeomVertexWriter vw(vd, InternalName::get_vertex());
  GeomVertexWriter cw(vd, InternalName::get_color());
  GeomVertexWriter tw(vd, InternalName::get_texcoord());
  for (const Rml::Vertex &v : vertices) {
    vw.add_data3f(LVector3f::right() * v.position.x +
                  LVector3f::up()    * v.position.y);
    cw.add_data4i(v.colour.red, v.colour.green,
                  v.colour.blue, v.colour.alpha);
    // Pass tex_coords verbatim.  RmlUi uses two conventions:
    //   image textures:  0..1 normalised, (0,0)=top-left (CSS).
    //   shader geometry: pixel-space position (gradient p0/p1 coords).
    // Both share the CSS y-down direction.  Image textures are loaded with
    // a Y-flip (see LoadTexture) so their (0,0) is already top-left from
    // Panda's perspective, keeping both conventions consistent.
    tw.add_data2f(v.tex_coord.x, v.tex_coord.y);
  }

  PT(GeomTriangles) tris = new GeomTriangles(uh);
  PT(GeomVertexArrayData) idata = tris->modify_vertices();
  idata->unclean_set_num_rows((int)indices.size());
  GeomVertexWriter iw(idata, 0);
  for (int i : indices) iw.add_data1i(i);

  PT(Geom) g = new Geom(vd);
  g->add_primitive(tris);
  return g;
}

/**
 * Submits a compiled Geom with the given render state to the active cull
 * handler, applying the current scissor and CSS transform.
 */
void RmlRenderInterface::
render_geom(const Geom *geom, const RenderState *state, Rml::Vector2f translation) {
  // If the active layer failed to bind (its buffer couldn't be opened), drop its
  // geometry instead of drawing it into the still-active parent target.
  if (!_layer_stack.empty() && _layer_stack.back()._suppressed) {
    return;
  }

  LVector3 off = LVector3::right() * translation.x + LVector3::up() * translation.y;

  CPT(RenderState) full = apply_scissor(_net_state->compose(state));

  // Apply the clip-mask stencil test, unless this draw is itself writing the
  // mask (in which case `state` already carries a StencilAttrib).
  if (_clip_mask_active &&
      full->get_attrib(StencilAttrib::get_class_slot()) == nullptr) {
    full = full->add_attrib(StencilAttrib::make(
      true, RenderAttrib::M_equal,
      StencilAttrib::SO_keep, StencilAttrib::SO_keep, StencilAttrib::SO_keep,
      _clip_mask_ref, ~0u, /*write_mask=*/0u));
  }

  CPT(TransformState) model =
    _css_transform != nullptr
      ? _css_transform->compose(TransformState::make_pos(off))
      : TransformState::make_pos(off);
  CPT(TransformState) xform =
    _trav->get_scene()->get_cs_world_transform()->compose(
      _net_transform->compose(model));

  CullableObject obj(geom, full, xform);
  _trav->get_cull_handler()->record_object(std::move(obj), _trav);
}

/**
 * Submits a fullscreen NDC quad with tex wired into the state's ShaderAttrib
 * as the u_source_tex sampler.
 */
void RmlRenderInterface::
composite_quad(CPT(RenderState) state, PT(Texture) tex) {
  CPT(RenderAttrib) sa = state->get_attrib(ShaderAttrib::get_class_slot());
  if (sa != nullptr && tex != nullptr) {
    sa = DCAST(ShaderAttrib, sa)->set_shader_input(
      InternalName::make("u_source_tex"), tex);
    state = state->add_attrib(sa);
  }
  state = _net_state->compose(state);

  if (_fsq == nullptr) _fsq = build_fullscreen_quad();
  CPT(TransformState) ident = _trav->get_scene()->get_cs_world_transform();
  CullableObject obj(_fsq, state, ident);
  _trav->get_cull_handler()->record_object(std::move(obj), _trav);
}

/**
 * Composes the active scissor rectangle (context pixel coords, y-down) onto
 * state as a ScissorAttrib, relative to the current render target's display
 * region.  Returns state unchanged when no scissor is active.
 */
CPT(RenderState) RmlRenderInterface::
apply_scissor(CPT(RenderState) state) const {
  if (!_scissor_active || _dimensions.x <= 0 || _dimensions.y <= 0) {
    return state;
  }
  const auto &r = _scissor_rect;
  LVecBase4 sc(
    r.Left()   / (PN_stdfloat)_dimensions.x,
    r.Right()  / (PN_stdfloat)_dimensions.x,
    1.f - r.Bottom() / (PN_stdfloat)_dimensions.y,
    1.f - r.Top()    / (PN_stdfloat)_dimensions.y);
  return state->add_attrib(ScissorAttrib::make(sc));
}

// ===========================================================================
// Required interface — CompileGeometry / RenderGeometry / ReleaseGeometry
// ===========================================================================

/**
 * Compiles RmlUi vertex/index data into a cached Panda3D Geom.
 */
Rml::CompiledGeometryHandle RmlRenderInterface::
CompileGeometry(Rml::Span<const Rml::Vertex> vertices,
                Rml::Span<const int> indices) {
  CompiledGeometry *cg = new CompiledGeometry;
  cg->_geom = make_geom(vertices, indices, GeomEnums::UH_static);
  return (Rml::CompiledGeometryHandle)cg;
}

/**
 * Draws a previously compiled geometry with an optional texture.
 */
void RmlRenderInterface::
RenderGeometry(Rml::CompiledGeometryHandle geometry,
               Rml::Vector2f translation,
               Rml::TextureHandle texture) {
  CompiledGeometry *cg = (CompiledGeometry *)geometry;
  if (!cg) return;

  CPT(RenderState) state;
  Texture *tex = (Texture *)texture;
  if (tex) {
    TextureStage *ts = TextureStage::get_default();
    CPT(TextureAttrib) ta = DCAST(TextureAttrib, TextureAttrib::make());
    ta = DCAST(TextureAttrib, ta->add_on_stage(ts, tex));
    state = RenderState::make(ta);
  } else {
    state = RenderState::make_empty();
  }
  render_geom(cg->_geom, state, translation);
}

/**
 * Frees a compiled geometry object.
 */
void RmlRenderInterface::
ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
  delete (CompiledGeometry *)geometry;
}

// ===========================================================================
// Required interface — LoadTexture / GenerateTexture / ReleaseTexture
// ===========================================================================

/**
 * Loads a texture from disk via Panda's VFS/TexturePool.  Y-flips the RAM
 * image so that UV (0,0) maps to the image's top-left (CSS convention).
 */
Rml::TextureHandle RmlRenderInterface::
LoadTexture(Rml::Vector2i &texture_dimensions, const Rml::String &source) {
  LoaderOptions opts;
  opts.set_auto_texture_scale(ATS_none);

  PT(Texture) pooled = TexturePool::load_texture(
    Filename::from_os_specific(source), 0, false, opts);
  if (!pooled) { texture_dimensions = {0, 0}; return 0; }

  // TexturePool::load_texture returns a cached instance shared with the rest of
  // the application (and with any later LoadTexture of the same source).  We
  // mutate the RAM image in place below (Y-flip + premultiply), so work on a
  // private copy; mutating the pooled texture would corrupt the shared image and
  // double-premultiply on a repeat load.
  PT(Texture) tex = pooled->make_copy();

  // Flip Y in RAM so UV (0,0) maps to the image top-left (CSS convention).
  // make_geom passes tex_coords verbatim; this keeps image and shader
  // geometry on the same coordinate convention.
  //
  // Only possible on uncompressed images: the byte-wise flip/premultiply
  // below assumes an uncompressed row layout and would overrun the smaller
  // buffer of a block-compressed RAM image (.dds/.txo).
  if (tex->has_ram_image() &&
      tex->get_ram_image_compression() != Texture::CM_off) {
    rmlui_cat.warning()
      << "Texture '" << source << "' has a compressed RAM image; skipping "
         "CSS y-flip and alpha premultiply.  It may render flipped or with "
         "incorrect blending; prefer an uncompressed format for UI images.\n";
  }
  else if (tex->has_ram_image()) {
    int xsize = tex->get_x_size();
    int ysize = tex->get_y_size();
    int row = xsize * tex->get_num_components() * tex->get_component_width();
    PTA_uchar img = tex->modify_ram_image();
    unsigned char *data = &img[0];
    for (int y = 0; y < ysize / 2; ++y) {
      unsigned char *a = data + y * row;
      unsigned char *b = data + (ysize - 1 - y) * row;
      for (int x = 0; x < row; ++x) std::swap(a[x], b[x]);
    }

    // RmlUi v6 blends in premultiplied-alpha space (see the _net_state blend in
    // render()).  TexturePool loads straight (non-premultiplied) RGBA, so
    // premultiply RGB by alpha here for 8-bit 4-component images.  Panda stores
    // BGRA on little-endian; the alpha byte is the last component regardless.
    if (tex->get_num_components() == 4 && tex->get_component_width() == 1) {
      for (int i = 0; i < ysize * row; i += 4) {
        unsigned int alpha = data[i + 3];
        data[i + 0] = (unsigned char)((data[i + 0] * alpha + 127) / 255);
        data[i + 1] = (unsigned char)((data[i + 1] * alpha + 127) / 255);
        data[i + 2] = (unsigned char)((data[i + 2] * alpha + 127) / 255);
      }
    }
  }

  // Nearest filtering keeps pixel-art UI sprites (icons, slices) crisp at or
  // near their native size; linear would smear them.  (The font atlas, by
  // contrast, uses linear in GenerateTexture for smooth glyph anti-aliasing.)
  tex->set_minfilter(SamplerState::FT_nearest);
  tex->set_magfilter(SamplerState::FT_nearest);

  int w = tex->get_orig_file_x_size();
  int h = tex->get_orig_file_y_size();
  if (!w && !h) { w = tex->get_x_size(); h = tex->get_y_size(); }
  texture_dimensions = {w, h};

  tex->ref();
  return (Rml::TextureHandle)tex.p();
}

/**
 * Creates a texture from raw RGBA data supplied by RmlUi (e.g. font atlases).
 */
Rml::TextureHandle RmlRenderInterface::
GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i dims) {
  PT(Texture) tex = new Texture;
  tex->setup_2d_texture(dims.x, dims.y, Texture::T_unsigned_byte, Texture::F_rgba);
  tex->set_size_padded(dims.x, dims.y);

  // Optional stem darkening: build a 256-entry alpha remap LUT applying
  // coverage^(1/gamma).  gamma == 1.0 leaves the data untouched (identity LUT).
  // Glyph atlases are premultiplied with rgb == alpha (mono glyphs) or already
  // premultiplied colour, so remapping all four channels by the alpha's scale
  // factor preserves premultiplication and thickens anti-aliased edges.
  const double gamma = rmlui_text_gamma;
  unsigned char alpha_lut[256];
  const bool remap = (gamma > 1.0001 || gamma < 0.9999);
  if (remap) {
    const double inv_gamma = 1.0 / gamma;
    for (int a = 0; a < 256; ++a) {
      double v = std::pow(a / 255.0, inv_gamma);
      alpha_lut[a] = (unsigned char)(v * 255.0 + 0.5);
    }
  }

  PTA_uchar img = tex->modify_ram_image();
  size_t src_stride = dims.x * 4;
  size_t dst_stride = tex->get_x_size() * 4;
  const unsigned char *src = source.data();
  unsigned char *dst = &img[0];
  // RmlUi supplies RGBA; Panda stores BGR(A) on little-endian, so swap R/B.
  // No Y-flip: make_geom passes tex_coords verbatim (CSS y-down convention).
  for (int y = 0; y < dims.y; ++y, src += src_stride, dst += dst_stride) {
    for (size_t i = 0; i < src_stride; i += 4) {
      unsigned char r = src[i + 2], g = src[i + 1], b = src[i + 0], a = src[i + 3];
      if (remap && a != 0) {
        // Scale every (premultiplied) channel by the alpha's remap ratio.
        unsigned int scale = ((unsigned int)alpha_lut[a] << 8) / a;  // 8.8 fixed
        r = (unsigned char)std::min(255u, (r * scale) >> 8);
        g = (unsigned char)std::min(255u, (g * scale) >> 8);
        b = (unsigned char)std::min(255u, (b * scale) >> 8);
        a = alpha_lut[a];
      }
      dst[i + 0] = r; dst[i + 1] = g; dst[i + 2] = b; dst[i + 3] = a;
    }
  }
  tex->set_wrap_u(SamplerState::WM_clamp);
  tex->set_wrap_v(SamplerState::WM_clamp);
  // Linear filtering keeps anti-aliased font glyphs smooth; the reference
  // backends use GL_LINEAR here too.  Nearest sampling makes text look jagged.
  tex->set_minfilter(SamplerState::FT_linear);
  tex->set_magfilter(SamplerState::FT_linear);
  tex->ref();
  return (Rml::TextureHandle)tex.p();
}

/**
 * Releases a texture created by LoadTexture or GenerateTexture.
 */
void RmlRenderInterface::
ReleaseTexture(Rml::TextureHandle texture) {
  Texture *tex = (Texture *)texture;
  if (tex) unref_delete(tex);
}

// ===========================================================================
// Required interface — EnableScissorRegion / SetScissorRegion / SetTransform
// ===========================================================================

/**
 * Enables or disables scissor clipping for subsequent geometry.
 */
void RmlRenderInterface::
EnableScissorRegion(bool enable) {
  _scissor_active = enable;
}

/**
 * Sets the scissor rectangle (CSS pixel coords, y-down).
 */
void RmlRenderInterface::
SetScissorRegion(Rml::Rectanglei region) {
  _scissor_rect = region;
}

/**
 * Sets the CSS transform matrix for subsequent geometry, or nullptr to reset
 * to identity.
 *
 * Rml::Matrix4f is column-major (RMLUI_MATRIX_ROW_MAJOR not defined).
 * data() returns elements column-by-column: d[0..3]=col0, d[4..7]=col1, …
 * Reading d[0],d[4],d[8],d[12] as Panda row 0 transposes the Rml matrix.
 * Panda re-transposes on upload, so the round-trip is identity — the same
 * matrix values reach GLSL as were in the Rml Matrix4f.
 */
void RmlRenderInterface::
SetTransform(const Rml::Matrix4f *transform) {
  if (transform == nullptr) {
    _css_transform = nullptr;
    return;
  }
  const float *d = transform->data();
  LMatrix4f mat(
    d[ 0], d[ 4], d[ 8], d[12],
    d[ 1], d[ 5], d[ 9], d[13],
    d[ 2], d[ 6], d[10], d[14],
    d[ 3], d[ 7], d[11], d[15]);
  _css_transform = TransformState::make_mat(mat);
}

// ===========================================================================
// Clip mask — non-rectangular clipping via the stencil buffer
// ===========================================================================

/**
 * Enables or disables stencil-based clip masking for subsequent geometry.
 * While enabled, render_geom() composes a stencil test that passes only where
 * the clip mask (written by RenderToClipMask) is set.
 */
void RmlRenderInterface::
EnableClipMask(bool enable) {
  _clip_mask_active = enable;
  if (enable) {
    // Reset the reference for a fresh mask; RenderToClipMask(Set) clears the
    // stencil and writes this value, Intersect raises it.
    _clip_mask_ref = 1;

    // Clip masking on the BASE layer writes/tests the main window's stencil
    // buffer; if the window was created without stencil bits the mask silently
    // has no effect (content is left unclipped).  Layer buffers always allocate
    // 8 stencil bits, so masking inside a pushed layer still works.  Warn once
    // so a missing window stencil buffer isn't a silent mystery.
    if (!_warned_no_stencil && _layer_stack.size() <= 1 && _window != nullptr &&
        _window->get_fb_properties().get_stencil_bits() == 0) {
      _warned_no_stencil = true;
      rmlui_cat.warning()
        << "border-radius / transform clipping needs a stencil buffer, but the "
           "window has none (framebuffer-stencil-bits 0).  Such clipping will "
           "be ignored on the base layer.  Request stencil bits on the window "
           "(e.g. FrameBufferProperties::set_stencil_bits(8)).\n";
    }
  }
}

/**
 * Writes the given geometry into the stencil buffer to build the clip mask.
 * Mirrors the reference backend's stencil logic, expressed as StencilAttribs so
 * it flows through Panda's deferred cull/draw pipeline in submission order.
 */
void RmlRenderInterface::
RenderToClipMask(Rml::ClipMaskOperation operation,
                 Rml::CompiledGeometryHandle geometry,
                 Rml::Vector2f translation) {
  CompiledGeometry *cg = (CompiledGeometry *)geometry;
  if (cg == nullptr) return;

  CPT(RenderAttrib) stencil;
  switch (operation) {
  case Rml::ClipMaskOperation::Set:
    // Clear the stencil to 0, write _clip_mask_ref (1) where geometry covers.
    _clip_mask_ref = 1;
    stencil = StencilAttrib::make_with_clear(
      true, RenderAttrib::M_always,
      StencilAttrib::SO_keep, StencilAttrib::SO_keep, StencilAttrib::SO_replace,
      _clip_mask_ref, ~0u, ~0u,
      /*clear=*/true, /*clear_value=*/0);
    break;

  case Rml::ClipMaskOperation::SetInverse:
    // Clear the stencil to 1, write 0 where geometry covers; the test for
    // _clip_mask_ref (1) then passes only *outside* the geometry.
    _clip_mask_ref = 1;
    stencil = StencilAttrib::make_with_clear(
      true, RenderAttrib::M_always,
      StencilAttrib::SO_keep, StencilAttrib::SO_keep, StencilAttrib::SO_replace,
      /*reference=*/0, ~0u, ~0u,
      /*clear=*/true, /*clear_value=*/1);
    break;

  case Rml::ClipMaskOperation::Intersect:
    // Increment the stencil where geometry covers; raise the test reference so
    // only pixels covered by every intersected mask pass.
    ++_clip_mask_ref;
    stencil = StencilAttrib::make(
      true, RenderAttrib::M_always,
      StencilAttrib::SO_keep, StencilAttrib::SO_keep, StencilAttrib::SO_increment,
      _clip_mask_ref - 1, ~0u, ~0u);
    break;

  default:
    return;
  }

  // Mask writes touch only the stencil buffer, never color.
  CPT(RenderState) state = RenderState::make(
    stencil, ColorWriteAttrib::make(ColorWriteAttrib::C_off));
  render_geom(cg->_geom, state, translation);
}

// ===========================================================================
// Layer interface — PushLayer / CompositeLayers / PopLayer
// ===========================================================================

/**
 * Allocates a new offscreen layer and binds it as the current render target.
 */
Rml::LayerHandle RmlRenderInterface::
PushLayer() {
  LayerBuffer *lb = alloc_layer();
  if (!lb) return 0;

  // Resize the layer buffer (and scratch buffers) to match the window if needed.
  if (_window) {
    int w = _window->get_x_size();
    int h = _window->get_y_size();
    if (lb->_tex->get_x_size() != w || lb->_tex->get_y_size() != h) {
      lb->_buf->set_size_and_recalc(w, h);
    }
    for (int si = 0; si < 2; ++si) {
      if (_scratch[si] &&
          (_scratch[si]->_tex->get_x_size() != w ||
           _scratch[si]->_tex->get_y_size() != h)) {
        _scratch[si]->_buf->set_size_and_recalc(w, h);
      }
    }
  }

  begin_layer(lb);

  Rml::LayerHandle handle = reinterpret_cast<Rml::LayerHandle>(lb) + 1;

  LayerEntry entry;
  entry._handle  = handle;
  entry._scissor = _scissor_rect;
  // If the layer's buffer did not bind (begin_frame failed), suppress its
  // geometry so it isn't drawn into the still-active parent target.
  entry._suppressed = !lb->_frame_open;
  _layer_stack.push_back(entry);

  return handle;
}

/**
 * Composites the source layer onto the destination layer, running the filter
 * chain in between.
 */
void RmlRenderInterface::
CompositeLayers(Rml::LayerHandle source_handle,
                Rml::LayerHandle destination_handle,
                Rml::BlendMode blend_mode,
                Rml::Span<const Rml::CompiledFilterHandle> filters) {

  LayerBuffer *src_lb  = get_layer(source_handle);
  LayerBuffer *dest_lb = get_layer(destination_handle);
  if (src_lb == nullptr || !src_lb->_frame_open) return;

  end_layer(src_lb, nullptr);
  PT(Texture) result = apply_filters(src_lb->_tex, filters);

  // Bind the destination (composite the source ONTO it, preserving its existing
  // content — do not clear).  dest_lb == nullptr means composite to whatever is
  // currently active (the main window / parent), which end_layer already
  // restored above.
  if (dest_lb != nullptr && !dest_lb->_frame_open) {
    bind_target(dest_lb, /*clear=*/false);
  }

  CPT(RenderState) state = _shader_passthrough;
  CPT(RenderAttrib) sa = state->get_attrib(ShaderAttrib::get_class_slot());
  sa = DCAST(ShaderAttrib, sa)->set_shader_input(
    InternalName::make("u_blend_factor"), LVecBase4f(1.f, 0.f, 0.f, 0.f));
  state = state->add_attrib(sa);

  if (blend_mode == Rml::BlendMode::Replace) {
    state = state->add_attrib(ColorBlendAttrib::make(
      ColorBlendAttrib::M_add,
      ColorBlendAttrib::O_one, ColorBlendAttrib::O_zero));
  }

  // The reference backends keep the scissor test enabled while compositing, so
  // a filtered element inside an overflow clip stays clipped; mirror that.
  state = apply_scissor(state);

  composite_quad(state, result);
}

/**
 * Pops the current layer, ending its frame and freeing it back to the pool.
 * Restores the scissor rect that was active before the matching PushLayer.
 */
void RmlRenderInterface::
PopLayer() {
  if (_layer_stack.size() <= 1) return;
  LayerEntry top = _layer_stack.back();
  _layer_stack.pop_back();

  // Restore scissor to the state saved at PushLayer time.
  _scissor_rect = top._scissor;

  if (top._handle == 0) return;
  LayerBuffer *lb = get_layer(top._handle);
  if (!lb) return;

  // End this layer's frame if still open.  CompositeLayers should have
  // already done this, but guard against partially-rendered frames.
  if (lb->_frame_open) {
    LayerBuffer *parent = (_layer_stack.empty() ||
                           _layer_stack.back()._handle == 0)
                          ? nullptr
                          : get_layer(_layer_stack.back()._handle);
    end_layer(lb, parent);
  }
  free_layer(lb);
}

// ===========================================================================
// SaveLayerAsTexture / SaveLayerAsMaskImage
// ===========================================================================

/**
 * Composites src_tex into a brand-new independent RGBA texture and returns it.
 *
 * A pool buffer is borrowed to render into, then given a fresh replacement
 * texture so the buffer stays valid and reusable; the rendered texture is
 * detached and returned.  The returned texture never aliases any live layer
 * render target, so the caller can hold it for as long as it likes.
 *
 * When a scissor is active (RmlUi anchors it at the origin via FromSize for the
 * box-shadow/CallbackTexture path), the scissor sub-rectangle of the source is
 * scaled to fill the full 0..1 of the returned texture, matching the reference
 * backend's scissor-cropped result.  RmlUi reports the CallbackTexture's size as
 * the scissor size and samples it with rect/dimensions UVs (see
 * ElementImage.cpp), so the content must fill the texture's UV range.
 */
PT(Texture) RmlRenderInterface::
snapshot_texture(Texture *src_tex) {
  if (src_tex == nullptr) return nullptr;

  LayerBuffer *snap_lb = alloc_layer();
  if (snap_lb == nullptr) return nullptr;

  // If a scissor is active, sample only its (origin-anchored) sub-rectangle of
  // the source and scale it to fill the output, so the snapshot is effectively
  // cropped to the scissor with its content filling the texture's UV range.
  LVecBase2f uv_offset(0.f, 0.f);
  LVecBase2f uv_scale(1.f, 1.f);
  if (_scissor_active && _dimensions.x > 0 && _dimensions.y > 0) {
    const auto &r = _scissor_rect;
    int sw = r.Right() - r.Left();
    int sh = r.Bottom() - r.Top();
    if (sw > 0 && sh > 0) {
      uv_offset.set(r.Left() / (float)_dimensions.x,
                    r.Top()  / (float)_dimensions.y);
      uv_scale.set(sw / (float)_dimensions.x, sh / (float)_dimensions.y);
    }
  }

  begin_layer(snap_lb);
  CPT(RenderState) st = _shader_passthrough;
  CPT(RenderAttrib) sa = st->get_attrib(ShaderAttrib::get_class_slot());
  sa = DCAST(ShaderAttrib, sa)->set_shader_input(
    InternalName::make("u_blend_factor"), LVecBase4f(1.f, 0.f, 0.f, 0.f));
  sa = DCAST(ShaderAttrib, sa)->set_shader_input(
    InternalName::make("u_uv_offset"), uv_offset);
  sa = DCAST(ShaderAttrib, sa)->set_shader_input(
    InternalName::make("u_uv_scale"), uv_scale);
  st = st->add_attrib(sa);
  composite_quad(st, src_tex);
  end_layer(snap_lb, nullptr);

  // Detach the rendered texture and give the pool buffer a fresh replacement
  // bound as its render target, so the buffer remains a valid, reusable pool
  // entry (alloc_layer requires _tex != nullptr).  This avoids both the
  // dead-pool-slot retirement and any aliasing of the live render target.
  PT(Texture) result = snap_lb->_tex;

  PT(Texture) fresh = new Texture(snap_lb->_buf->get_name());
  int w = result->get_x_size();
  int h = result->get_y_size();
  fresh->setup_2d_texture(w, h, Texture::T_unsigned_byte, Texture::F_rgba);
  fresh->set_wrap_u(SamplerState::WM_clamp);
  fresh->set_wrap_v(SamplerState::WM_clamp);
  fresh->set_minfilter(SamplerState::FT_linear);
  fresh->set_magfilter(SamplerState::FT_linear);
  snap_lb->_buf->clear_render_textures();
  snap_lb->_buf->add_render_texture(fresh, GraphicsOutput::RTM_bind_or_copy,
                                    GraphicsOutput::RTP_color);
  snap_lb->_tex = fresh;

  free_layer(snap_lb);

  return result;
}

/**
 * Snapshots the current layer's contents into a new texture handle that the
 * caller owns.  The layer remains active for further rendering.
 */
Rml::TextureHandle RmlRenderInterface::
SaveLayerAsTexture() {
  if (_layer_stack.empty()) return 0;
  const LayerEntry &top = _layer_stack.back();
  if (top._handle == 0) return 0;

  LayerBuffer *lb = get_layer(top._handle);
  if (!lb || !lb->_tex) return 0;

  // End the layer's pass (push-aware) so its texture is complete before we
  // sample it; snapshot_texture borrows another buffer and restores state.
  if (lb->_frame_open) {
    end_layer(lb, nullptr);
  }

  PT(Texture) snap_tex = snapshot_texture(lb->_tex);

  // Restart the layer's frame for any further rendering into it, PRESERVING its
  // accumulated content (clear=false) — the RmlUi contract is that the layer
  // stays active with its existing contents after a snapshot.
  bind_target(lb, /*clear=*/false);

  if (snap_tex == nullptr) return 0;

  // RmlUi owns the returned handle and releases it via ReleaseTexture, which
  // calls unref_delete; balance that with a manual ref() here.
  snap_tex->ref();
  return (Rml::TextureHandle)snap_tex.p();
}

/**
 * Snapshots the current layer as a mask-image filter handle.
 */
Rml::CompiledFilterHandle RmlRenderInterface::
SaveLayerAsMaskImage() {
  if (_layer_stack.empty()) return 0;
  const LayerEntry &top = _layer_stack.back();
  if (top._handle == 0) return 0;

  LayerBuffer *lb = get_layer(top._handle);
  if (!lb || !lb->_tex) return 0;

  if (lb->_frame_open) {
    end_layer(lb, nullptr);
  }

  // Snapshot into an independent texture; the live layer texture would be
  // cleared by the begin_layer() restart below before the mask is consumed.
  PT(Texture) mask_tex = snapshot_texture(lb->_tex);

  // Restart the layer for further rendering, preserving its content (the mask is
  // an independent snapshot, so the live layer is untouched by it).
  bind_target(lb, /*clear=*/false);

  if (mask_tex == nullptr) return 0;

  CompiledFilterData *fd = new CompiledFilterData;
  fd->_type     = FilterType::MASK_IMAGE;
  // _mask_tex is a PT, so it keeps the snapshot alive; ReleaseFilter just
  // deletes the CompiledFilterData and the PT drops the reference.  No ref().
  fd->_mask_tex = mask_tex;
  return reinterpret_cast<Rml::CompiledFilterHandle>(fd);
}

// ===========================================================================
// apply_filters — ping-pong filter chain
// ===========================================================================

/**
 * Applies the filter chain to src using ping-pong scratch buffers.
 * Returns the texture holding the final result (may be src if no filters).
 */
PT(Texture) RmlRenderInterface::
apply_filters(PT(Texture) src,
              Rml::Span<const Rml::CompiledFilterHandle> filters) {
  if (filters.empty()) return src;

  int w = _window ? _window->get_x_size() : 1;
  int h = _window ? _window->get_y_size() : 1;
  float inv_w = w > 0 ? 1.f / (float)w : 1.f;
  float inv_h = h > 0 ? 1.f / (float)h : 1.f;

  PT(Texture) ping = src;
  int scratch_idx = 0;

  auto blit = [&](CPT(RenderState) state) {
    LayerBuffer *dest = _scratch[scratch_idx ^ 1];
    if (dest) {
      begin_layer(dest);
      composite_quad(state, ping);
      end_layer(dest, nullptr);
      ping = dest->_tex;
    }
    scratch_idx ^= 1;
  };

  auto make_blur_state = [&](LVecBase2f axis, PTA_float &weights) {
    // Clamp sampling to the scissor sub-rectangle when one is active, so the
    // blur does not pull in content from outside the clip (the reference
    // backends limit the blur's tex coords the same way).
    LVecBase2f uv_min(0.f, 0.f);
    LVecBase2f uv_max(1.f, 1.f);
    if (_scissor_active && _dimensions.x > 0 && _dimensions.y > 0) {
      const auto &r = _scissor_rect;
      uv_min.set(r.Left() / (float)_dimensions.x,
                 1.f - r.Bottom() / (float)_dimensions.y);
      uv_max.set(r.Right() / (float)_dimensions.x,
                 1.f - r.Top() / (float)_dimensions.y);
    }
    CPT(RenderState) st = _shader_blur;
    CPT(RenderAttrib) sa = st->get_attrib(ShaderAttrib::get_class_slot());
    sa = DCAST(ShaderAttrib, sa)->set_shader_input(InternalName::make("u_weights"),  weights);
    sa = DCAST(ShaderAttrib, sa)->set_shader_input(InternalName::make("u_axis"),     axis);
    sa = DCAST(ShaderAttrib, sa)->set_shader_input(InternalName::make("u_inv_size"), LVecBase2f(inv_w, inv_h));
    sa = DCAST(ShaderAttrib, sa)->set_shader_input(InternalName::make("u_uv_min"),   uv_min);
    sa = DCAST(ShaderAttrib, sa)->set_shader_input(InternalName::make("u_uv_max"),   uv_max);
    return st->add_attrib(sa);
  };

  auto do_blur_pass = [&](LVecBase2f axis, PTA_float &weights) {
    blit(make_blur_state(axis, weights));
  };

  for (const Rml::CompiledFilterHandle fh : filters) {
    if (!fh) continue;
    const CompiledFilterData &fd = *reinterpret_cast<const CompiledFilterData *>(fh);

    switch (fd._type) {

    case FilterType::PASSTHROUGH: {
      CPT(RenderState) st = _shader_passthrough;
      CPT(RenderAttrib) sa = st->get_attrib(ShaderAttrib::get_class_slot());
      sa = DCAST(ShaderAttrib, sa)->set_shader_input(
        InternalName::make("u_blend_factor"), LVecBase4f(fd._blend_factor, 0.f, 0.f, 0.f));
      blit(st->add_attrib(sa));
      break;
    }

    case FilterType::COLOR_MATRIX: {
      CPT(RenderState) st = _shader_color_matrix;
      PTA_LMatrix4f mat;
      mat.push_back(UnalignedLMatrix4f(fd._color_matrix));
      CPT(RenderAttrib) sa = st->get_attrib(ShaderAttrib::get_class_slot());
      sa = DCAST(ShaderAttrib, sa)->set_shader_input(
        InternalName::make("u_color_matrix"), mat);
      blit(st->add_attrib(sa));
      break;
    }

    case FilterType::BLUR: {
      float sigma = std::max(fd._sigma, 0.1f);
      float wf[BLUR_WEIGHTS];
      compute_blur_weights(sigma, wf);
      PTA_float weights;
      for (int i = 0; i < BLUR_WEIGHTS; ++i) weights.push_back(wf[i]);
      do_blur_pass({1.f, 0.f}, weights);
      do_blur_pass({0.f, 1.f}, weights);
      break;
    }

    case FilterType::DROP_SHADOW: {
      PT(Texture) original = ping;
      PT(Texture) shadow_src = original;

      // Blur the shadow silhouette.  A preceding filter may have left
      // `original` in a scratch buffer, and the two blur passes together
      // would cycle back into that same buffer (begin_layer clears it),
      // destroying the original.  So blur through the free scratch buffer
      // into a borrowed pool buffer instead of ping-ponging.
      LayerBuffer *borrowed = nullptr;
      if (fd._sigma >= 0.5f) {
        float wf2[BLUR_WEIGHTS];
        compute_blur_weights(fd._sigma, wf2);
        PTA_float weights;
        for (int i = 0; i < BLUR_WEIGHTS; ++i) weights.push_back(wf2[i]);

        LayerBuffer *tmp = _scratch[scratch_idx ^ 1];  // never holds `original`
        borrowed = alloc_layer();
        if (tmp != nullptr && borrowed != nullptr) {
          begin_layer(tmp);
          composite_quad(make_blur_state({1.f, 0.f}, weights), original);
          end_layer(tmp, nullptr);

          begin_layer(borrowed);
          composite_quad(make_blur_state({0.f, 1.f}, weights), tmp->_tex);
          end_layer(borrowed, nullptr);
          shadow_src = borrowed->_tex;
        }
        // If no buffer could be borrowed, degrade to a sharp shadow.
      }

      // Shadow pass: render the tinted/offset shadow into the scratch buffer
      // that does not hold `original`, then composite the original on top in
      // the same FBO (frame left open, no clear in between).
      {
        LayerBuffer *dest = _scratch[scratch_idx ^ 1];
        if (dest) {
          begin_layer(dest);
          CPT(RenderState) st = _shader_drop_shadow;
          CPT(RenderAttrib) sa = st->get_attrib(ShaderAttrib::get_class_slot());
          sa = DCAST(ShaderAttrib, sa)->set_shader_input(InternalName::make("u_shadow_color"), fd._color);
          sa = DCAST(ShaderAttrib, sa)->set_shader_input(InternalName::make("u_offset"),       fd._offset);
          sa = DCAST(ShaderAttrib, sa)->set_shader_input(InternalName::make("u_inv_size"),     LVecBase2f(inv_w, inv_h));
          composite_quad(st->add_attrib(sa), shadow_src);

          // Premultiplied "over": the layer texture is premultiplied, so the
          // source factor is O_one.
          CPT(RenderState) over_st = _shader_passthrough;
          CPT(RenderAttrib) osa = over_st->get_attrib(ShaderAttrib::get_class_slot());
          osa = DCAST(ShaderAttrib, osa)->set_shader_input(
            InternalName::make("u_blend_factor"), LVecBase4f(1.f, 0.f, 0.f, 0.f));
          over_st = over_st->add_attrib(osa)->add_attrib(
            ColorBlendAttrib::make(ColorBlendAttrib::M_add,
                                   ColorBlendAttrib::O_one,
                                   ColorBlendAttrib::O_one_minus_incoming_alpha));
          composite_quad(over_st, original);

          end_layer(dest, nullptr);
          ping = dest->_tex;
        }
        scratch_idx ^= 1;
      }

      if (borrowed != nullptr) {
        free_layer(borrowed);
      }
      break;
    }

    case FilterType::MASK_IMAGE: {
      if (!fd._mask_tex) break;
      CPT(RenderState) st = _shader_blend_mask;
      CPT(RenderAttrib) sa = st->get_attrib(ShaderAttrib::get_class_slot());
      sa = DCAST(ShaderAttrib, sa)->set_shader_input(InternalName::make("u_mask_tex"), fd._mask_tex);
      blit(st->add_attrib(sa));
      break;
    }

    case FilterType::INVALID:
    default: break;
    }
  }
  return ping;
}

// ===========================================================================
// CompileFilter / ReleaseFilter
// ===========================================================================

/**
 * Compiles an RmlUi CSS filter into a filter data struct.
 */
Rml::CompiledFilterHandle RmlRenderInterface::
CompileFilter(const Rml::String &name, const Rml::Dictionary &parameters) {
  CompiledFilterData *fd = new CompiledFilterData;

  if (name == "opacity") {
    fd->_type = FilterType::PASSTHROUGH;
    fd->_blend_factor = Rml::Get(parameters, "value", 1.f);
  }
  else if (name == "blur") {
    fd->_type  = FilterType::BLUR;
    fd->_sigma = Rml::Get(parameters, "sigma", 1.f);
  }
  else if (name == "drop-shadow") {
    fd->_type  = FilterType::DROP_SHADOW;
    fd->_sigma = Rml::Get(parameters, "sigma", 0.f);
    // Premultiply alpha to match the premultiplied-alpha render pipeline.
    Rml::Colourb c = Rml::Get(parameters, "color", Rml::Colourb());
    float a = c.alpha / 255.f;
    fd->_color = LColor(c.red/255.f * a, c.green/255.f * a, c.blue/255.f * a, a);
    Rml::Vector2f off = Rml::Get(parameters, "offset", Rml::Vector2f(0.f));
    fd->_offset = LVecBase2f(off.x, off.y);
  }
  // The color matrices below are written in mathematical (column-vector) row
  // order, matching the reference backend and the W3C filter-effects spec; the
  // constant term sits in the fourth column.  s_frag_color_matrix applies them
  // as c * M, which given Panda's untransposed row-major upload computes the
  // intended M * c.
  else if (name == "brightness") {
    fd->_type = FilterType::COLOR_MATRIX;
    float v = Rml::Get(parameters, "value", 1.f);
    fd->_color_matrix = LMatrix4f(v, 0, 0, 0, 0, v, 0, 0, 0, 0, v, 0, 0, 0, 0, 1);
  }
  else if (name == "contrast") {
    fd->_type = FilterType::COLOR_MATRIX;
    float v = Rml::Get(parameters, "value", 1.f);
    float g = 0.5f - 0.5f * v;
    fd->_color_matrix = LMatrix4f(v, 0, 0, g, 0, v, 0, g, 0, 0, v, g, 0, 0, 0, 1);
  }
  else if (name == "invert") {
    fd->_type = FilterType::COLOR_MATRIX;
    float v = std::min(std::max(Rml::Get(parameters, "value", 1.f), 0.f), 1.f);
    float i = 1.f - 2.f * v;
    fd->_color_matrix = LMatrix4f(i, 0, 0, v, 0, i, 0, v, 0, 0, i, v, 0, 0, 0, 1);
  }
  else if (name == "grayscale") {
    fd->_type = FilterType::COLOR_MATRIX;
    float v = Rml::Get(parameters, "value", 1.f), rv = 1.f - v;
    float r = v * 0.2126f, g = v * 0.7152f, b = v * 0.0722f;
    fd->_color_matrix = LMatrix4f(
      r + rv, g,      b,      0,
      r,      g + rv, b,      0,
      r,      g,      b + rv, 0,
      0,      0,      0,      1);
  }
  else if (name == "sepia") {
    fd->_type = FilterType::COLOR_MATRIX;
    float v = Rml::Get(parameters, "value", 1.f), rv = 1.f - v;
    fd->_color_matrix = LMatrix4f(
      v * 0.393f + rv, v * 0.769f,       v * 0.189f,       0,
      v * 0.349f,      v * 0.686f + rv,  v * 0.168f,       0,
      v * 0.272f,      v * 0.534f,       v * 0.131f + rv,  0,
      0,               0,                0,                 1);
  }
  else if (name == "hue-rotate") {
    fd->_type = FilterType::COLOR_MATRIX;
    float a = Rml::Get(parameters, "value", 0.f);
    float s = sinf(a), c = cosf(a);
    fd->_color_matrix = LMatrix4f(
      0.213f + 0.787f*c - 0.213f*s, 0.715f - 0.715f*c - 0.715f*s, 0.072f - 0.072f*c + 0.928f*s, 0,
      0.213f - 0.213f*c + 0.143f*s, 0.715f + 0.285f*c + 0.140f*s, 0.072f - 0.072f*c - 0.283f*s, 0,
      0.213f - 0.213f*c - 0.787f*s, 0.715f - 0.715f*c + 0.715f*s, 0.072f + 0.928f*c + 0.072f*s, 0,
      0,                             0,                             0,                             1);
  }
  else if (name == "saturate") {
    fd->_type = FilterType::COLOR_MATRIX;
    float v = Rml::Get(parameters, "value", 1.f);
    fd->_color_matrix = LMatrix4f(
      0.213f + 0.787f*v, 0.715f - 0.715f*v, 0.072f - 0.072f*v, 0,
      0.213f - 0.213f*v, 0.715f + 0.285f*v, 0.072f - 0.072f*v, 0,
      0.213f - 0.213f*v, 0.715f - 0.715f*v, 0.072f + 0.928f*v, 0,
      0,                 0,                 0,                 1);
  }
  else {
    delete fd;
    rmlui_cat.warning() << "Unknown filter '" << name << "'\n";
    return 0;
  }
  return reinterpret_cast<Rml::CompiledFilterHandle>(fd);
}

/**
 * Frees a compiled filter.
 */
void RmlRenderInterface::
ReleaseFilter(Rml::CompiledFilterHandle filter) {
  delete reinterpret_cast<CompiledFilterData *>(filter);
}

// ===========================================================================
// CompileShader / RenderShader / ReleaseShader
// ===========================================================================

/**
 * Compiles an RmlUi decorator shader (gradients, procedural creation shader).
 */
Rml::CompiledShaderHandle RmlRenderInterface::
CompileShader(const Rml::String &name, const Rml::Dictionary &parameters) {
  CompiledShaderData *sd = new CompiledShaderData;

  auto load_stops = [&](const Rml::Dictionary &p) {
    auto it = p.find("color_stop_list");
    if (it == p.end() ||
        it->second.GetType() != Rml::Variant::COLORSTOPLIST) return;
    const Rml::ColorStopList &stops =
      it->second.GetReference<Rml::ColorStopList>();
    int n = std::min((int)stops.size(), MAX_STOPS);
    sd->_num_stops = n;
    for (int i = 0; i < n; ++i) {
      sd->_stop_positions[i] = stops[i].position.number;
      const auto &c = stops[i].color;
      sd->_stop_colors[i] = LVecBase4f(
        c.red/255.f, c.green/255.f, c.blue/255.f, c.alpha/255.f);
    }
  };

  if (name == "linear-gradient") {
    sd->_type = ShaderType::GRADIENT;
    bool rep = Rml::Get(parameters, "repeating", false);
    sd->_gradient_func = rep ? GradientFunc::REPEATING_LINEAR : GradientFunc::LINEAR;
    Rml::Vector2f p0 = Rml::Get(parameters, "p0", Rml::Vector2f(0.f));
    Rml::Vector2f p1 = Rml::Get(parameters, "p1", Rml::Vector2f(0.f));
    sd->_p = LVecBase2f(p0.x, p0.y);
    sd->_v = LVecBase2f(p1.x - p0.x, p1.y - p0.y);
    load_stops(parameters);
  }
  else if (name == "radial-gradient") {
    sd->_type = ShaderType::GRADIENT;
    bool rep = Rml::Get(parameters, "repeating", false);
    sd->_gradient_func = rep ? GradientFunc::REPEATING_RADIAL : GradientFunc::RADIAL;
    Rml::Vector2f c  = Rml::Get(parameters, "center", Rml::Vector2f(0.f));
    Rml::Vector2f r  = Rml::Get(parameters, "radius", Rml::Vector2f(1.f));
    sd->_p = LVecBase2f(c.x, c.y);
    sd->_v = LVecBase2f(1.f / r.x, 1.f / r.y);
    load_stops(parameters);
  }
  else if (name == "conic-gradient") {
    sd->_type = ShaderType::GRADIENT;
    bool rep = Rml::Get(parameters, "repeating", false);
    sd->_gradient_func = rep ? GradientFunc::REPEATING_CONIC : GradientFunc::CONIC;
    Rml::Vector2f c = Rml::Get(parameters, "center", Rml::Vector2f(0.f));
    float angle     = Rml::Get(parameters, "angle", 0.f);
    sd->_p = LVecBase2f(c.x, c.y);
    sd->_v = LVecBase2f(cosf(angle), sinf(angle));
    load_stops(parameters);
  }
  else if (name == "shader") {
    Rml::String val = Rml::Get(parameters, "value", Rml::String());
    if (val == "creation") {
      sd->_type = ShaderType::CREATION;
      Rml::Vector2f dim = Rml::Get(parameters, "dimensions", Rml::Vector2f(1.f));
      sd->_dimensions = LVecBase2f(dim.x, dim.y);
    } else {
      delete sd;
      rmlui_cat.warning() << "Unknown shader value '" << val << "'\n";
      return 0;
    }
  }
  else {
    delete sd;
    rmlui_cat.warning() << "Unknown shader '" << name << "'\n";
    return 0;
  }
  return reinterpret_cast<Rml::CompiledShaderHandle>(sd);
}

/**
 * Renders a compiled shader over the given geometry.
 */
void RmlRenderInterface::
RenderShader(Rml::CompiledShaderHandle shader_handle,
             Rml::CompiledGeometryHandle geometry_handle,
             Rml::Vector2f translation,
             Rml::TextureHandle /*texture*/) {
  if (!shader_handle || !geometry_handle) return;

  const CompiledShaderData &sd =
    *reinterpret_cast<const CompiledShaderData *>(shader_handle);
  const CompiledGeometry *cg =
    reinterpret_cast<const CompiledGeometry *>(geometry_handle);

  CPT(RenderState) state;

  switch (sd._type) {
  case ShaderType::GRADIENT: {
    state = _shader_gradient;
    CPT(RenderAttrib) sa = state->get_attrib(ShaderAttrib::get_class_slot());
    // u_func and u_num_stops are declared as int in the shader; passing as
    // a 4-component float vector lets Panda's shader system handle the
    // int/float cast on upload.
    sa = DCAST(ShaderAttrib, sa)->set_shader_input(
      InternalName::make("u_func"), LVecBase4f((float)sd._gradient_func, 0, 0, 0));
    sa = DCAST(ShaderAttrib, sa)->set_shader_input(
      InternalName::make("u_p"), sd._p);
    sa = DCAST(ShaderAttrib, sa)->set_shader_input(
      InternalName::make("u_v"), sd._v);
    sa = DCAST(ShaderAttrib, sa)->set_shader_input(
      InternalName::make("u_num_stops"), LVecBase4f((float)sd._num_stops, 0, 0, 0));
    PTA_LVecBase4f colors;
    PTA_float positions;
    for (int i = 0; i < sd._num_stops; ++i) {
      colors.push_back(sd._stop_colors[i]);
      positions.push_back(sd._stop_positions[i]);
    }
    sa = DCAST(ShaderAttrib, sa)->set_shader_input(
      InternalName::make("u_stop_colors"), colors);
    sa = DCAST(ShaderAttrib, sa)->set_shader_input(
      InternalName::make("u_stop_positions"), positions);
    state = state->add_attrib(sa);
    break;
  }
  case ShaderType::CREATION: {
    if (_shader_creation == nullptr) {
      PT(Shader) sh = Shader::make(Shader::SL_GLSL, s_vert_ui, s_frag_creation);
      // Reuse the same blend-over base state as _shader_gradient.
      _shader_creation = _shader_gradient->add_attrib(ShaderAttrib::make(sh));
    }
    state = _shader_creation;
    CPT(RenderAttrib) sa = state->get_attrib(ShaderAttrib::get_class_slot());
    float t = (float)Rml::GetSystemInterface()->GetElapsedTime();
    sa = DCAST(ShaderAttrib, sa)->set_shader_input(
      InternalName::make("u_time"), LVecBase4f(t, 0, 0, 0));
    sa = DCAST(ShaderAttrib, sa)->set_shader_input(
      InternalName::make("u_dimensions"), sd._dimensions);
    state = state->add_attrib(sa);
    break;
  }
  default: return;
  }

  render_geom(cg->_geom, state, translation);
}

/**
 * Frees a compiled shader.
 */
void RmlRenderInterface::
ReleaseShader(Rml::CompiledShaderHandle shader_handle) {
  delete reinterpret_cast<CompiledShaderData *>(shader_handle);
}
