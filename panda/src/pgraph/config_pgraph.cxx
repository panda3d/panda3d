/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pgraph.cxx
 * @author drose
 * @date 2002-02-21
 */

#include "config_pgraph.h"

#include "alphaTestAttrib.h"
#include "audioVolumeAttrib.h"
#include "auxBitplaneAttrib.h"
#include "antialiasAttrib.h"
#include "auxSceneData.h"
#include "billboardEffect.h"
#include "camera.h"
#include "clipPlaneAttrib.h"
#include "colorAttrib.h"
#include "colorBlendAttrib.h"
#include "colorScaleAttrib.h"
#include "colorWriteAttrib.h"
#include "compassEffect.h"
#include "cullFaceAttrib.h"
#include "cullBin.h"
#include "cullBinAttrib.h"
#include "cullResult.h"
#include "cullTraverser.h"
#include "cullableObject.h"
#include "decalEffect.h"
#include "depthOffsetAttrib.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "findApproxLevelEntry.h"
#include "fog.h"
#include "fogAttrib.h"
#include "geomDrawCallbackData.h"
#include "geomNode.h"
#include "geomTransformer.h"
#include "lensNode.h"
#include "light.h"
#include "lightAttrib.h"
#include "lightRampAttrib.h"
#include "loader.h"
#include "loaderFileType.h"
#include "loaderFileTypeBam.h"
#include "loaderFileTypeRegistry.h"
#include "logicOpAttrib.h"
#include "materialAttrib.h"
#include "modelFlattenRequest.h"
#include "modelLoadRequest.h"
#include "modelSaveRequest.h"
#include "modelNode.h"
#include "modelRoot.h"
#include "nodePath.h"
#include "nodePathComponent.h"
#include "pandaNode.h"
#include "paramNodePath.h"
#include "planeNode.h"
#include "polylightEffect.h"
#include "polylightNode.h"
#include "portalNode.h"
#include "occluderEffect.h"
#include "occluderNode.h"
#include "portalClipper.h"
#include "renderAttrib.h"
#include "renderEffect.h"
#include "renderEffects.h"
#include "renderModeAttrib.h"
#include "renderState.h"
#include "rescaleNormalAttrib.h"
#include "sceneSetup.h"
#include "scissorAttrib.h"
#include "scissorEffect.h"
#include "shadeModelAttrib.h"
#include "shaderAttrib.h"
#include "shader.h"
#include "showBoundsEffect.h"
#include "stencilAttrib.h"
#include "stateMunger.h"
#include "texMatrixAttrib.h"
#include "texProjectorEffect.h"
#include "textureAttrib.h"
#include "texGenAttrib.h"
#include "transformState.h"
#include "transparencyAttrib.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_PGRAPH)
  #error Buildsystem error: BUILDING_PANDA_PGRAPH not defined
#endif

ConfigureDef(config_pgraph);
NotifyCategoryDef(pgraph, "");
NotifyCategoryDef(loader, "");
NotifyCategoryDef(portal, "");

ConfigureFn(config_pgraph) {
  init_libpgraph();
}

ConfigVariableBool fake_view_frustum_cull
("fake-view-frustum-cull", false,
 PRC_DESC("Set this true to cause culling to be performed by rendering the "
          "object in red wireframe, rather than actually culling it.  This "
          "helps make culling errors obvious.  This variable only has an "
          "effect when Panda is not compiled for a release build."));

ConfigVariableBool clip_plane_cull
("clip-plane-cull", true,
 PRC_DESC("This is normally true; set it false to disable culling of objects "
          "that are completely behind one or more clip planes (primarily "
          "useful for debugging)  This also disables the use of occluders."));

ConfigVariableBool allow_portal_cull
("allow-portal-cull", false,
 PRC_DESC("Set this true to enable portal clipping.  This will enable the "
          "renderer to cull more objects that are clipped if not in the "
          "current list of portals.  This is still somewhat experimental."));

