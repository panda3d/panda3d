// Filename: rocketRenderInterface.cxx
// Created by:  rdb (04Nov11)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "rocketRenderInterface.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "geomVertexData.h"
#include "geomVertexArrayData.h"
#include "internalName.h"
#include "geomVertexWriter.h"
#include "geomTriangles.h"
#include "colorBlendAttrib.h"
#include "cullBinAttrib.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "scissorAttrib.h"
#include "texture.h"
#include "textureAttrib.h"
#include "texturePool.h"

////////////////////////////////////////////////////////////////////
//     Function: RocketRenderInterface::render
//       Access: Public
//  Description: Called by RocketNode in cull_callback.  Invokes
//               context->Render() and culls the result.
////////////////////////////////////////////////////////////////////
void RocketRenderInterface::
render(Rocket::Core::Context* context, CullTraverser *trav) {
  nassertv(context != NULL);
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
    )
  );
  _dimensions = context->GetDimensions();

  context->Render();

  _trav = NULL;
  _net_transform = NULL;
  _net_state = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: RocketRenderInterface::make_geom
//       Access: Protected
//  Description: Called internally to make a Geom from Rocket data.
////////////////////////////////////////////////////////////////////
PT(Geom) RocketRenderInterface::
make_geom(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, GeomEnums::UsageHint uh) {
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
      twriter.add_data2f(vertex.tex_coord.x, 1.0f - vertex.tex_coord.y);
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

////////////////////////////////////////////////////////////////////
//     Function: RocketRenderInterface::render_geom
//       Access: Protected
//  Description: Only call this during render().  Culls a geom.
////////////////////////////////////////////////////////////////////
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

  CPT(TransformState) net_transform, modelview_transform;
  net_transform = _net_transform->compose(TransformState::make_pos(offset));
  modelview_transform = _trav->get_world_transform()->compose(net_transform);

  CullableObject *object =
    new CullableObject(geom, _net_state->compose(state),
                       net_transform, modelview_transform,
                       _trav->get_scene());
  _trav->get_cull_handler()->record_object(object, _trav);
}

////////////////////////////////////////////////////////////////////
//     Function: RocketRenderInterface::RenderGeometry
//       Access: Protected
//  Description: Called by Rocket when it wants to render geometry
//               that the application does not wish to optimize.
////////////////////////////////////////////////////////////////////
void RocketRenderInterface::
RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture, const Rocket::Core::Vector2f& translation) {
  PT(Geom) geom = make_geom(vertices, num_vertices, indices, num_indices, GeomEnums::UH_stream);

  CPT(RenderState) state;
  if ((Texture*) texture != (Texture*) NULL) {
    state = RenderState::make(TextureAttrib::make((Texture*) texture));
  } else {
    state = RenderState::make_empty();
  }

  render_geom(geom, state, translation);
}

////////////////////////////////////////////////////////////////////
//     Function: RocketRenderInterface::CompileGeometry
//       Access: Protected
//  Description: Called by Rocket when it wants to compile geometry
//               it believes will be static for the forseeable future.
////////////////////////////////////////////////////////////////////
Rocket::Core::CompiledGeometryHandle RocketRenderInterface::
CompileGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture) {

  CompiledGeometry *c = new CompiledGeometry;
  c->_geom = make_geom(vertices, num_vertices, indices, num_indices, GeomEnums::UH_static);

  if ((Texture*) texture != (Texture*) NULL) {
    PT(TextureStage) stage = new TextureStage("");
    stage->set_mode(TextureStage::M_modulate);

    CPT(TextureAttrib) attr = DCAST(TextureAttrib, TextureAttrib::make());
    attr = DCAST(TextureAttrib, attr->add_on_stage(stage, (Texture*) texture));

    c->_state = RenderState::make(attr);

    rocket_cat.debug()
      << "Compiled geom " << c->_geom << " with texture '"
      << ((Texture*) texture)->get_name() << "'\n";
  } else {
    c->_state = RenderState::make_empty();

    rocket_cat.debug()
      << "Compiled geom " << c->_geom << " without texture\n";
  }

  return (Rocket::Core::CompiledGeometryHandle) c;
}

////////////////////////////////////////////////////////////////////
//     Function: RocketRenderInterface::RenderCompiledGeometry
//       Access: Protected
//  Description: Called by Rocket when it wants to render
//               application-compiled geometry.
////////////////////////////////////////////////////////////////////
void RocketRenderInterface::
RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation) {

  CompiledGeometry *c = (CompiledGeometry*) geometry;
  render_geom(c->_geom, c->_state, translation);
}

