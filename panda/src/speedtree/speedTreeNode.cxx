/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file speedTreeNode.cxx
 * @author drose
 * @date 2009-03-13
 */

#include "pandabase.h"
#include "speedTreeNode.h"
#include "stBasicTerrain.h"
#include "virtualFileSystem.h"
#include "config_putil.h"
#include "cullTraverser.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "omniBoundingVolume.h"
#include "boundingSphere.h"
#include "boundingBox.h"
#include "clockObject.h"
#include "geomDrawCallbackData.h"
#include "graphicsStateGuardian.h"
#include "textureAttrib.h"
#include "lightAttrib.h"
#include "directionalLight.h"
#include "ambientLight.h"
#include "loader.h"
#include "deg_2_rad.h"
#include "sceneGraphReducer.h"
#include "pStatTimer.h"

#ifdef SPEEDTREE_OPENGL
#include <glew/glew.h>
#endif  // SPEEDTREE_OPENGL

#ifdef SPEEDTREE_DIRECTX9
#include "dxGraphicsStateGuardian9.h"
#endif

using std::istream;
using std::ostream;
using std::string;

double SpeedTreeNode::_global_time_delta = 0.0;
bool SpeedTreeNode::_authorized;
bool SpeedTreeNode::_done_first_init;
TypeHandle SpeedTreeNode::_type_handle;
TypeHandle SpeedTreeNode::DrawCallback::_type_handle;

PStatCollector SpeedTreeNode::_cull_speedtree_pcollector("Cull:SpeedTree");
PStatCollector SpeedTreeNode::_cull_speedtree_shadows_pcollector("Cull:SpeedTree:Shadows");
PStatCollector SpeedTreeNode::_cull_speedtree_trees_pcollector("Cull:SpeedTree:Trees");
PStatCollector SpeedTreeNode::_cull_speedtree_terrain_pcollector("Cull:SpeedTree:Terrain");
PStatCollector SpeedTreeNode::_draw_speedtree_pcollector("Draw:SpeedTree");
PStatCollector SpeedTreeNode::_draw_speedtree_shadows_pcollector("Draw:SpeedTree:Shadows");
PStatCollector SpeedTreeNode::_draw_speedtree_trees_pcollector("Draw:SpeedTree:Trees");
PStatCollector SpeedTreeNode::_draw_speedtree_terrain_pcollector("Draw:SpeedTree:Terrain");
PStatCollector SpeedTreeNode::_draw_speedtree_terrain_update_pcollector("Draw:SpeedTree:Terrain:Update");

/**
 *
 */
SpeedTreeNode::
SpeedTreeNode(const string &name) :
  PandaNode(name),
#ifdef ST_DELETE_FOREST_HACK
  // Early versions of SpeedTree don't destruct unused CForestRender objects
  // correctly.  To avoid crashes, we have to leak these things.
  _forest_render(*(new SpeedTree::CForestRender)),
#endif
  _time_delta(0.0)
{
  init_node();
  // For now, set an infinite bounding volume.  Maybe in the future we'll
  // change this to match whatever set of trees we're holding, though it
  // probably doesn't really matter too much.  set_internal_bounds(new
  // OmniBoundingVolume); set_internal_bounds(new
  // BoundingSphere(LPoint3::zero(), 10.0f));

  // Intialize the render params.  First, get the shader directory.
  Filename shaders_dir = speedtree_shaders_dir;

  // We expect the shader directory to contain at least this one token
  // filename (to prove it's the right directory).
  Filename token_filename = "Branch.hlsl";
  if (!Filename(shaders_dir, token_filename).exists()) {
    // If that shader directory doesn't work, look along the model-path.
    if (token_filename.resolve_filename(get_model_path())) {
      shaders_dir = token_filename.get_dirname();
    } else {
      if (!shaders_dir.is_directory()) {
        speedtree_cat.warning()
          << "speedtree-shaders-dir is set to " << shaders_dir
          << ", which doesn't exist.\n";
      } else {
        speedtree_cat.warning()
          << "speedtree-shaders-dir is set to " << shaders_dir
          << ", which exists but doesn't contain " <<  token_filename
          << ".\n";
      }
    }
  }

  _os_shaders_dir = shaders_dir.to_os_specific();
  // Ensure the path ends with a terminal slash; SpeedTree requires this.
#if defined(WIN32) || defined(WIN64)
  if (!_os_shaders_dir.empty() && _os_shaders_dir[_os_shaders_dir.length() - 1] != '\\') {
    _os_shaders_dir += "\\";
  }
#else
  if (!_os_shaders_dir.empty() && _os_shaders_dir[_os_shaders_dir.length() - 1] != '/') {
    _os_shaders_dir += "/";
  }
#endif

  SpeedTree::SForestRenderInfo render_info;
  render_info.m_strShaderPath = _os_shaders_dir.c_str();
  _forest_render.SetRenderInfo(render_info);

  // Now apply the rest of the config settings.
  reload_config();
}

/**
 * Returns the total number of trees that will be rendered by this node,
 * counting all instances of all trees.
 */
int SpeedTreeNode::
count_total_instances() const {
  int total_instances = 0;
  Trees::const_iterator ti;
  for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
    InstanceList *instance_list = (*ti);
    total_instances += instance_list->get_num_instances();
  }

  return total_instances;
}

/**
 * Adds a new tree for rendering.  Returns the InstanceList which can be used
 * to add to the instances for this tree.  If the tree has previously been
 * added, returns the existing InstanceList.
 */
SpeedTreeNode::InstanceList &SpeedTreeNode::
add_tree(const STTree *tree) {
  // TODO: These should be nassertr instead of assert, but there's nothing we
  // can really return when the assert fails.
  assert(is_valid());
  assert(tree->is_valid());

  InstanceList ilist(tree);
  Trees::iterator ti = _trees.find(&ilist);
  if (ti == _trees.end()) {
    // This is the first time that this particular tree has been added.
    InstanceList *instance_list = new InstanceList(tree);
    std::pair<Trees::iterator, bool> result = _trees.insert(instance_list);
    ti = result.first;
    bool inserted = result.second;
    nassertr(inserted, *(*ti));

    if (!_forest_render.RegisterTree((SpeedTree::CTree *)tree->get_tree())) {
      speedtree_cat.warning()
        << "Failed to register tree " << tree->get_fullpath() << "\n";
      write_error(speedtree_cat.warning());
    }
  }

  _needs_repopulate = true;
  mark_internal_bounds_stale();
  InstanceList *instance_list = (*ti);
  return *instance_list;
}

/**
 * Removes all instances of the indicated tree.  Returns the number of
 * instances removed.
 */
int SpeedTreeNode::
remove_tree(const STTree *tree) {
  InstanceList ilist(tree);
  Trees::iterator ti = _trees.find(&ilist);
  if (ti == _trees.end()) {
    // The tree was not already present.
    return 0;
  }

  if (!_forest_render.UnregisterTree(tree->get_tree())) {
    speedtree_cat.warning()
      << "Failed to unregister tree " << tree->get_fullpath() << "\n";
    write_error(speedtree_cat.warning());
  }

  _needs_repopulate = true;
  mark_internal_bounds_stale();

  InstanceList *instance_list = (*ti);
  int num_removed = instance_list->get_num_instances();
  _trees.erase(ti);
  delete instance_list;

  return num_removed;
}

/**
 * Removes all instances of all trees from the node.
 */
void SpeedTreeNode::
remove_all_trees() {
  Trees::iterator ti;
  for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
    InstanceList *instance_list = (*ti);
    const STTree *tree = instance_list->get_tree();
    if (!_forest_render.UnregisterTree(tree->get_tree())) {
      speedtree_cat.warning()
        << "Failed to unregister tree " << tree->get_fullpath() << "\n";
      write_error(speedtree_cat.warning());
    }
    delete instance_list;
  }

  _trees.clear();
  _needs_repopulate = true;
  mark_internal_bounds_stale();
}

/**
 * Returns true if the indicated tree has any instances within this node,
 * false otherwise.
 */
bool SpeedTreeNode::
has_instance_list(const STTree *tree) const {
  InstanceList ilist(tree);
  Trees::const_iterator ti = _trees.find(&ilist);
  return (ti != _trees.end());
}

/**
 * Returns a list of transforms that corresponds to the instances at which the
 * indicated tree appears.  You should ensure that has_instance_list() returns
 * true before calling this method.
 */
