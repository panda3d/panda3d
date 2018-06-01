/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rocketRenderInterface.cxx
 * @author rdb
 * @date 2011-11-04
 */

#include "rocketRenderInterface.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "geomVertexData.h"
#include "geomVertexArrayData.h"
#include "internalName.h"
#include "geomVertexWriter.h"
#include "geomTriangles.h"
#include "colorAttrib.h"
#include "colorBlendAttrib.h"
#include "cullBinAttrib.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "scissorAttrib.h"
#include "texture.h"
#include "textureAttrib.h"
#include "texturePool.h"

/**
 * Called by RocketNode in cull_callback.  Invokes context->Render() and culls
 * the result.
 */
void RocketRenderInterface::
render(Rocket::Core::Context* context, CullTraverser *trav) {
  nassertv(context != nullptr);
  MutexHolder holder(_lock);

  _trav = trav;
  _net_transform = trav->get_world_transform();
  _net_state = RenderState::make(
    CullBinAttrib::make("unsorted", 0),
    DepthTestAttrib::make(RenderAttrib::M_none),
    DepthWriteAttrib::make(DepthWriteAttrib::M_off),
    ColorBlendAttrib::make(ColorBlendAttrib::M_add,
      ColorBlendAttrib::O_incoming_alpha,
      ColorBlendAttrib::O_one_minus_incoming_alpha
    ),
    ColorAttrib::make_vertex()
  );
  _dimensions = context->GetDimensions();

  context->Render();

  _trav = nullptr;
  _net_transform = nullptr;
  _net_state = nullptr;
}

/**
 * Called internally to make a Geom from Rocket data.
 */
PT(Geom) RocketRenderInterface::
make_geom(Rocket::Core::Vertex* vertices,
          int num_vertices, int* indices, int num_indices,
          GeomEnums::UsageHint uh, const LVecBase2 &tex_scale) {

  PT(GeomVertexData) vdata = new GeomVertexData("", GeomVertexFormat::get_v3c4t2(), uh);
  vdata->unclean_set_num_rows(num_vertices);
  {
    GeomVertexWriter vwriter(vdata, InternalName::get_vertex());
    GeomVertexWriter cwriter(vdata, InternalName::get_color());
    GeomVertexWriter twriter(vdata, InternalName::get_texcoord());

    // Write the vertex information.
    for (int i = 0; i < num_vertices; ++i) {
      const Rocket::Core::Vertex &vertex = vertices[i];

      vwriter.add_data3f(LVector3f::right() * vertex.position.x + LVector3f::up() * vertex.position.y);
      cwriter.add_data4i(vertex.colour.red, vertex.colour.green,
                         vertex.colour.blue, vertex.colour.alpha);
      twriter.add_data2f(vertex.tex_coord.x * tex_scale[0],
                         (1.0f - vertex.tex_coord.y) * tex_scale[1]);
    }
  }

  // Create a primitive and write the indices.
  PT(GeomTriangles) triangles = new GeomTriangles(uh);
  {
    PT(GeomVertexArrayData) idata = triangles->modify_vertices();
    idata->unclean_set_num_rows(num_indices);
    GeomVertexWriter iwriter(idata, 0);

    for (int i = 0; i < num_indices; ++i) {
      iwriter.add_data1i(indices[i]);
    }
  }

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(triangles);
  return geom;
}

/**
 * Only call this during render().  Culls a geom.
 */
void RocketRenderInterface::
render_geom(const Geom* geom, const RenderState* state, const Rocket::Core::Vector2f& translation) {
  LVector3 offset = LVector3::right() * translation.x + LVector3::up() * translation.y;

  if (_enable_scissor) {
    state = state->add_attrib(ScissorAttrib::make(_scissor));
    rocket_cat.spam()
      << "Rendering geom " << geom << " with state "
      << *state << ", translation (" << offset << "), "
      << "scissor region (" << _scissor << ")\n";
  } else {
    rocket_cat.spam()
      << "Rendering geom " << geom << " with state "
      << *state << ", translation (" << offset << ")\n";
  }

  CPT(TransformState) internal_transform =
    _trav->get_scene()->get_cs_world_transform()->compose(
      _net_transform->compose(TransformState::make_pos(offset)));

  CullableObject *object =
    new CullableObject(geom, _net_state->compose(state),
                       internal_transform);
  _trav->get_cull_handler()->record_object(object, _trav);
}