////////////////////////////////////////////////////////////////////
//     Function: RocketRenderInterface::ReleaseCompiledGeometry
//       Access: Protected
//  Description: Called by Rocket when it wants to release
//               application-compiled geometry.
////////////////////////////////////////////////////////////////////
void RocketRenderInterface::
ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry) {
  delete (CompiledGeometry*) geometry;
}

////////////////////////////////////////////////////////////////////
//     Function: RocketRenderInterface::LoadTexture
//       Access: Protected
//  Description: Called by Rocket when a texture is required by the
//               library.
////////////////////////////////////////////////////////////////////
bool RocketRenderInterface::
LoadTexture(Rocket::Core::TextureHandle& texture_handle,
            Rocket::Core::Vector2i& texture_dimensions,
            const Rocket::Core::String& source) {

  PT(Texture) tex = TexturePool::load_texture(Filename::from_os_specific(source.CString()));
  if (tex == NULL) {
    texture_handle = 0;
    texture_dimensions.x = 0;
    texture_dimensions.y = 0;
    return false;
  }

  tex->set_minfilter(Texture::FT_nearest);
  tex->set_magfilter(Texture::FT_nearest);

  texture_dimensions.x = tex->get_x_size();
  texture_dimensions.y = tex->get_y_size();
  tex->ref();
  texture_handle = (Rocket::Core::TextureHandle) tex.p();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RocketRenderInterface::GenerateTexture
//       Access: Protected
//  Description: Called by Rocket when a texture is required to be
//               built from an internally-generated sequence of pixels.
////////////////////////////////////////////////////////////////////
bool RocketRenderInterface::
GenerateTexture(Rocket::Core::TextureHandle& texture_handle,
                const Rocket::Core::byte* source,
                const Rocket::Core::Vector2i& source_dimensions) {

  PT(Texture) tex = new Texture;
  tex->setup_2d_texture(source_dimensions.x, source_dimensions.y,
                        Texture::T_unsigned_byte, Texture::F_rgba);
  PTA_uchar image = tex->modify_ram_image();

  // Convert RGBA to BGRA
  size_t row_size = source_dimensions.x * 4;
  size_t y2 = image.size();
  for (size_t y = 0; y < image.size(); y += row_size) {
    y2 -= row_size;
    for (size_t i = 0; i < row_size; i += 4) {
      image[y2 + i + 0] = source[y + i + 2];
      image[y2 + i + 1] = source[y + i + 1];
      image[y2 + i + 2] = source[y + i];
      image[y2 + i + 3] = source[y + i + 3];
    }
  }

  tex->set_wrap_u(Texture::WM_clamp);
  tex->set_wrap_v(Texture::WM_clamp);
  tex->set_minfilter(Texture::FT_nearest);
  tex->set_magfilter(Texture::FT_nearest);

  tex->ref();
  texture_handle = (Rocket::Core::TextureHandle) tex.p();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RocketRenderInterface::ReleaseTexture
//       Access: Protected
//  Description: Called by Rocket when a loaded texture is no longer
//               required.
////////////////////////////////////////////////////////////////////
void RocketRenderInterface::
ReleaseTexture(Rocket::Core::TextureHandle texture_handle) {
  Texture* tex = (Texture*) texture_handle;
  if (tex != (Texture*) NULL) {
    tex->unref();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RocketRenderInterface::EnableScissorRegion
//       Access: Protected
//  Description: Called by Rocket when it wants to enable or disable
//               scissoring to clip content.
////////////////////////////////////////////////////////////////////
void RocketRenderInterface::
EnableScissorRegion(bool enable) {
  _enable_scissor = enable;
}

////////////////////////////////////////////////////////////////////
//     Function: RocketRenderInterface::SetScissorRegion
//       Access: Protected
//  Description: Called by Rocket when it wants to change the
//               scissor region.
////////////////////////////////////////////////////////////////////
void RocketRenderInterface::
SetScissorRegion(int x, int y, int width, int height) {
  _scissor[0] = x / (PN_stdfloat) _dimensions.x;
  _scissor[1] = (x + width) / (PN_stdfloat) _dimensions.x;
  _scissor[2] = 1.0f - ((y + height) / (PN_stdfloat) _dimensions.y);
  _scissor[3] = 1.0f - (y / (PN_stdfloat) _dimensions.y);
}