ConfigVariableBool debug_portal_cull
("debug-portal-cull", false,
 PRC_DESC("Set this true to enable debug visualization during portal clipping."
          "(You first need to enable portal culling, using the allow-portal-cull"
          "variable.)"));

ConfigVariableBool show_occluder_volumes
("show-occluder-volumes", false,
 PRC_DESC("Set this true to enable debug visualization of the volumes used "
          "to cull objects behind an occluder."));

ConfigVariableBool unambiguous_graph
("unambiguous-graph", false,
 PRC_DESC("Set this true to make ambiguous path warning messages generate an "
          "assertion failure instead of just a warning (which can then be "
          "trapped with assert-abort)."));

ConfigVariableBool detect_graph_cycles
("detect-graph-cycles", true,
 PRC_DESC("Set this true to attempt to detect cycles in the scene graph "
          "(e.g. a node which is its own parent) as soon as they are "
          "made.  This has no effect in NDEBUG mode."));

ConfigVariableBool no_unsupported_copy
("no-unsupported-copy", false,
 PRC_DESC("Set this true to make an attempt to copy an unsupported type "
          "generate an assertion failure instead of just a warning (which "
          "can then be trapped with assert-abort)."));

ConfigVariableBool allow_unrelated_wrt
("allow-unrelated-wrt", true,
 PRC_DESC("Set this true to allow unrelated NodePaths (that is, nodes which "
          "have no common ancestor) to be adjusted relative to each other.  If "
          "true, these will be treated as if they had a common node above "
          "their top nodes."));

ConfigVariableBool paranoid_compose
("paranoid-compose", false,
 PRC_DESC("Set this true to double-check the componentwise transform compose "
          "(or invert) operation against the equivalent matrix-based "
          "operation.  This has no effect if NDEBUG is defined."));

ConfigVariableBool compose_componentwise
("compose-componentwise", true,
 PRC_DESC("Set this true to perform componentwise compose and invert "
          "operations when possible.  If this is false, the compositions "
          "are always computed by matrix."));

ConfigVariableBool paranoid_const
("paranoid-const", false,
 PRC_DESC("Set this true to double-check that nothing is inappropriately "
          "modifying the supposedly const structures like RenderState, "
          "RenderAttrib, TransformState, and RenderEffect.  This has no effect "
          "if NDEBUG is defined."));

ConfigVariableBool auto_break_cycles
("auto-break-cycles", true,
 PRC_DESC("Set this true to automatically detect and break reference-count "
          "cycles in the TransformState and RenderState caches.  When this "
          "is false, you must explicitly call TransformState.clear_cache() "
          "from time to time to prevent gradual memory bloat."));

ConfigVariableBool garbage_collect_states
("garbage-collect-states", true,
 PRC_DESC("Set this true to defer destruction of TransformState and "
          "RenderState objects until the end of the frame (or whenever "
          "TransformState::garbage_collect() and RenderState::garbage_collect() "
          "are called).  This is a particularly useful thing to do when "
          "using multiple threads, because it improves parallelization."));

ConfigVariableDouble garbage_collect_states_rate
("garbage-collect-states-rate", 1.0,
 PRC_DESC("The fraction of the total number of TransformStates "
          "(or RenderStates, or whatever) that are processed with "
          "each garbage collection step.  Setting this smaller than "
          "1.0 will collect fewer states each frame, which may require "
          "less processing time, but risks getting unstable cache "
          "performance if states accumulate faster than they can be "
          "cleaned up."));

ConfigVariableBool transform_cache
("transform-cache", true,
 PRC_DESC("Set this true to enable the cache of TransformState objects.  "
          "Using the cache saves time computing transforms and inverse "
          "transforms, but imposes some overhead for maintaining the "
          "cache itself."));

ConfigVariableBool state_cache
("state-cache", true,
 PRC_DESC("Set this true to enable the cache of RenderState objects, "
          "similar to the TransformState cache controlled via "
          "transform-cache."));