const SpeedTreeNode::InstanceList &SpeedTreeNode::
get_instance_list(const STTree *tree) const {
  InstanceList ilist(tree);
  Trees::const_iterator ti = _trees.find(&ilist);
  if (ti == _trees.end()) {
    // The tree was not already present.
    static InstanceList empty_list(nullptr);
    return empty_list;
  }

  InstanceList *instance_list = (*ti);
  return *instance_list;
}

/**
 * Returns a modifiable list of transforms that corresponds to the instances
 * of this tree.  This is equivalent to add_tree().
 */
SpeedTreeNode::InstanceList &SpeedTreeNode::
modify_instance_list(const STTree *tree) {
  return add_tree(tree);
}

/**
 * Adds a new instance of the indicated tree at the indicated transform.
 */
void SpeedTreeNode::
add_instance(const STTree *tree, const STTransform &transform) {
  if (speedtree_follow_terrain && has_terrain()) {
    STTransform new_transform = transform;
    new_transform._pos[2] = _terrain->get_height(new_transform._pos[0], new_transform._pos[1]);
    add_tree(tree).add_instance(new_transform);
  } else {
    add_tree(tree).add_instance(transform);
  }
}

/**
 * Walks the scene graph beginning at root, looking for nested SpeedTreeNodes.
 * For each SpeedTreeNode found, adds all of the instances defined within that
 * SpeedTreeNode as instances of this node, after applying the indicated
 * scene-graph transform.
 */
void SpeedTreeNode::
add_instances(const NodePath &root, const TransformState *transform) {
  nassertv(!root.is_empty());
  r_add_instances(root.node(), transform->compose(root.get_transform()),
                  Thread::get_current_thread());
}

/**
 * Adds all of the instances defined within the indicated SpeedTreeNode as
 * instances of this node.  Does not recurse to children.
 */
void SpeedTreeNode::
add_instances_from(const SpeedTreeNode *other) {
  int num_trees = other->get_num_trees();
  for (int ti = 0; ti < num_trees; ++ti) {
    const InstanceList &other_instance_list = other->get_instance_list(ti);
    const STTree *tree = other_instance_list.get_tree();
    InstanceList &this_instance_list = add_tree(tree);

    int num_instances = other_instance_list.get_num_instances();
    for (int i = 0; i < num_instances; ++i) {
      STTransform other_trans = other_instance_list.get_instance(i);
      this_instance_list.add_instance(other_trans);
    }
  }
}

/**
 * Adds all of the instances defined within the indicated SpeedTreeNode as
 * instances of this node, after applying the indicated scene-graph transform.
 * Does not recurse to children.
 */
void SpeedTreeNode::
add_instances_from(const SpeedTreeNode *other, const TransformState *transform) {
  int num_trees = other->get_num_trees();
  for (int ti = 0; ti < num_trees; ++ti) {
    const InstanceList &other_instance_list = other->get_instance_list(ti);
    const STTree *tree = other_instance_list.get_tree();
    InstanceList &this_instance_list = add_tree(tree);

    int num_instances = other_instance_list.get_num_instances();
    for (int i = 0; i < num_instances; ++i) {
      CPT(TransformState) other_trans = other_instance_list.get_instance(i);
      CPT(TransformState) new_trans = transform->compose(other_trans);

      if (speedtree_follow_terrain && has_terrain()) {
        STTransform new_transform = new_trans;
        new_transform._pos[2] = _terrain->get_height(new_transform._pos[0], new_transform._pos[1]);
        this_instance_list.add_instance(new_transform);

      } else {
        this_instance_list.add_instance(new_trans.p());
      }
    }
  }
}

/**
 * Creates a number of random instances of the indicated true, within the
 * indicated range.  If a terrain is present, height_min and height_max
 * restrict trees to the (x, y) positions that fall within the indicated
 * terrain, and slope_min and slope_max restrict trees to the (x, y) positions
 * that have a matching slope.  If a terrain is not present, height_min and
 * height_max specify a random range of Z heights, and slope_min and slope_max
 * are ignored.
 */
void SpeedTreeNode::
add_random_instances(const STTree *tree, int quantity,
                     PN_stdfloat x_min, PN_stdfloat x_max,
                     PN_stdfloat y_min, PN_stdfloat y_max,
                     PN_stdfloat scale_min, PN_stdfloat scale_max,
                     PN_stdfloat height_min, PN_stdfloat height_max,
                     PN_stdfloat slope_min, PN_stdfloat slope_max,
                     Randomizer &randomizer) {
  InstanceList &instance_list = add_tree(tree);
  _needs_repopulate = true;

  for (int i = 0; i < quantity; ++i) {
    STTransform transform;
    transform._pos[0] = randomizer.random_real(x_max - x_min) + x_min;
    transform._pos[1] = randomizer.random_real(y_max - y_min) + y_min;
    transform._rotate = randomizer.random_real(360.0);
    transform._scale = randomizer.random_real(scale_max - scale_min) + scale_min;

    if (has_terrain()) {
      // Spin till we find a valid match with terrain.
      int repeat_count = speedtree_max_random_try_count;
      while (!_terrain->placement_is_acceptable(transform._pos[0], transform._pos[1], height_min, height_max, slope_min, slope_max)) {
        transform._pos[0] = randomizer.random_real(x_max - x_min) + x_min;
        transform._pos[1] = randomizer.random_real(y_max - y_min) + y_min;
        if (--repeat_count == 0) {
          nassert_raise("Exceeded speedtree-max-random-try-count; bad placement parameters?");
          return;
        }
      }
      transform._pos[2] = _terrain->get_height(transform._pos[0], transform._pos[1]);

    } else {
      // No terrain; just pick a random height.
      transform._pos[2] = randomizer.random_real(height_max - height_min) + height_min;
    }
    instance_list.add_instance(transform);
  }
}

/**
 * Opens and reads the named STF (SpeedTree Forest) file, and adds the SRT
 * files named within as instances of this node.  Returns true on success,
 * false on failure.
 */
bool SpeedTreeNode::
add_from_stf(const Filename &stf_filename, const LoaderOptions &options) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename fullpath = Filename::text_filename(stf_filename);
  vfs->resolve_filename(fullpath, get_model_path());

  if (!vfs->exists(fullpath)) {
    speedtree_cat.warning()
      << "Couldn't find " << stf_filename << "\n";
    return false;
  }

  PT(VirtualFile) file = vfs->get_file(fullpath);
  if (file == nullptr) {
    // No such file.
    speedtree_cat.error()
      << "Could not find " << stf_filename << "\n";
    return false;
  }

  if (speedtree_cat.is_debug()) {
    speedtree_cat.debug()
      << "Reading STF file " << fullpath << "\n";
  }

  istream *in = file->open_read_file(true);
  bool success = add_from_stf(*in, fullpath, options);
  vfs->close_read_file(in);

  return success;
}

/**
 * Reads text data from the indicated stream, which is understood to represent
 * the named STF (SpeedTree Forest) file, and adds the SRT files named within
 * as instances of this node.  Returns true on success, false on failure.
 *
 * The pathname is used for reference only; if nonempty, it provides a search
 * directory for named SRT files.
 *
 * The Loader and LoaderOptions, if provided, are used to load the SRT files.
 * If the Loader pointer is NULL, the default global Loader is used instead.
 */