/**
 * Called by Rocket when it wants to render geometry that the application does
 * not wish to optimize.
 */
void RocketRenderInterface::
RenderGeometry(Rocket::Core::Vertex* vertices,
               int num_vertices, int* indices, int num_indices,
               Rocket::Core::TextureHandle thandle,
               const Rocket::Core::Vector2f& translation) {

  Texture *texture = (Texture *)thandle;

  LVecBase2 tex_scale(1, 1);
  if (texture != nullptr) {
    tex_scale = texture->get_tex_scale();
  }

  PT(Geom) geom = make_geom(vertices, num_vertices, indices, num_indices,
                            GeomEnums::UH_stream, tex_scale);

  CPT(RenderState) state;
  if (texture != nullptr) {
    state = RenderState::make(TextureAttrib::make(texture));
  } else {
    state = RenderState::make_empty();
  }

  render_geom(geom, state, translation);
}

/**
 * Called by Rocket when it wants to compile geometry it believes will be
 * static for the forseeable future.
 */
Rocket::Core::CompiledGeometryHandle RocketRenderInterface::
CompileGeometry(Rocket::Core::Vertex* vertices,
                int num_vertices, int* indices, int num_indices,
                Rocket::Core::TextureHandle thandle) {

  Texture *texture = (Texture *)thandle;

  CompiledGeometry *c = new CompiledGeometry;
  LVecBase2 tex_scale(1, 1);

  if (texture != nullptr) {
    rocket_cat.debug()
      << "Compiling geom " << c->_geom << " with texture '"
      << texture->get_name() << "'\n";

    tex_scale = texture->get_tex_scale();

    PT(TextureStage) stage = new TextureStage("");
    stage->set_mode(TextureStage::M_modulate);

    CPT(TextureAttrib) attr = DCAST(TextureAttrib, TextureAttrib::make());
    attr = DCAST(TextureAttrib, attr->add_on_stage(stage, (Texture *)texture));

    c->_state = RenderState::make(attr);

  } else {
    rocket_cat.debug()
      << "Compiling geom " << c->_geom << " without texture\n";

    c->_state = RenderState::make_empty();
  }

  c->_geom = make_geom(vertices, num_vertices, indices, num_indices,
                       GeomEnums::UH_static, tex_scale);

  return (Rocket::Core::CompiledGeometryHandle) c;
}

/**
 * Called by Rocket when it wants to render application-compiled geometry.
 */
void RocketRenderInterface::
RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation) {

  CompiledGeometry *c = (CompiledGeometry*) geometry;
  render_geom(c->_geom, c->_state, translation);
}

/**
 * Called by Rocket when it wants to release application-compiled geometry.
 */
void RocketRenderInterface::
ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry) {
  delete (CompiledGeometry*) geometry;
}

/**
 * Called by Rocket when a texture is required by the library.
 */
