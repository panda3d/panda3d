// Filename: rocketRenderInterface.h
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

#ifndef ROCKET_RENDER_INTERFACE_H
#define ROCKET_RENDER_INTERFACE_H

#include "config_rocket.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "geom.h"
#include "renderState.h"
#include "transformState.h"

#include <Rocket/Core/RenderInterface.h>

////////////////////////////////////////////////////////////////////
//       Class : RocketRenderInterface
// Description : Class that provides the main render interface for
//               libRocket integration.
////////////////////////////////////////////////////////////////////
class RocketRenderInterface : public Rocket::Core::RenderInterface {
public:
  void render(Rocket::Core::Context* context, CullTraverser *trav);

protected:
  struct CompiledGeometry {
    CPT(Geom) _geom;
    CPT(RenderState) _state;
  };

  PT(Geom) make_geom(Rocket::Core::Vertex* vertices,
                     int num_vertices, int* indices, int num_indices,
                     GeomEnums::UsageHint uh, const LVecBase2 &tex_scale);
  void render_geom(const Geom* geom, const RenderState* state, const Rocket::Core::Vector2f& translation);

  void RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture, const Rocket::Core::Vector2f& translation);
  Rocket::Core::CompiledGeometryHandle CompileGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture);
  void RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation);
  void ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry);

  bool LoadTexture(Rocket::Core::TextureHandle& texture_handle,
                   Rocket::Core::Vector2i& texture_dimensions,
                   const Rocket::Core::String& source);
  bool GenerateTexture(Rocket::Core::TextureHandle& texture_handle,
                       const Rocket::Core::byte* source,
                       const Rocket::Core::Vector2i& source_dimensions);
  void ReleaseTexture(Rocket::Core::TextureHandle texture_handle);

  void EnableScissorRegion(bool enable);
  void SetScissorRegion(int x, int y, int width, int height);

private:
  Mutex _lock;

  // Hold the scissor settings and whether or not to enable scissoring.
  bool _enable_scissor;
  LVecBase4 _scissor;

  // These are temporarily filled in by render().
  CullTraverser *_trav;
  CPT(TransformState) _net_transform;
  CPT(RenderState) _net_state;
  Rocket::Core::Vector2i _dimensions;

};

#endif