bool SpeedTreeNode::
add_from_stf(istream &in, const Filename &pathname,
             const LoaderOptions &options, Loader *loader) {
  if (loader == nullptr) {
    loader = Loader::get_global_ptr();
  }
  string os_filename;

  Filename dirname = pathname.get_dirname();
  dirname.make_absolute();
  DSearchPath search;
  search.append_directory(dirname);

  typedef pmap<Filename, CPT(STTree) > AlreadyLoaded;
  AlreadyLoaded already_loaded;

  // The STF file format doesn't allow for spaces in the SRT filename.
  in >> os_filename;
  while (in && !in.eof()) {
    CPT(STTree) tree;
    Filename srt_filename = Filename::from_os_specific(os_filename);
    AlreadyLoaded::iterator ai = already_loaded.find(srt_filename);
    if (ai != already_loaded.end()) {
      tree = (*ai).second;
    } else {
      // Resolve the SRT filename relative to the STF file first.
      srt_filename.resolve_filename(search);

      // Now load up the SRT file using the Panda loader (which will also
      // search the model-path if necessary).
      PT(PandaNode) srt_root = loader->load_sync(srt_filename);

      if (srt_root != nullptr) {
        NodePath srt(srt_root);
        NodePath srt_np = srt.find("**/+SpeedTreeNode");
        if (!srt_np.is_empty()) {
          SpeedTreeNode *srt_node = DCAST(SpeedTreeNode, srt_np.node());
          if (srt_node->get_num_trees() >= 1) {
            tree = srt_node->get_tree(0);
          }
        }
      }
      already_loaded[srt_filename] = tree;
    }

    // Now we've loaded the SRT data, so apply it the appropriate number of
    // times to the locations specified.
    int num_instances;
    in >> num_instances;
    for (int ni = 0; ni < num_instances && in && !in.eof(); ++ni) {
      LPoint3 pos;
      PN_stdfloat rotate, scale;
      in >> pos[0] >> pos[1] >> pos[2] >> rotate >> scale;

      if (!speedtree_5_2_stf) {
        // 5.1 or earlier stf files also included these additional values,
        // which we will ignore:
        PN_stdfloat height_min, height_max, slope_min, slope_max;
        in >> height_min >> height_max >> slope_min >> slope_max;
      }

      if (tree != nullptr) {
        add_instance(tree, STTransform(pos, rad_2_deg(rotate), scale));
      }
    }
    in >> os_filename;
  }

  // Consume any whitespace at the end of the file.
  in >> std::ws;

  if (!in.eof()) {
    // If we didn't read all the way to end-of-file, there was an error.
    in.clear();
    string text;
    in >> text;
    speedtree_cat.error()
      << "Unexpected text in " << pathname << " at \"" << text << "\"\n";
    return false;
  }

  // Return true if we successfully read all the way to end-of-file.
  return true;
}

/**
 * A convenience function to set up terrain geometry by reading a terrain.txt
 * file as defined by SpeedTree.  This file names the various map files that
 * define the terrain, as well as defining parameters size as its size and
 * color.
 *
 * This method implicitly creates a STBasicTerrain object and passes it to
 * set_terrain().
 */
bool SpeedTreeNode::
setup_terrain(const Filename &terrain_file) {
  PT(STBasicTerrain) terrain = new STBasicTerrain;
  if (terrain->setup_terrain(terrain_file)) {
    set_terrain(terrain);
    return true;
  }

  return false;
}

/**
 * Associated a terrain with the node.  If the terrain has not already been
 * loaded prior to this call, load_data() will be called immediately.
 *
 * The terrain will be rendered using SpeedTree callbacks, and trees may be
 * repositioned with a call to snap_to_terrain().
 */
void SpeedTreeNode::
set_terrain(STTerrain *terrain) {
  _terrain = nullptr;
  _needs_repopulate = true;

  if (terrain == nullptr) {
    return;
  }

  if (!terrain->is_valid()) {
    // If the terrain was not already loaded, load it immediately.
    terrain->load_data();
  }

  nassertv(terrain->is_valid());
  nassertv(terrain->get_num_splat_layers() == SpeedTree::c_nNumTerrainSplatLayers);
  _terrain = terrain;

  _terrain_render.SetShaderLoader(_forest_render.GetShaderLoader());

  SpeedTree::STerrainRenderInfo trender_info;
  trender_info.m_strShaderPath = _os_shaders_dir.c_str();

  string os_specific = terrain->get_normal_map().to_os_specific();
  trender_info.m_strNormalMap = os_specific.c_str();
  os_specific = terrain->get_splat_map().to_os_specific();
  trender_info.m_strSplatMap = os_specific.c_str();

  for (int i = 0; i < SpeedTree::c_nNumTerrainSplatLayers; ++i) {
    os_specific = terrain->get_splat_layer(i).to_os_specific();
    trender_info.m_astrSplatLayers[i] = os_specific.c_str();
    trender_info.m_afSplatTileValues[i] = terrain->get_splat_layer_tiling(i);
  }

  trender_info.m_fNormalMapBlueScale = 1.0f;
  trender_info.m_bShadowsEnabled = false; // what does this do?
  trender_info.m_bZPrePass = false;

  _terrain_render.SetRenderInfo(trender_info);

  _terrain_render.SetHeightHints(terrain->get_min_height(), terrain->get_max_height());

  if (speedtree_follow_terrain) {
    snap_to_terrain();
  }
}

/**
 * Adjusts all the trees in this node so that their Z position matches the
 * height of the terrain at their X, Y position.
 */
void SpeedTreeNode::
snap_to_terrain() {
  Trees::iterator ti;
  for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
    InstanceList *instance_list = (*ti);

    int num_instances = instance_list->get_num_instances();
    if (_terrain != nullptr) {
      for (int i = 0; i < num_instances; ++i) {
        STTransform trans = instance_list->get_instance(i);
        LPoint3 pos = trans.get_pos();
        pos[2] = _terrain->get_height(pos[0], pos[1]);
        trans.set_pos(pos);
        instance_list->set_instance(i, trans);
      }
    } else {
      for (int i = 0; i < num_instances; ++i) {
        STTransform trans = instance_list->get_instance(i);
        LPoint3 pos = trans.get_pos();
        pos[2] = 0.0f;
        trans.set_pos(pos);
        instance_list->set_instance(i, trans);
      }
    }
  }

  _needs_repopulate = true;
}

/**
 * Re-reads the current setting of all of the relevant config variables and
 * applies them to this node.  This can be called after changing config
 * settings, to make them apply to this particular node.
 */
void SpeedTreeNode::
reload_config() {

  _shadow_infos.clear();
  int num_shadow_maps = speedtree_cascading_shadow_splits.get_num_words();
  if (num_shadow_maps > SpeedTree::c_nMaxNumShadowMaps) {
    speedtree_cat.warning()
      << "SpeedTree is current compiled to support a maximum of "
      << SpeedTree::c_nMaxNumShadowMaps << " shadow maps.\n";
    num_shadow_maps = SpeedTree::c_nMaxNumShadowMaps;
  }
  _shadow_infos.insert(_shadow_infos.begin(), num_shadow_maps, ShadowInfo());
  for (int smi = 0; smi < num_shadow_maps; ++smi) {
    _shadow_infos[smi]._shadow_split = speedtree_cascading_shadow_splits[smi];
  }

  SpeedTree::SForestRenderInfo render_info = _forest_render.GetRenderInfo();

  render_info.m_nMaxAnisotropy = speedtree_max_anisotropy;
  render_info.m_bHorizontalBillboards = speedtree_horizontal_billboards;
  render_info.m_fAlphaTestScalar = speedtree_alpha_test_scalar;
  render_info.m_bZPrePass = speedtree_z_pre_pass;
  render_info.m_nMaxBillboardImagesByBase = speedtree_max_billboard_images_by_base;
  render_info.m_fVisibility = speedtree_visibility;
  render_info.m_fGlobalLightScalar = speedtree_global_light_scalar;
  render_info.m_sLightMaterial.m_vSpecular = SpeedTree::Vec4(speedtree_specular_color[0], speedtree_specular_color[1], speedtree_specular_color[2], speedtree_specular_color[3]);
  render_info.m_sLightMaterial.m_vEmissive = SpeedTree::Vec4(speedtree_emissive_color[0], speedtree_emissive_color[1], speedtree_emissive_color[2], speedtree_emissive_color[3]);
  render_info.m_bSpecularLighting = speedtree_specular_lighting;
  render_info.m_bTransmissionLighting = speedtree_transmission_lighting;
  render_info.m_bDetailLayer = speedtree_detail_layer;
  render_info.m_bDetailNormalMapping = speedtree_detail_normal_mapping;
  render_info.m_bAmbientContrast = speedtree_ambient_contrast;
  render_info.m_fTransmissionScalar = speedtree_transmission_scalar;
  render_info.m_fFogStartDistance = speedtree_fog_distance[0];
  render_info.m_fFogEndDistance = speedtree_fog_distance[1];
  render_info.m_vFogColor = SpeedTree::Vec3(speedtree_fog_color[0], speedtree_fog_color[1], speedtree_fog_color[2]);
  render_info.m_vSkyColor = SpeedTree::Vec3(speedtree_sky_color[0], speedtree_sky_color[1], speedtree_sky_color[2]);
  render_info.m_fSkyFogMin = speedtree_sky_fog[0];
  render_info.m_fSkyFogMax = speedtree_sky_fog[1];
  render_info.m_vSunColor = SpeedTree::Vec3(speedtree_sun_color[0], speedtree_sun_color[1], speedtree_sun_color[2]);
  render_info.m_fSunSize = speedtree_sun_size;
  render_info.m_fSunSpreadExponent = speedtree_sun_spread_exponent;
  render_info.m_fSunFogBloom = speedtree_sun_fog_bloom;
  render_info.m_nNumShadowMaps = num_shadow_maps;
  render_info.m_nShadowMapResolution = speedtree_shadow_map_resolution;
  render_info.m_bSmoothShadows = speedtree_smooth_shadows;
  render_info.m_bShowShadowSplitsOnTerrain = speedtree_show_shadow_splits_on_terrain;
  render_info.m_bWindEnabled = speedtree_wind_enabled;
  render_info.m_bFrondRippling = speedtree_frond_rippling;

  _forest_render.SetRenderInfo(render_info);

  _terrain_render.SetMaxAnisotropy(speedtree_max_anisotropy);
  _terrain_render.SetHint(SpeedTree::CTerrain::HINT_MAX_NUM_VISIBLE_CELLS,
                          speedtree_max_num_visible_cells);
  _visible_terrain.Reserve(speedtree_max_num_visible_cells);

  _needs_repopulate = true;
}