ConfigVariableBool uniquify_transforms
("uniquify-transforms", true,
 PRC_DESC("Set this true to ensure that equivalent TransformStates "
          "are pointerwise equal.  This may improve caching performance, "
          "but also adds additional overhead to maintain the cache, "
          "including the need to check for a composition cycle in "
          "the cache."));

ConfigVariableBool uniquify_states
("uniquify-states", true,
 PRC_DESC("Set this true to ensure that equivalent RenderStates "
          "are pointerwise equal.  This may improve caching performance, "
          "but also adds additional overhead to maintain the cache, "
          "including the need to check for a composition cycle in "
          "the cache.  It is highly recommended to keep this on."));

ConfigVariableBool uniquify_attribs
("uniquify-attribs", true,
 PRC_DESC("Set this true to ensure that equivalent RenderAttribs "
          "are pointerwise equal.  This may improve caching performance, "
          "but also adds additional overhead to maintain the cache, "
          "including the need to check for a composition cycle in "
          "the cache."));

ConfigVariableBool retransform_sprites
("retransform-sprites", true,
 PRC_DESC("To render sprite-based particle effects, Panda must convert "
          "the sprite points from object space into clip space, and compute "
          "the corners of the quads in clip space.  When this variable is "
          "false, the resulting quads are then sent to the graphics "
          "hardware in clip space.  When this is true, the quads are "
          "re-transformed back into the original object space, which is "
          "necessary in order for fog to work correctly on the sprites."));

ConfigVariableBool depth_offset_decals
("depth-offset-decals", true,
 PRC_DESC("Set this true to allow decals to be implemented via the advanced "
          "depth offset feature, if supported, instead of via the traditional "
          "(and slower) two-pass approach.  This is currently the only method "
          "by which decals are implemented in Panda3D, and as such, this "
          "setting is ignored."));

ConfigVariableInt max_collect_vertices
("max-collect-vertices", 65534,
 PRC_DESC("Specifies the maximum number of vertices that are allowed to be "
          "accumulated into any one GeomVertexData structure as a result "
          "of collecting objects together during a flatten operation.  "
          "This prevents the accidental generation of large vertex buffers "
          "from lots of smaller vertex buffers, while not "
          "imposing a limit on the original size of any one "
          "GeomVertexData structure."));

ConfigVariableInt max_collect_indices
("max-collect-indices", 65535,
 PRC_DESC("Specifies the maximum number of vertex indices that are allowed "
          "to be accumulated into any one GeomPrimitive as a result "
          "of collecting objects together during a flatten operation.  "
          "This prevents the accidental generation of large index buffers "
          "from lots of smaller index buffers, while not "
          "imposing a limit on the original size of any one "
          "GeomPrimitive."));

ConfigVariableBool premunge_data
("premunge-data", true,
 PRC_DESC("Set this true to preconvert vertex data at model load time to "
          "match the data requirements of the current GSG.  For instance, "
          "color columns are pre-converted to match OpenGL or DirectX "
          "encoding requirements, as appropriate.  When this is false, the "
          "data will be munged at render time instead."));

ConfigVariableBool preserve_geom_nodes
("preserve-geom-nodes", false,
 PRC_DESC("This specifies the default value for the \"preserved\" flag on "
          "every GeomNode created.  When this is true, GeomNodes will not "
          "be flattened, so setting this true effectively disables the "
          "use of flatten to combine GeomNodes."));

ConfigVariableBool flatten_geoms
("flatten-geoms", true,
 PRC_DESC("When this is true (the default), NodePath::flatten_strong() and "
          "flatten_medium() will attempt to combine multiple Geoms into "
          "as few Geoms as possible, by combing GeomVertexDatas and then "
          "unifying.  Setting this false disables this behavior, so that "
          "NodePath flatten operations will only reduce nodes.  This affects "
          "only the NodePath interfaces; you may still make the lower-level "
          "SceneGraphReducer calls directly."));

