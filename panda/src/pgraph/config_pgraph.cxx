// Filename: config_pgraph.cxx
// Created by:  drose (21Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "config_pgraph.h"

#include "alphaTestAttrib.h"
#include "ambientLight.h"
#include "billboardEffect.h"
#include "camera.h"
#include "colorAttrib.h"
#include "colorBlendAttrib.h"
#include "colorScaleAttrib.h"
#include "colorWriteAttrib.h"
#include "cullFaceAttrib.h"
#include "cullBin.h"
#include "cullBinAttrib.h"
#include "cullBinBackToFront.h"
#include "cullBinUnsorted.h"
#include "cullTraverser.h"
#include "cullableObject.h"
#include "decalEffect.h"
#include "depthOffsetAttrib.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "directionalLight.h"
#include "fog.h"
#include "fogAttrib.h"
#include "geomNode.h"
#include "lensNode.h"
#include "light.h"
#include "lightAttrib.h"
#include "lightLensNode.h"
#include "lightNode.h"
#include "lodNode.h"
#include "materialAttrib.h"
#include "modelNode.h"
#include "modelRoot.h"
#include "nodePath.h"
#include "nodePathComponent.h"
#include "pandaNode.h"
#include "pointLight.h"
#include "renderAttrib.h"
#include "renderEffect.h"
#include "renderEffects.h"
#include "renderModeAttrib.h"
#include "renderState.h"
#include "selectiveChildNode.h"
#include "sequenceNode.h"
#include "showBoundsEffect.h"
#include "spotlight.h"
#include "texMatrixAttrib.h"
#include "textureApplyAttrib.h"
#include "textureAttrib.h"
#include "transformState.h"
#include "transparencyAttrib.h"
#include "nodePathLerps.h"

#include "dconfig.h"

ConfigureDef(config_pgraph);
NotifyCategoryDef(pgraph, "");

ConfigureFn(config_pgraph) {
  init_libpgraph();
}

// Set this true to cause culling to be performed by rendering the
// object in red wireframe, rather than actually culling it.  This
// helps make culling errors obvious.
const bool fake_view_frustum_cull = config_pgraph.GetBool("fake-view-frustum-cull", false);

// Set this true to make ambiguous path warning messages generate an
// assertion failure instead of just a warning (which can then be
// trapped with assert-abort).
const bool unambiguous_graph = config_pgraph.GetBool("unambiguous-graph", false);

////////////////////////////////////////////////////////////////////
//     Function: init_libpgraph
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpgraph() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  AlphaTestAttrib::init_type();
  AmbientLight::init_type();
  BillboardEffect::init_type();
  Camera::init_type();
  ColorAttrib::init_type();
  ColorBlendAttrib::init_type();
  ColorScaleAttrib::init_type();
  ColorWriteAttrib::init_type();
  CullFaceAttrib::init_type();
  CullBin::init_type();
  CullBinAttrib::init_type();
  CullBinBackToFront::init_type();
  CullBinUnsorted::init_type();
  CullTraverser::init_type();
  CullableObject::init_type();
  DecalEffect::init_type();
  DepthOffsetAttrib::init_type();
  DepthTestAttrib::init_type();
  DepthWriteAttrib::init_type();
  DirectionalLight::init_type();
  Fog::init_type();
  FogAttrib::init_type();
  GeomNode::init_type();
  LensNode::init_type();
  Light::init_type();
  LightAttrib::init_type();
  LightLensNode::init_type();
  LightNode::init_type();
  LODNode::init_type();
  MaterialAttrib::init_type();
  ModelNode::init_type();
  ModelRoot::init_type();
  NodePath::init_type();
  NodePathComponent::init_type();
  PandaNode::init_type();
  PointLight::init_type();
  RenderAttrib::init_type();
  RenderEffect::init_type();
  RenderEffects::init_type();
  RenderModeAttrib::init_type();
  RenderState::init_type();
  SelectiveChildNode::init_type();
  SequenceNode::init_type();
  ShowBoundsEffect::init_type();
  Spotlight::init_type();
  TexMatrixAttrib::init_type();
  TextureApplyAttrib::init_type();
  TextureAttrib::init_type();
  TransformState::init_type();
  TransparencyAttrib::init_type();
  PosLerpFunctor::init_type();
  HprLerpFunctor::init_type();
  ScaleLerpFunctor::init_type();
  PosHprLerpFunctor::init_type();
  HprScaleLerpFunctor::init_type();
  PosHprScaleLerpFunctor::init_type();
  ColorLerpFunctor::init_type();
  ColorScaleLerpFunctor::init_type();

  AlphaTestAttrib::register_with_read_factory();
  AmbientLight::register_with_read_factory();
  BillboardEffect::register_with_read_factory();
  Camera::register_with_read_factory();
  ColorAttrib::register_with_read_factory();
  ColorBlendAttrib::register_with_read_factory();
  ColorScaleAttrib::register_with_read_factory();
  ColorWriteAttrib::register_with_read_factory();
  CullBinAttrib::register_with_read_factory();
  CullFaceAttrib::register_with_read_factory();
  DecalEffect::register_with_read_factory();
  DepthOffsetAttrib::register_with_read_factory();
  DepthTestAttrib::register_with_read_factory();
  DepthWriteAttrib::register_with_read_factory();
  DirectionalLight::register_with_read_factory();
  Fog::register_with_read_factory();
  FogAttrib::register_with_read_factory();
  GeomNode::register_with_read_factory();
  LensNode::register_with_read_factory();
  LightAttrib::register_with_read_factory();
  LODNode::register_with_read_factory();
  MaterialAttrib::register_with_read_factory();  
  ModelNode::register_with_read_factory();
  ModelRoot::register_with_read_factory();
  PandaNode::register_with_read_factory();
  PointLight::register_with_read_factory();
  RenderEffects::register_with_read_factory();
  RenderModeAttrib::register_with_read_factory();
  RenderState::register_with_read_factory();
  SequenceNode::register_with_read_factory();
  ShowBoundsEffect::register_with_read_factory();
  Spotlight::register_with_read_factory();
  TexMatrixAttrib::register_with_read_factory();
  TextureApplyAttrib::register_with_read_factory();
  TextureAttrib::register_with_read_factory();
  TransformState::register_with_read_factory();
  TransparencyAttrib::register_with_read_factory();
}