/**
 * Specifies the overall wind strength and direction.  Gusts are controlled
 * internally.
 */
void SpeedTreeNode::
set_wind(double strength, const LVector3 &direction) {
  _forest_render.SetGlobalWindStrength(strength);
  _forest_render.SetGlobalWindDirection(SpeedTree::Vec3(direction[0], direction[1], direction[2]));
}

/**
 * Make this call to initialized the SpeedTree API and verify the license.  If
 * an empty string is passed for the license, the config variable speedtree-
 * license is consulted.  Returns true on success, false on failure.  If this
 * call is not made explicitly, it will be made implicitly the first time a
 * SpeedTreeNode is created.
 */
bool SpeedTreeNode::
authorize(const string &license) {
  if (!_authorized) {
    if (!license.empty()) {
      SpeedTree::CCore::Authorize(license.c_str());
    } else {
      if (!speedtree_license.empty()) {
        SpeedTree::CCore::Authorize(speedtree_license.c_str());
      }
    }

    _authorized = SpeedTree::CCore::IsAuthorized();

    SpeedTree::CCore::SetTextureFlip(true);
  }

  return _authorized;
}

/**
 *
 */
SpeedTreeNode::
SpeedTreeNode(const SpeedTreeNode &copy) :
  PandaNode(copy),
  _os_shaders_dir(copy._os_shaders_dir),
  _shadow_infos(copy._shadow_infos),
#ifdef ST_DELETE_FOREST_HACK
  // Early versions of SpeedTree don't destruct unused CForestRender objects
  // correctly.  To avoid crashes, we have to leak these things.
  _forest_render(*(new SpeedTree::CForestRender)),
#endif
  _time_delta(copy._time_delta)
{
  init_node();

  _forest_render.SetRenderInfo(copy._forest_render.GetRenderInfo());
  _terrain_render.SetRenderInfo(copy._terrain_render.GetRenderInfo());

  // No way to copy these parameters, so we just re-assign them.
  _terrain_render.SetMaxAnisotropy(speedtree_max_anisotropy);
  _terrain_render.SetHint(SpeedTree::CTerrain::HINT_MAX_NUM_VISIBLE_CELLS,
                          speedtree_max_num_visible_cells);
  _visible_terrain.Reserve(speedtree_max_num_visible_cells);

  Trees::const_iterator ti;
  for (ti = copy._trees.begin(); ti != copy._trees.end(); ++ti) {
    InstanceList *instance_list = (*ti);
    const STTree *tree = instance_list->get_tree();
    if (!_forest_render.RegisterTree((SpeedTree::CTree *)tree->get_tree())) {
      speedtree_cat.warning()
        << "Failed to register tree " << tree->get_fullpath() << "\n";
      write_error(speedtree_cat.warning());
    }

    _trees.push_back(new InstanceList(*instance_list));
  }
  _trees.sort();

  set_terrain(copy._terrain);

  _needs_repopulate = true;
  mark_internal_bounds_stale();
}

/**
 *
 */
SpeedTreeNode::
~SpeedTreeNode() {
  remove_all_trees();
  // Help reduce memory waste from ST_DELETE_FOREST_HACK.
  _forest_render.ClearInstances();
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *SpeedTreeNode::
make_copy() const {
  return new SpeedTreeNode(*this);
}

/**
 * Collapses this node with the other node, if possible, and returns a pointer
 * to the combined node, or NULL if the two nodes cannot safely be combined.
 *
 * The return value may be this, other, or a new node altogether.
 *
 * This function is called from GraphReducer::flatten(), and need not deal
 * with children; its job is just to decide whether to collapse the two nodes
 * and what the collapsed node should look like.
 */
PandaNode *SpeedTreeNode::
combine_with(PandaNode *other) {
  if (is_exact_type(get_class_type()) &&
      other->is_exact_type(get_class_type())) {
    // Two SpeedTreeNodes can combine by moving trees from one to the other,
    // similar to the way GeomNodes combine.
    SpeedTreeNode *gother = DCAST(SpeedTreeNode, other);

    // But, not if they both have a terrain set.
    if (has_terrain() && gother->has_terrain()) {
      return nullptr;

    } else if (gother->has_terrain()) {
      set_terrain(gother->get_terrain());
    }

    add_instances_from(gother);
    return this;
  }

  return PandaNode::combine_with(other);
}

/**
 * Applies whatever attributes are specified in the AccumulatedAttribs object
 * (and by the attrib_types bitmask) to the vertices on this node, if
 * appropriate.  If this node uses geom arrays like a GeomNode, the supplied
 * GeomTransformer may be used to unify shared arrays across multiple
 * different nodes.
 *
 * This is a generalization of xform().
 */
void SpeedTreeNode::
apply_attribs_to_vertices(const AccumulatedAttribs &attribs, int attrib_types,
                          GeomTransformer &transformer) {
  if ((attrib_types & SceneGraphReducer::TT_transform) != 0) {
    STTransform xform = attribs._transform;
    Trees::iterator ti;
    for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
      InstanceList *instance_list = (*ti);
      STInstances &instances = instance_list->_instances;
      STInstances::iterator sti;
      for (sti = instances.begin(); sti != instances.end(); ++sti) {
        STTransform orig_transform = *sti;
        (*sti) = orig_transform * xform;
      }
    }
  }
  mark_internal_bounds_stale();
}

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool SpeedTreeNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  if (!_is_valid) {
    return false;
  }
  PStatTimer timer(_cull_speedtree_pcollector);

  GraphicsStateGuardian *gsg = DCAST(GraphicsStateGuardian, trav->get_gsg());
  nassertr(gsg != nullptr, true);
  if (!validate_api(gsg)) {
    return false;
  }

  ClockObject *clock = ClockObject::get_global_clock();
  _forest_render.SetGlobalTime(clock->get_frame_time() + _time_delta + _global_time_delta);
  _forest_render.AdvanceGlobalWind();

  // Compute the modelview and camera transforms, to pass to the SpeedTree
  // CView structure.
  CPT(TransformState) orig_modelview = data.get_modelview_transform(trav);
  CPT(TransformState) modelview = trav->get_scene()->get_cs_transform()->compose(orig_modelview);
  CPT(TransformState) camera_transform = modelview->invert_compose(TransformState::make_identity());
  LMatrix4f modelview_mat = LCAST(float, modelview->get_mat());
  const LPoint3 &camera_pos = camera_transform->get_pos();
  const Lens *lens = trav->get_scene()->get_lens();

  LMatrix4f projection_mat =
    LCAST(float, LMatrix4::convert_mat(gsg->get_internal_coordinate_system(), lens->get_coordinate_system()) *
          lens->get_projection_mat());

  _view.Set(SpeedTree::Vec3(camera_pos[0], camera_pos[1], camera_pos[2]),
            SpeedTree::Mat4x4(projection_mat.get_data()),
            SpeedTree::Mat4x4(modelview_mat.get_data()),
            lens->get_near(), lens->get_far());

  // Convert the render state to SpeedTree's input.
  const RenderState *state = data._state;

  // Check texture state.  If all textures are disabled, then we ask SpeedTree
  // to disable textures.
  bool show_textures = true;
  const TextureAttrib *ta = DCAST(TextureAttrib, state->get_attrib(TextureAttrib::get_class_slot()));
  if (ta != nullptr) {
    show_textures = !ta->has_all_off();
  }
  _forest_render.EnableTexturing(show_textures);
  _terrain_render.EnableTexturing(show_textures);

  // Check lighting state.  SpeedTree only supports a single directional
  // light; we look for a directional light in the lighting state and pass its
  // direction and color to SpeedTree.  We also accumulate the ambient light
  // colors.
  LColor ambient_color(0.0f, 0.0f, 0.0f, 0.0f);
  DirectionalLight *dlight = nullptr;
  NodePath dlight_np;
  LColor diffuse_color;

  int diffuse_priority = 0;
  const LightAttrib *la = DCAST(LightAttrib, state->get_attrib(LightAttrib::get_class_slot()));
  if (la != nullptr) {
    for (int i = 0; i < la->get_num_on_lights(); ++i) {
      NodePath light = la->get_on_light(i);
      if (!light.is_empty() && light.node()->is_of_type(DirectionalLight::get_class_type())) {
        // A directional light.
        DirectionalLight *light_obj = DCAST(DirectionalLight, light.node());
        if (dlight == nullptr || light_obj->get_priority() > dlight->get_priority()) {
          // Here's the most important directional light.
          dlight = light_obj;
          dlight_np = light;
        }
      } else if (!light.is_empty() && light.node()->is_of_type(AmbientLight::get_class_type())) {
        // An ambient light.  We keep the color only.
        AmbientLight *light_obj = DCAST(AmbientLight, light.node());
        ambient_color += light_obj->get_color();
      }
    }
  }

  if (dlight != nullptr) {
    CPT(TransformState) transform = dlight_np.get_transform(trav->get_scene()->get_scene_root().get_parent());
    LVector3 dir = dlight->get_direction() * transform->get_mat();
    dir.normalize();
    _light_dir = SpeedTree::Vec3(dir[0], dir[1], dir[2]);
    diffuse_color = dlight->get_color();

  } else {
    // No light.  But there's no way to turn off lighting in SpeedTree.  In
    // lieu of this, we just shine a light from above.
    _light_dir = SpeedTree::Vec3(0.0, 0.0, -1.0);

    // Also, we set ambient and diffuse colors to the same full-white value.
    ambient_color.set(1.0f, 1.0f, 1.0f, 1.0f);
    diffuse_color.set(1.0f, 1.0f, 1.0f, 1.0f);
  }

  SpeedTree::SForestRenderInfo render_info = _forest_render.GetRenderInfo();
  render_info.m_sLightMaterial.m_vAmbient = SpeedTree::Vec4(ambient_color[0], ambient_color[1], ambient_color[2], 1.0f);
  render_info.m_sLightMaterial.m_vDiffuse = SpeedTree::Vec4(diffuse_color[0], diffuse_color[1], diffuse_color[2], 1.0f);
  _forest_render.SetRenderInfo(render_info);

  _forest_render.SetLightDir(_light_dir);

  SpeedTree::st_float32 updated_splits[SpeedTree::c_nMaxNumShadowMaps];
  memset(updated_splits, 0, sizeof(updated_splits));
  for (int smi = 0; smi < (int)_shadow_infos.size(); ++smi) {
    updated_splits[smi] = _shadow_infos[smi]._shadow_split;
  };

  _forest_render.SetCascadedShadowMapDistances(updated_splits, lens->get_far());
  _forest_render.SetShadowFadePercentage(speedtree_shadow_fade);

  if (!_needs_repopulate) {
    // Don't bother culling now unless we're correctly fully populated.
    // (Culling won't be accurate unless the forest has been populated, but we
    // have to be in the draw traversal to populate.)
    cull_forest();
  }

  // Recurse onto the node's children.
  return true;
}