ConfigVariableInt max_lenses
("max-lenses", 100,
 PRC_DESC("Specifies an upper limit on the maximum number of lenses "
          "and the maximum lens index number) that may be associated with "
          "a single LensNode.  There is no technical reason for this "
          "limitation, but very large numbers are probably a mistake, so "
          "this can be used as a simple sanity check.  Set it larger or "
          "smaller to suit your needs."));

ConfigVariableBool polylight_info
("polylight-info", false,
 PRC_DESC("Set this true to view some info statements regarding the polylight. "
          "It is helpful for debugging."));

ConfigVariableBool show_vertex_animation
("show-vertex-animation", false,
 PRC_DESC("Set this true to flash any objects whose vertices are animated "
          "by Panda on the CPU (flash red) or by hardware (flash blue).  "
          "This only has effect when NDEBUG is not defined."));

ConfigVariableBool show_transparency
("show-transparency", false,
 PRC_DESC("Set this true to flash any objects that are rendered in "
          "some transparency mode.  The color chosen is based on the  "
          "particular transparency mode in effect.  This only has effect "
          "when NDEBUG is not defined."));

ConfigVariableBool m_dual
("m-dual", true,
 PRC_DESC("Set this false to disable TransparencyAttrib::M_dual altogether "
          "(and use M_alpha in its place)."));

ConfigVariableBool m_dual_opaque
("m-dual-opaque", true,
 PRC_DESC("Set this false to disable just the opaque part of M_dual."));

ConfigVariableBool m_dual_transparent
("m-dual-transparent", true,
 PRC_DESC("Set this false to disable just the transparent part of M_dual."));

ConfigVariableBool m_dual_flash
("m-dual-flash", false,
 PRC_DESC("Set this true to flash any objects that use M_dual, for debugging."));

ConfigVariableList load_file_type
("load-file-type",
 PRC_DESC("List the model loader modules that Panda will automatically "
          "import when a new, unknown model type is loaded.  This may be "
          "either the name of a module, or a space-separate list of filename "
          "extensions, followed by the name of the module."));

ConfigVariableString default_model_extension
("default-model-extension", "",
 PRC_DESC("This specifies the filename extension (with leading dot) that "
          "should be assumed if an attempt is made to load a filename that "
          "has no extension.  This is primarily designed to support legacy "
          "code that used the now-deprecated implicit-extension feature of "
          "Panda's loader; new code should probably give the correct name "
          "for each model file they intend to load."));