bool RocketRenderInterface::
LoadTexture(Rocket::Core::TextureHandle& texture_handle,
            Rocket::Core::Vector2i& texture_dimensions,
            const Rocket::Core::String& source) {

  // Prefer padding over scaling to avoid blurring people's pixel art.
  LoaderOptions options;
  if (Texture::get_textures_power_2() == ATS_none) {
    options.set_auto_texture_scale(ATS_none);
  } else {
    options.set_auto_texture_scale(ATS_pad);
  }

  Filename fn = Filename::from_os_specific(source.CString());
  PT(Texture) tex = TexturePool::load_texture(fn, 0, false, options);
  if (tex == nullptr) {
    texture_handle = 0;
    texture_dimensions.x = 0;
    texture_dimensions.y = 0;
    return false;
  }

  tex->set_minfilter(SamplerState::FT_nearest);
  tex->set_magfilter(SamplerState::FT_nearest);

  // Since libRocket may make layout decisions based on the size of the image,
  // it's important that we give it the original size of the image file in
  // order to produce consistent results.
  int width = tex->get_orig_file_x_size();
  int height = tex->get_orig_file_y_size();
  if (width == 0 && height == 0) {
    // This shouldn't happen unless someone is playing very strange tricks
    // with the TexturePool, but we might as well handle it.
    width = tex->get_x_size();
    height = tex->get_y_size();
  }
  texture_dimensions.x = width;
  texture_dimensions.y = height;

  tex->ref();
  texture_handle = (Rocket::Core::TextureHandle) tex.p();

  return true;
}

/**
 * Called by Rocket when a texture is required to be built from an internally-
 * generated sequence of pixels.
 */
bool RocketRenderInterface::
GenerateTexture(Rocket::Core::TextureHandle& texture_handle,
                const Rocket::Core::byte* source,
                const Rocket::Core::Vector2i& source_dimensions) {

  PT(Texture) tex = new Texture;
  tex->setup_2d_texture(source_dimensions.x, source_dimensions.y,
                        Texture::T_unsigned_byte, Texture::F_rgba);

  // Pad to nearest power of two if necessary.  It may not be necessary as
  // libRocket seems to give power-of-two sizes already, but can't hurt.
  tex->set_size_padded(source_dimensions.x, source_dimensions.y);

  PTA_uchar image = tex->modify_ram_image();

  // Convert RGBA to BGRA
  size_t src_stride = source_dimensions.x * 4;
  size_t dst_stride = tex->get_x_size() * 4;
  const unsigned char *src_ptr = source + (src_stride * source_dimensions.y);
  unsigned char *dst_ptr = &image[0];

  for (; src_ptr > source; dst_ptr += dst_stride) {
    src_ptr -= src_stride;
    for (size_t i = 0; i < src_stride; i += 4) {
      dst_ptr[i + 0] = src_ptr[i + 2];
      dst_ptr[i + 1] = src_ptr[i + 1];
      dst_ptr[i + 2] = src_ptr[i];
      dst_ptr[i + 3] = src_ptr[i + 3];
    }
  }

  tex->set_wrap_u(SamplerState::WM_clamp);
  tex->set_wrap_v(SamplerState::WM_clamp);
  tex->set_minfilter(SamplerState::FT_nearest);
  tex->set_magfilter(SamplerState::FT_nearest);

  tex->ref();
  texture_handle = (Rocket::Core::TextureHandle) tex.p();

  return true;
}

/**
 * Called by Rocket when a loaded texture is no longer required.
 */
void RocketRenderInterface::
ReleaseTexture(Rocket::Core::TextureHandle texture_handle) {
  Texture *tex = (Texture *)texture_handle;
  if (tex != nullptr) {
    unref_delete(tex);
  }
}

/**
 * Called by Rocket when it wants to enable or disable scissoring to clip
 * content.
 */
void RocketRenderInterface::
EnableScissorRegion(bool enable) {
  _enable_scissor = enable;
}

/**
 * Called by Rocket when it wants to change the scissor region.
 */
void RocketRenderInterface::
SetScissorRegion(int x, int y, int width, int height) {
  _scissor[0] = x / (PN_stdfloat) _dimensions.x;
  _scissor[1] = (x + width) / (PN_stdfloat) _dimensions.x;
  _scissor[2] = 1.0f - ((y + height) / (PN_stdfloat) _dimensions.y);
  _scissor[3] = 1.0f - (y / (PN_stdfloat) _dimensions.y);
}