/**
 * Returns true if there is some value to visiting this particular node during
 * the cull traversal for any camera, false otherwise.  This will be used to
 * optimize the result of get_net_draw_show_mask(), so that any subtrees that
 * contain only nodes for which is_renderable() is false need not be visited.
 */
bool SpeedTreeNode::
is_renderable() const {
  return true;
}

/**
 * Adds the node's contents to the CullResult we are building up during the
 * cull traversal, so that it will be drawn at render time.  For most nodes
 * other than GeomNodes, this is a do-nothing operation.
 */
void SpeedTreeNode::
add_for_draw(CullTraverser *trav, CullTraverserData &data) {
  if (_is_valid) {
    // We create a CullableObject that has an explicit draw_callback into this
    // node, so that we can make the appropriate calls into SpeedTree to
    // render the forest during the actual draw.
    CullableObject *object =
      new CullableObject(nullptr, data._state,
                         TransformState::make_identity());
    object->set_draw_callback(new DrawCallback(this));
    trav->get_cull_handler()->record_object(object, trav);
  }
}

/**
 * Walks through the scene graph beginning at this node, and does whatever
 * initialization is required to render the scene properly with the indicated
 * GSG.  It is not strictly necessary to call this, since the GSG will
 * initialize itself when the scene is rendered, but this may take some of the
 * overhead away from that process.
 *
 * In particular, this will ensure that textures within the scene are loaded
 * in texture memory, and display lists are built up from static geometry.
 */
void SpeedTreeNode::
prepare_scene(GraphicsStateGuardianBase *gsgbase, const RenderState *) {
  GraphicsStateGuardian *gsg = DCAST(GraphicsStateGuardian, gsgbase);
  if (validate_api(gsg)) {
    setup_for_render(gsg);
  }
}

/**
 * Returns a newly-allocated BoundingVolume that represents the internal
 * contents of the node.  Should be overridden by PandaNode classes that
 * contain something internally.
 */
void SpeedTreeNode::
compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                        int &internal_vertices,
                        int pipeline_stage,
                        Thread *current_thread) const {
  internal_vertices = 0;

  SpeedTree::CExtents extents;
  Trees::const_iterator ti;
  for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
    InstanceList *instance_list = (*ti);
    const STTree *tree = instance_list->get_tree();

    const STInstances &st_instances = instance_list->_instances;
    STInstances::const_iterator ii;
    for (ii = st_instances.begin(); ii != st_instances.end(); ++ii) {
      SpeedTree::CExtents tree_extents = tree->get_tree()->GetExtents();
      tree_extents.Rotate((*ii).GetRotationAngle());
      tree_extents.Scale((*ii).GetScale());
      tree_extents.Translate((*ii).GetPos());
      extents.ExpandAround(tree_extents);
    }
  }

  const SpeedTree::Vec3 &emin = extents.Min();
  const SpeedTree::Vec3 &emax = extents.Max();
  internal_bounds = new BoundingBox(LPoint3(emin[0], emin[1], emin[2]),
                                    LPoint3(emax[0], emax[1], emax[2]));
}

/**
 * Writes a brief description of the node to the indicated output stream.
 * This is invoked by the << operator.  It may be overridden in derived
 * classes to include some information relevant to the class.
 */
void SpeedTreeNode::
output(ostream &out) const {
  PandaNode::output(out);
  out
    << " (" << get_num_trees() << " unique trees with "
    << count_total_instances() << " total instances)";
}

/**
 *
 */
void SpeedTreeNode::
write(ostream &out, int indent_level) const {
  PandaNode::write(out, indent_level);

  // This makes NodePath.ls() too confusing.
  /*
  Trees::const_iterator ti;
  for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
    InstanceList *instance_list = (*ti);
    indent(out, indent_level + 2)
      << *instance_list << "\n";
  }
  */
}


/**
 * Writes the current SpeedTree error message to the indicated stream.
 */
void SpeedTreeNode::
write_error(ostream &out) {
  const char *error = SpeedTree::CCore::GetError();
  if (error != nullptr) {
    out << error;
  }
  out << "\n";
}

/**
 * Uses SpeedTree::CRenderState to set the indicated transparency mode.
 */
