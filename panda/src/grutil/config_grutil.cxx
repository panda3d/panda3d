/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_grutil.cxx
 * @author drose
 * @date 2000-05-24
 */

#include "config_grutil.h"
#include "frameRateMeter.h"
#include "sceneGraphAnalyzerMeter.h"
#include "meshDrawer.h"
#include "meshDrawer2D.h"
#include "geoMipTerrain.h"
#include "movieTexture.h"
#include "pandaSystem.h"
#include "texturePool.h"
#include "nodeVertexTransform.h"
#include "rigidBodyCombiner.h"
#include "pipeOcclusionCullTraverser.h"
#include "shaderTerrainMesh.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_GRUTIL)
  #error Buildsystem error: BUILDING_PANDA_GRUTIL not defined
#endif

Configure(config_grutil);
NotifyCategoryDef(grutil, "");

ConfigureFn(config_grutil) {
  init_libgrutil();
}

ConfigVariableBool frame_rate_meter_milliseconds
("frame-rate-meter-milliseconds", false);

ConfigVariableDouble frame_rate_meter_update_interval
("frame-rate-meter-update-interval", 1.5);

ConfigVariableString frame_rate_meter_text_pattern
("frame-rate-meter-text-pattern", "%0.1f fps");

ConfigVariableString frame_rate_meter_ms_text_pattern
("frame-rate-meter-ms-text-pattern", "%0.1f ms");

ConfigVariableInt frame_rate_meter_layer_sort
("frame-rate-meter-layer-sort", 1000);

ConfigVariableDouble frame_rate_meter_scale
("frame-rate-meter-scale", 0.05);

ConfigVariableDouble frame_rate_meter_side_margins
("frame-rate-meter-side-margins", 0.5);

ConfigVariableDouble scene_graph_analyzer_meter_update_interval
("scene-graph-analyzer-meter-update-interval", 2.0);

ConfigVariableInt scene_graph_analyzer_meter_layer_sort
("scene-graph-analyzer-meter-layer-sort", 1000);

ConfigVariableDouble scene_graph_analyzer_meter_scale
("scene-graph-analyzer-meter-scale", 0.05);

ConfigVariableDouble scene_graph_analyzer_meter_side_margins
("scene-graph-analyzer-meter-side-margins", 0.5);

ConfigVariableBool movies_sync_pages
("movies-sync-pages", true,
 PRC_DESC("Set this true to force multi-page MovieTextures to hold pages "
          "back if necessary until all pages are ready to render at once, "
          "so that the multiple pages of a single movie are always in sync "
          "with each other.  Set this false to allow individual pages to be "
          "visible as soon as they come available, which means pages might "
          "sometimes be out of sync.  This only affects multi-page MovieTextures "
          "such as cube maps, 3-d textures, or stereo textures, or textures "
          "with separate color and alpha channel movie sources."));

ConfigVariableInt pfm_vis_max_vertices
("pfm-vis-max-vertices", 65535,
 PRC_DESC("Specifies the maximum number of vertex entries that may appear in "
          "a single generated mesh.  If the mesh would require more than that, "
          "the mesh is subdivided into smaller pieces."));

ConfigVariableInt pfm_vis_max_indices
("pfm-vis-max-indices", 1048576,
 PRC_DESC("Specifies the maximum number of vertex references that may appear in "
          "a single generated mesh.  If the mesh would require more than that, "
          "the mesh is subdivided into smaller pieces."));

ConfigVariableDouble ae_undershift_factor_16
("ae-undershift-factor-16", 1.004,
 PRC_DESC("Specifies the factor by which After Effects under-applies the specified "
          "maximum pixel shift when applying a displacement map, in a 16-bit project file.  This is used "
          "to control PfmVizzer::make_displacement()."));

ConfigVariableDouble ae_undershift_factor_32
("ae-undershift-factor-32", 1.0,
 PRC_DESC("Specifies the factor by which After Effects under-applies the specified "
          "maximum pixel shift when applying a displacement map, in a 32-bit project file.  This is used "
          "to control PfmVizzer::make_displacement()."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libgrutil() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  FrameRateMeter::init_type();
  MeshDrawer::init_type();
  MeshDrawer2D::init_type();
  GeoMipTerrain::init_type();
  NodeVertexTransform::init_type();
  RigidBodyCombiner::init_type();
  PipeOcclusionCullTraverser::init_type();
  SceneGraphAnalyzerMeter::init_type();
  ShaderTerrainMesh::init_type();

#ifdef HAVE_AUDIO
  MovieTexture::init_type();
  MovieTexture::register_with_read_factory();

  TexturePool *ts = TexturePool::get_global_ptr();
  ts->register_texture_type(MovieTexture::make_texture, "avi m2v mov mpg mpeg mp4 wmv asf flv nut ogm mkv ogv webm");
#endif
}