ConfigVariableBool allow_live_flatten
("allow-live-flatten", true,
 PRC_DESC("Set this true to allow the use of flatten_strong() or any "
          "variant on a node that is attached to a live scene graph node, "
          "or false to disallow this.  Flattening a live scene graph node "
          "can cause problems when threading is enabled.  This variable "
          "only has an effect when Panda is not compiled for a release "
          "build."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpgraph() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  AlphaTestAttrib::init_type();
  AntialiasAttrib::init_type();
  AudioVolumeAttrib::init_type();
  AuxBitplaneAttrib::init_type();
  AuxSceneData::init_type();
  BillboardEffect::init_type();
  Camera::init_type();
  ClipPlaneAttrib::init_type();
  ColorAttrib::init_type();
  ColorBlendAttrib::init_type();
  ColorScaleAttrib::init_type();
  ColorWriteAttrib::init_type();
  CompassEffect::init_type();
  CullFaceAttrib::init_type();
  CullBin::init_type();
  CullBinAttrib::init_type();
  CullResult::init_type();
  CullTraverser::init_type();
  CullableObject::init_type();
  DecalEffect::init_type();
  DepthOffsetAttrib::init_type();
  DepthTestAttrib::init_type();
  DepthWriteAttrib::init_type();
  FindApproxLevelEntry::init_type();
  Fog::init_type();
  FogAttrib::init_type();
  GeomDrawCallbackData::init_type();
  GeomNode::init_type();
  GeomTransformer::init_type();
  LensNode::init_type();
  Light::init_type();
  LightAttrib::init_type();
  LightRampAttrib::init_type();
  Loader::init_type();
  LoaderFileType::init_type();
  LoaderFileTypeBam::init_type();
  LogicOpAttrib::init_type();
  MaterialAttrib::init_type();
  ModelFlattenRequest::init_type();
  ModelLoadRequest::init_type();
  ModelSaveRequest::init_type();
  ModelNode::init_type();
  ModelRoot::init_type();
  NodePath::init_type();
  NodePathComponent::init_type();
  PandaNode::init_type();
  PandaNodePipelineReader::init_type();
  ParamNodePath::init_type();
  PlaneNode::init_type();
  PolylightNode::init_type();
  PolylightEffect::init_type();
  PortalNode::init_type();
  OccluderEffect::init_type();
  OccluderNode::init_type();
  PortalClipper::init_type();
  RenderAttrib::init_type();
  RenderEffect::init_type();
  RenderEffects::init_type();
  RenderModeAttrib::init_type();
  RenderState::init_type();
  RescaleNormalAttrib::init_type();
  SceneSetup::init_type();
  ScissorAttrib::init_type();
  ScissorEffect::init_type();
  ShadeModelAttrib::init_type();
  ShaderAttrib::init_type();
  ShowBoundsEffect::init_type();
  StateMunger::init_type();
  StencilAttrib::init_type();
  TexMatrixAttrib::init_type();
  TexProjectorEffect::init_type();
  TextureAttrib::init_type();
  TexGenAttrib::init_type();
  TransformState::init_type();
  TransparencyAttrib::init_type();

  AlphaTestAttrib::register_with_read_factory();
  AntialiasAttrib::register_with_read_factory();
  AudioVolumeAttrib::register_with_read_factory();
  AuxBitplaneAttrib::register_with_read_factory();
  BillboardEffect::register_with_read_factory();
  Camera::register_with_read_factory();
  ClipPlaneAttrib::register_with_read_factory();
  CompassEffect::register_with_read_factory();
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
  Fog::register_with_read_factory();
  FogAttrib::register_with_read_factory();
  GeomNode::register_with_read_factory();
  LensNode::register_with_read_factory();
  LightAttrib::register_with_read_factory();
  LightRampAttrib::register_with_read_factory();
  LogicOpAttrib::register_with_read_factory();
  MaterialAttrib::register_with_read_factory();
  ModelNode::register_with_read_factory();
  ModelRoot::register_with_read_factory();
  PandaNode::register_with_read_factory();
  ParamNodePath::register_with_read_factory();
  PlaneNode::register_with_read_factory();
  PolylightNode::register_with_read_factory();
  PortalNode::register_with_read_factory();
  OccluderEffect::register_with_read_factory();
  OccluderNode::register_with_read_factory();
  RenderEffects::register_with_read_factory();
  RenderModeAttrib::register_with_read_factory();
  RenderState::register_with_read_factory();
  RescaleNormalAttrib::register_with_read_factory();
  ScissorAttrib::register_with_read_factory();
  ScissorEffect::register_with_read_factory();
  ShadeModelAttrib::register_with_read_factory();
  ShaderAttrib::register_with_read_factory();
  ShowBoundsEffect::register_with_read_factory();
  TexMatrixAttrib::register_with_read_factory();
  TexProjectorEffect::register_with_read_factory();
  TextureAttrib::register_with_read_factory();
  TexGenAttrib::register_with_read_factory();
  TransformState::register_with_read_factory();
  TransparencyAttrib::register_with_read_factory();

  // By initializing the _states map up front, we also guarantee that the
  // _states_lock mutex gets created before we spawn any threads (assuming no
  // one is creating threads at static init time).
  TransformState::init_states();
  RenderState::init_states();
  RenderEffects::init_states();

  RenderAttrib::init_attribs();

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_global_ptr();
  reg->register_type(new LoaderFileTypeBam);
}