void SpeedTreeNode::
set_transparent_texture_mode(SpeedTree::ETextureAlphaRenderMode eMode) const {
  // turn all modes off (no telling what render state the client application
  // might be in before this call)
  SpeedTree::CRenderState::SetBlending(false);
  SpeedTree::CRenderState::SetAlphaTesting(false);
  SpeedTree::CRenderState::SetAlphaToCoverage(false);

  switch (eMode) {
  case SpeedTree::TRANS_TEXTURE_ALPHA_TESTING:
    SpeedTree::CRenderState::SetAlphaTesting(true);
    break;
  case SpeedTree::TRANS_TEXTURE_ALPHA_TO_COVERAGE:
    SpeedTree::CRenderState::SetAlphaToCoverage(true);
    break;
  case SpeedTree::TRANS_TEXTURE_BLENDING:
    SpeedTree::CRenderState::SetBlending(true);
    break;
  default:
    // intentionally do nothing (TRANS_TEXTURE_NOTHING)
    break;
  }
}

/**
 * Called from the constructor to initialize some internal values.
 */
void SpeedTreeNode::
init_node() {
  PandaNode::set_cull_callback();

  _is_valid = false;
  _needs_repopulate = false;

  // Ensure we have a license.
  if (!authorize()) {
    speedtree_cat.warning()
      << "SpeedTree license not available.\n";
    return;
  }

  _forest_render.SetHint(SpeedTree::CForest::HINT_MAX_NUM_VISIBLE_CELLS,
                         speedtree_max_num_visible_cells);

  _forest_render.SetCullCellSize(speedtree_cull_cell_size);

  // Doesn't appear to be necessary to call this explicitly.
  // _forest_render.EnableWind(true);

  _is_valid = true;
}

/**
 * The recursive implementation of add_instances().
 */
void SpeedTreeNode::
r_add_instances(PandaNode *node, const TransformState *transform,
                Thread *current_thread) {
  if (node->is_of_type(SpeedTreeNode::get_class_type()) && node != this) {
    SpeedTreeNode *other = DCAST(SpeedTreeNode, node);
    add_instances_from(other, transform);
  }

  Children children = node->get_children(current_thread);
  for (int i = 0; i < children.get_num_children(); i++) {
    PandaNode *child = children.get_child(i);
    CPT(TransformState) child_transform = transform->compose(child->get_transform());
    r_add_instances(child, child_transform, current_thread);
  }
}


/**
 * Rebuilds the internal structures as necessary for rendering.
 */
void SpeedTreeNode::
repopulate() {
  _forest_render.ClearInstances();

  Trees::iterator ti;
  for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
    InstanceList *instance_list = (*ti);
    const STTree *tree = instance_list->get_tree();
    const STInstances &instances = instance_list->_instances;
    if (instances.empty()) {
      // There are no instances, so don't bother.  (This shouldn't happen
      // often, because we remove trees from the SpeedTreeNode when their
      // instance list goes empty, though it's possible if the user has
      // explicitly removed all of the instances.)
      continue;
    }

    if (!_forest_render.AddInstances(tree->get_tree(), &instances[0], instances.size())) {
      speedtree_cat.warning()
        << "Failed to add " << instances.size()
        << " instances for " << *tree << "\n";
      write_error(speedtree_cat.warning());
    }
  }

  _forest_render.GetPopulationStats(_population_stats);
  print_forest_stats(_population_stats);

  // setup billboard caps based on instances-per-cell stats
  int max_instances_by_cell = 1;
  for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
    InstanceList *instance_list = (*ti);
    const STTree *tree = instance_list->get_tree();
    const STInstances &instances = instance_list->_instances;
    if (instances.empty()) {
      continue;
    }

    int max_instances = 1;
    SpeedTree::CMap<const SpeedTree::CTree*, SpeedTree::st_int32>::const_iterator si;
    si = _population_stats.m_mMaxNumInstancesPerCellPerBase.find(tree->get_tree());
    if (si != _population_stats.m_mMaxNumInstancesPerCellPerBase.end()) {
      max_instances = std::max(max_instances, (int)si->second);
    }

    max_instances_by_cell = std::max(max_instances_by_cell, max_instances);
  }

  _visible_trees.Reserve(_forest_render.GetBaseTrees(),
                         _forest_render.GetBaseTrees().size(),
                         speedtree_max_num_visible_cells,
                         max_instances_by_cell,
                         speedtree_horizontal_billboards);
}

/**
 * Called once a frame to load vertex data for newly-visible terrain cells.
 */
void SpeedTreeNode::
update_terrain_cells() {
  nassertv(has_terrain());

  SpeedTree::TTerrainCellArray &cells = _visible_terrain.m_aCellsToUpdate;

  int num_tile_res = _terrain_render.GetMaxTileRes();
  PN_stdfloat cell_size = _terrain_render.GetCellSize();

  // A temporary vertex data object for populating terrain.
  PT(GeomVertexData) vertex_data =
    new GeomVertexData("terrain", _terrain->get_vertex_format(),
                       GeomEnums::UH_static);
  int num_vertices = num_tile_res * num_tile_res;
  vertex_data->set_num_rows(num_vertices);
  size_t num_bytes = vertex_data->get_array(0)->get_data_size_bytes();

  int num_cells = (int)cells.size();
  for (int ci = 0; ci < num_cells; ++ci) {
    SpeedTree::CTerrainCell *cell = cells[ci];
    nassertv(cell != nullptr && cell->GetVbo() != nullptr);
    int cell_yi = cell->Row();
    int cell_xi = cell->Col();
    // cerr << "populating cell " << cell_xi << " " << cell_yi << "\n";

    _terrain->fill_vertices(vertex_data,
                            cell_xi * cell_size, cell_yi * cell_size,
                            cell_size, num_tile_res);

    const GeomVertexArrayData *array_data = vertex_data->get_array(0);
    CPT(GeomVertexArrayDataHandle) handle = array_data->get_handle();
    const unsigned char *data_pointer = handle->get_read_pointer(true);
    SpeedTree::CGeometryBuffer *vbo = (SpeedTree::CGeometryBuffer *)cell->GetVbo();

    nassertv(vbo->NumVertices() == num_tile_res * num_tile_res);
    nassertv(vbo->NumVertices() * vbo->VertexSize() == handle->get_data_size_bytes());
    vbo->OverwriteVertices(data_pointer, num_vertices, 0);
  }
}

/**
 * Returns true if the indicated GSG shares the appropriate API for this
 * SpeedTreeNode, false otherwise.
 */
bool SpeedTreeNode::
validate_api(GraphicsStateGuardian *gsg) {
  GraphicsPipe *pipe = gsg->get_pipe();
  nassertr(pipe != nullptr, true);

#if defined(SPEEDTREE_OPENGL)
  static const string compiled_api = "OpenGL";
#elif defined(SPEEDTREE_DIRECTX9)
  static const string compiled_api = "DirectX9";
#else
  #error Unexpected graphics API.
#endif

  if (pipe->get_interface_name() != compiled_api) {
    speedtree_cat.error()
      << "SpeedTree is compiled for " << compiled_api
      << ", cannot render with " << pipe->get_interface_name()
      << "\n";
    _is_valid = false;
    return false;
  }

  return true;
}

/**
 * Called when the node is visited during the draw traversal, by virtue of our
 * DrawCallback construct.  This makes the calls into SpeedTree to perform the
 * actual rendering.
 */
void SpeedTreeNode::
draw_callback(CallbackData *data) {
  PStatTimer timer(_draw_speedtree_pcollector);
  GeomDrawCallbackData *geom_cbdata;
  DCAST_INTO_V(geom_cbdata, data);

  GraphicsStateGuardian *gsg = DCAST(GraphicsStateGuardian, geom_cbdata->get_gsg());

  setup_for_render(gsg);

  // Set some initial state requirements.
  SpeedTree::CRenderState::SetAlphaFunction(SpeedTree::ALPHAFUNC_GREATER, 0.0f);

  // start the forest render
  _forest_render.StartRender();

  if (_forest_render.ShadowsAreEnabled()) {
    // Update the shadow maps.  TODO: consider updating these only every once
    // in a while, instead of every frame, as a simple optimization.
    PStatTimer timer(_draw_speedtree_shadows_pcollector);
    render_forest_into_shadow_maps();
    _forest_render.ClearBoundTextures( );
  }

  if (!_forest_render.UploadViewShaderParameters(_view)) {
    speedtree_cat.warning()
      << "Couldn't set view parameters\n";
    write_error(speedtree_cat.warning());
  }

  if (has_terrain()) {
    PStatTimer timer1(_draw_speedtree_terrain_pcollector);
    // Is this needed for terrain?
    _terrain_render.UploadShaderConstants
      (&_forest_render, _light_dir,
       _forest_render.GetRenderInfo().m_sLightMaterial);

    // set terrain render states
    set_transparent_texture_mode(SpeedTree::TRANS_TEXTURE_NOTHING);

    // render actual terrain
    bool terrain = _terrain_render.Render
      (&_forest_render, _visible_terrain, SpeedTree::RENDER_PASS_STANDARD,
       _light_dir, _forest_render.GetRenderInfo().m_sLightMaterial,
       &_forest_render.GetRenderStats());

    if (!terrain) {
      speedtree_cat.warning()
        << "Failed to render terrain\n";
      write_error(speedtree_cat.warning());

      // Clear the terrain so we don't keep spamming error messages.
      _terrain = nullptr;
    }
  }

  {
    // Now draw the actual trees.
    PStatTimer timer1(_draw_speedtree_trees_pcollector);

    // SpeedTree::ETextureAlphaRenderMode mode =
    // SpeedTree::TRANS_TEXTURE_ALPHA_TESTING;
    SpeedTree::ETextureAlphaRenderMode mode = SpeedTree::TRANS_TEXTURE_ALPHA_TO_COVERAGE;
    // SpeedTree::ETextureAlphaRenderMode mode =
    // SpeedTree::TRANS_TEXTURE_BLENDING; SpeedTree::ETextureAlphaRenderMode
    // mode = SpeedTree::TRANS_TEXTURE_NOTHING;
    set_transparent_texture_mode(SpeedTree::ETextureAlphaRenderMode(mode));

    bool branches = _forest_render.RenderBranches(_visible_trees, SpeedTree::RENDER_PASS_STANDARD);
    bool fronds = _forest_render.RenderFronds(_visible_trees, SpeedTree::RENDER_PASS_STANDARD);
    bool leaf_meshes = _forest_render.RenderLeafMeshes(_visible_trees, SpeedTree::RENDER_PASS_STANDARD);
    bool leaf_cards = _forest_render.RenderLeafCards(_visible_trees, SpeedTree::RENDER_PASS_STANDARD, _view);
    bool billboards = _forest_render.RenderBillboards(_visible_trees, SpeedTree::RENDER_PASS_STANDARD, _view);

    // Sometimes billboards comes back false, particularly if wind is
    // disabled; but the billboards appear to have been rendered successfully.
    // Weird.  Just removing this test from the condition.

    if (!branches || !fronds || !leaf_meshes || !leaf_cards /* || !billboards */) {
      speedtree_cat.warning()
        << "Failed to render forest completely: "
        << branches << " " << fronds << " " << leaf_meshes << " " << leaf_cards << " " << billboards << "\n";
      write_error(speedtree_cat.warning());
    }
  }

  _forest_render.EndRender();

  if (_forest_render.ShadowsAreEnabled() && speedtree_show_overlays) {
    _forest_render.RenderOverlays();
  }

  // SpeedTree leaves the graphics state indeterminate.  Make sure Panda
  // doesn't rely on anything in the state.
  geom_cbdata->set_lost_state(true);
}


/**
 * Renders the forest from the point of view of the light, to fill up the
 * shadow map(s).
 */
void SpeedTreeNode::
render_forest_into_shadow_maps() {
  bool success = true;

  // d3d10 allows A2C on render targets, so make sure to turn it off
  SpeedTree::CRenderState::SetMultisampling(false);
  SpeedTree::CRenderState::SetAlphaToCoverage(false);

#if defined(SPEEDTREE_OPENGL)
  // Ensure the viewport is not constrained.  SpeedTree doesn't expect that.
  glDisable(GL_SCISSOR_TEST);
#endif

  for (int smi = 0; smi < (int)_shadow_infos.size(); ++smi) {
    const SpeedTree::CView &light_view = _shadow_infos[smi]._light_view;
    const SpeedTree::SForestCullResults &light_cull = _shadow_infos[smi]._light_cull;

    if (_forest_render.BeginShadowMap(smi, light_view)) {
      success &= _forest_render.UploadViewShaderParameters(light_view);

      // branch geometry can be rendered with backfacing triangle removed, so
      // a closer tolerance can be used
      SpeedTree::CRenderState::SetPolygonOffset(1.0f, 0.125f);

      success &= _forest_render.RenderBranches(light_cull, SpeedTree::RENDER_PASS_SHADOW);

      // the remaining geometry types cannot be backface culled, so we need a
      // much more aggressive offset
      SpeedTree::CRenderState::SetPolygonOffset(10.0f, 1.0f);

      success &= _forest_render.RenderFronds(light_cull, SpeedTree::RENDER_PASS_SHADOW);
      success &= _forest_render.RenderLeafMeshes(light_cull, SpeedTree::RENDER_PASS_SHADOW);
      success &= _forest_render.RenderLeafCards(light_cull, SpeedTree::RENDER_PASS_SHADOW, light_view);

      // We don't bother to render billboard geometry into the shadow map(s).

      success &= _forest_render.EndShadowMap(smi);
    }
  }

  // SpeedTree::CRenderState::SetMultisampling(m_sUserSettings.m_nSampleCount
  // > 0);

  if (!success) {
    speedtree_cat.warning()
      << "Failed to render shadow maps\n";
    write_error(speedtree_cat.warning());
  }
}

/**
 * Does whatever calls are necessary to set up the forest for rendering--
 * create vbuffers, load shaders, and whatnot.  Primarily, this is the calls
 * to InitTreeGraphics and the like.
 */
void SpeedTreeNode::
setup_for_render(GraphicsStateGuardian *gsg) {
  if (!_done_first_init) {
    // This is the first time we have entered the draw callback since creating
    // any SpeedTreeNode.  Now we have an opportunity to do any initial setup
    // that requires a graphics context.

#ifdef SPEEDTREE_OPENGL
    // For OpenGL, we have to ensure GLEW has been initialized.  (SpeedTree
    // uses it, though Panda doesn't.)
    GLenum err = glewInit();
    if (err != GLEW_OK) {
      speedtree_cat.error()
        << "GLEW initialization failed: %s\n", glewGetErrorString(err);
      // Can't proceed without GLEW.
      _is_valid = false;
      return;
    }

    // Insist that OpenGL 2.0 is available as the SpeedTree renderer requires
    // it.
    if (!GLEW_VERSION_2_0) {
      speedtree_cat.error()
        << "The SpeedTree OpenGL implementation requires OpenGL 2.0 or better to run; this system has version " << glGetString(GL_VERSION) << "\n";
      _is_valid = false;
      return;
    }
#endif  // SPEEDTREE_OPENGL

    _done_first_init = true;
  }

#ifdef SPEEDTREE_DIRECTX9
  // In DirectX, we have to tell SpeedTree our device pointer.
  DXGraphicsStateGuardian9 *dxgsg = DCAST(DXGraphicsStateGuardian9, gsg);
  SpeedTree::DX9::SetDevice(dxgsg->_screen->_d3d_device);
#endif  // SPEEDTREE_DIRECTX9

  if (_needs_repopulate) {
    repopulate();

    // Now init per-tree graphics
    Trees::const_iterator ti;
    for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
      InstanceList *instance_list = (*ti);
      const STTree *tree = instance_list->get_tree();
      const STInstances &instances = instance_list->_instances;
      if (instances.empty()) {
        continue;
      }

      int max_instances = 2;
      SpeedTree::CMap<const SpeedTree::CTree*, SpeedTree::st_int32>::const_iterator si;
      si = _population_stats.m_mMaxNumInstancesPerCellPerBase.find(tree->get_tree());
      if (si != _population_stats.m_mMaxNumInstancesPerCellPerBase.end()) {
        max_instances = std::max(max_instances, (int)si->second);
      }

      // Get the speedtree-textures-dir to pass for initialization.
      string os_textures_dir;
      if (!speedtree_textures_dir.empty()) {
        os_textures_dir = speedtree_textures_dir.get_value().to_os_specific();
        // Ensure the path ends with a terminal slash; SpeedTree requires
        // this.
#if defined(WIN32) || defined(WIN64)
        if (!os_textures_dir.empty() && os_textures_dir[os_textures_dir.length() - 1] != '\\') {
          os_textures_dir += "\\";
        }
#else
        if (!os_textures_dir.empty() && os_textures_dir[os_textures_dir.length() - 1] != '/') {
          os_textures_dir += "/";
        }
#endif
      }

      if (!_forest_render.InitTreeGraphics((SpeedTree::CTreeRender *)tree->get_tree(),
                                           max_instances, speedtree_horizontal_billboards,
                                           os_textures_dir.c_str())) {
        if (speedtree_cat.is_debug()) {
          speedtree_cat.debug()
            << "Failed to init tree graphics for " << *tree << "\n";
          write_error(speedtree_cat.debug());
        }
      }
    }

    // Init overall graphics
    if (!_forest_render.InitGraphics(false)) {
      speedtree_cat.warning()
        << "Failed to init graphics\n";
      write_error(speedtree_cat.warning());
      _is_valid = false;
      return;
    }

    // This call apparently must be made at draw time, not earlier, because it
    // might attempt to create OpenGL index buffers and such.
    _forest_render.UpdateTreeCellExtents();

    if (has_terrain()) {
      // Now initialize the terrain.
      if (!_terrain_render.Init(speedtree_terrain_num_lods,
                                speedtree_terrain_resolution,
                                speedtree_terrain_cell_size,
                                _terrain->get_st_vertex_format())) {
        speedtree_cat.warning()
          << "Failed to init terrain\n";
        write_error(speedtree_cat.warning());
      }
    }

    // If we needed to repopulate, it means we didn't cull in the cull
    // traversal.  Do it now.
    cull_forest();
    _needs_repopulate = false;
  }
  if (has_terrain()) {
    PStatTimer timer1(_draw_speedtree_terrain_update_pcollector);
    update_terrain_cells();
  }
}

/**
 * Calls the SpeedTree methods to perform the needed cull calculations.
 */
void SpeedTreeNode::
cull_forest() {
  {
    PStatTimer timer1(_cull_speedtree_trees_pcollector);
    _forest_render.CullAndComputeLOD(_view, _visible_trees);
  }
  if (has_terrain()) {
    PStatTimer timer1(_cull_speedtree_terrain_pcollector);
    _terrain_render.CullAndComputeLOD(_view, _visible_terrain);
  }

  if (_forest_render.ShadowsAreEnabled()) {
    PStatTimer timer1(_cull_speedtree_shadows_pcollector);
    for (int smi = 0; smi < (int)_shadow_infos.size(); ++smi) {
      SpeedTree::CView &light_view = _shadow_infos[smi]._light_view;
      SpeedTree::SForestCullResultsRender &light_cull = _shadow_infos[smi]._light_cull;

      _forest_render.ComputeLightView
        (_forest_render.GetLightDir(), _view.GetFrustumPoints(), smi,
         light_view, 0.0f);

      light_view.SetLodRefPoint(_view.GetCameraPos());
      _forest_render.CullAndComputeLOD(light_view, light_cull, false);
    }
  }
}


/**
 *
 */
void SpeedTreeNode::
print_forest_stats(const SpeedTree::CForest::SPopulationStats &forest_stats) const {
  fprintf(stderr, "\n                Forest Population Statistics\n");
  fprintf(stderr, "   ---------------------------------------------------\n");
  fprintf(stderr, "                    # of tree cull cells: %d\n", forest_stats.m_nNumCells);
  fprintf(stderr, "                  # of unique base trees: %d\n", forest_stats.m_nNumBaseTrees);
  fprintf(stderr, "                    total # of instances: %d\n", forest_stats.m_nNumInstances);
  fprintf(stderr, "         average # of instances per base: %g\n", forest_stats.m_fAverageNumInstancesPerBase);
  fprintf(stderr, "  max # of billboards/instances per cell: %d\n", forest_stats.m_nMaxNumBillboardsPerCell);
  fprintf(stderr, "    max # of instances per cell per base:\n");
  SpeedTree::CMap<const SpeedTree::CTree*, SpeedTree::st_int32>::const_iterator i;
  for (i = forest_stats.m_mMaxNumInstancesPerCellPerBase.begin( ); i != forest_stats.m_mMaxNumInstancesPerCellPerBase.end( ); ++i) {
    fprintf(stderr, "        %35s: %4d\n", SpeedTree::CFixedString(i->first->GetFilename( )).NoPath( ).c_str( ), i->second);
  }
  fprintf(stderr, "            average # instances per cell: %g\n", forest_stats.m_fAverageInstancesPerCell);
  fprintf(stderr, "               max # of billboard images: %d\n", forest_stats.m_nMaxNumBillboardImages);
  fprintf(stderr, "\n");
}

/**
 * Tells the BamReader how to create objects of type SpeedTreeNode.
 */
void SpeedTreeNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void SpeedTreeNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);

  int num_trees = _trees.size();
  dg.add_uint32(num_trees);
  Trees::const_iterator ti;
  for (ti = _trees.begin(); ti != _trees.end(); ++ti) {
    InstanceList *instance_list = (*ti);
    instance_list->write_datagram(manager, dg);
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type SpeedTreeNode is encountered in the Bam file.  It should create the
 * SpeedTreeNode and extract its information from the file.
 */
TypedWritable *SpeedTreeNode::
make_from_bam(const FactoryParams &params) {
  SpeedTreeNode *node = new SpeedTreeNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new SpeedTreeNode.
 */
void SpeedTreeNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);

  int num_trees = scan.get_uint32();
  _trees.reserve(num_trees);
  for (int i = 0; i < num_trees; i++) {
    InstanceList *instance_list = new InstanceList(nullptr);
    instance_list->fillin(scan, manager);
    if (instance_list->get_tree() == nullptr) {
      // The tree wasn't successfully loaded.  Don't keep it.
      delete instance_list;
    } else {
      _trees.push_back(instance_list);
    }
  }

  _trees.sort();
}

/**
 *
 */
void SpeedTreeNode::InstanceList::
output(ostream &out) const {
  out << *_tree << ": " << _instances.size() << " instances";
}

/**
 *
 */
void SpeedTreeNode::InstanceList::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << *_tree << ": " << _instances.size() << " instances.\n";
  STInstances::const_iterator ii;
  for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
    indent(out, indent_level + 2)
      << STTransform(*ii) << "\n";
  }
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void SpeedTreeNode::InstanceList::
write_datagram(BamWriter *manager, Datagram &dg) {
  // Compute the relative pathname to the SRT file.
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  bool has_bam_dir = !manager->get_filename().empty();
  Filename bam_dir = manager->get_filename().get_dirname();
  Filename srt_filename = _tree->get_fullpath();

  bam_dir.make_absolute(vfs->get_cwd());
  if (!has_bam_dir || !srt_filename.make_relative_to(bam_dir, true)) {
    srt_filename.find_on_searchpath(get_model_path());
  }

  dg.add_string(srt_filename);

  // Now record the instances.
  int num_instances = _instances.size();
  dg.add_uint32(num_instances);
  STInstances::const_iterator ii;
  for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
    STTransform transform = (*ii);
    transform.write_datagram(manager, dg);
  }
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new SpeedTreeNode.
 */
void SpeedTreeNode::InstanceList::
fillin(DatagramIterator &scan, BamReader *manager) {
  // Get the relative pathname to the SRT file.
  string srt_filename = scan.get_string();

  // Now load up the SRT file using the Panda loader (which will also search
  // the model-path if necessary).
  Loader *loader = Loader::get_global_ptr();
  PT(PandaNode) srt_root = loader->load_sync(srt_filename);

  if (srt_root != nullptr) {
    NodePath srt(srt_root);
    NodePath srt_np = srt.find("**/+SpeedTreeNode");
    if (!srt_np.is_empty()) {
      SpeedTreeNode *srt_node = DCAST(SpeedTreeNode, srt_np.node());
      if (srt_node->get_num_trees() >= 1) {
        _tree = (STTree *)srt_node->get_tree(0);
      }
    }
  }

  // Now read the instances.
  int num_instances = scan.get_uint32();
  _instances.reserve(num_instances);
  for (int i = 0; i < num_instances; i++) {
    STTransform transform;
    transform.fillin(scan, manager);
    _instances.push_back(transform);
  }
}

/**
 * This method called when the callback is triggered; it *replaces* the
 * original function.  To continue performing the original function, you must
 * call cbdata->upcall() during the callback.
 */
void SpeedTreeNode::DrawCallback::
do_callback(CallbackData *data) {
  _node->draw_callback(data);
}
