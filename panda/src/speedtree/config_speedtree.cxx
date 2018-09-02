/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_speedtree.cxx
 * @author drose
 * @date 2010-09-30
 */

#include "config_speedtree.h"
#include "speedTreeNode.h"
#include "stBasicTerrain.h"
#include "stTerrain.h"
#include "stTree.h"
#include "loaderFileTypeSrt.h"
#include "loaderFileTypeStf.h"
#include "loaderFileTypeRegistry.h"
#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDASPEEDTREE)
  #error Buildsystem error: BUILDING_PANDASPEEDTREE not defined
#endif

ConfigureDef(config_speedtree);
NotifyCategoryDef(speedtree, "");

ConfigureFn(config_speedtree) {
  init_libspeedtree();
}

ConfigVariableString speedtree_license
("speedtree-license", "",
 PRC_DESC("Specify the license string to pass to SpeedTreeNode::authorize() by default."));

#ifndef CPPPARSER
ConfigVariableFilename speedtree_shaders_dir
("speedtree-shaders-dir", Filename(Filename::from_os_specific(SPEEDTREE_BIN_DIR), "Shaders"),
 PRC_DESC("Specifies the directory in which to locate SpeedTree's system "
          "shaders at runtime.  If this is empty, the default is based on "
          "SPEEDTREE_BIN_DIR, as provided at compile time."));
#endif  // CPPPARSER

ConfigVariableFilename speedtree_textures_dir
("speedtree-textures-dir", "",
 PRC_DESC("Specifies the directory in which to locate textures referenced "
          "by SRT files at runtime.  The default is to search in the same "
          "directory as the SRT file itself.  Unfortunately, the model-path "
          "cannot be searched, because SpeedTree only provides a single "
          "directory option."));

ConfigVariableDouble speedtree_max_anisotropy
("speedtree-max-anisotropy", 0.0,
 PRC_DESC("Specifies the maximum anisotropy for SpeedTree textures."));

ConfigVariableBool speedtree_horizontal_billboards
("speedtree-horizontal-billboards", true,
 PRC_DESC("Set this true to allow the use of horizontal billboards in "
          "SpeedTree, or false to disallow them.  Horizontal billboards "
          "may be defined for some trees to provide a billboard LOD "
          "when the tree is seen from above."));

ConfigVariableDouble speedtree_alpha_test_scalar
("speedtree-alpha-test-scalar", 0.57,
 PRC_DESC("Undocumented speedtree config."));

ConfigVariableBool speedtree_z_pre_pass
("speedtree-z-pre-pass", false,
 PRC_DESC("True if the SpeedTree renderer should perform a first pass "
          "to fill the depth buffer before going back to draw pixels.  "
          "This can result in a cost savings if there is much overdraw and "
          "if the pixel shader is particularly expensive, but in most cases "
          "it will result in a cost penalty."));

ConfigVariableInt speedtree_max_billboard_images_by_base
("speedtree-max-billboard-images-by-base", 20,
 PRC_DESC("Specifies the maximum number of billboard images used by any single "
          "tree."));

ConfigVariableDouble speedtree_visibility
("speedtree-visibility", 1000.0,
 PRC_DESC("Undocumented SpeedTree parameter."));

ConfigVariableDouble speedtree_global_light_scalar
("speedtree-global-light-scalar", 1.0,
 PRC_DESC("Undocumented SpeedTree parameter."));

ConfigVariableBool speedtree_specular_lighting
("speedtree-specular-lighting", false,
 PRC_DESC("True to enable specular lighting effects in SpeedTree."));

ConfigVariableBool speedtree_transmission_lighting
("speedtree-transmission-lighting", false,
 PRC_DESC("True to enable transmission lighting effects in SpeedTree."));

ConfigVariableBool speedtree_detail_layer
("speedtree-detail-layer", false,
 PRC_DESC("True to render the detail texture layer defined on tree and "
          "terrain objects, false otherwise."));

ConfigVariableBool speedtree_detail_normal_mapping
("speedtree-detail-normal-mapping", false,
 PRC_DESC("True to render the detail normal maps defined on tree and "
          "terrain objects, false otherwise."));

ConfigVariableBool speedtree_ambient_contrast
("speedtree-ambient-contrast", false,
 PRC_DESC("Undocumented SpeedTree parameter."));

ConfigVariableDouble speedtree_transmission_scalar
("speedtree-transmission-scalar", 1.0f,
 PRC_DESC("Undocumented SpeedTree parameter."));

ConfigVariableDouble speedtree_fog_distance
("speedtree-fog-distance", "2500 5000",
 PRC_DESC("Specifies the nearest and farthest distance of the fog on trees "
          "and terrain."));

ConfigVariableColor speedtree_fog_color
("speedtree-fog-color", "1.0 1.0 1.0",
 PRC_DESC("Specifies the r g b color of SpeedTree fog."));

ConfigVariableColor speedtree_sky_color
("speedtree-sky-color", "0.2 0.3 0.5",
 PRC_DESC("Specifies the r g b color of the SpeedTree sky, when the sky "
          "is enabled.  Currently unused."));

ConfigVariableDouble speedtree_sky_fog
("speedtree-sky-fog", "-0.5 1.0",
 PRC_DESC("Specifies the range of fog in the sky.  -1 is down, 1 is up."));

ConfigVariableColor speedtree_sun_color
("speedtree-sun-color", "1.0 1.0 0.85",
 PRC_DESC("Specifies the r g b color of the SpeedTree sun, when the sun "
          "is enabled.  Currently unused."));

ConfigVariableDouble speedtree_sun_size
("speedtree-sun-size", 0.001,
 PRC_DESC("Undocumented SpeedTree parameter."));

ConfigVariableDouble speedtree_sun_spread_exponent
("speedtree-sun-spread-exponent", 200.0,
 PRC_DESC("Undocumented SpeedTree parameter."));

ConfigVariableDouble speedtree_sun_fog_bloom
("speedtree-sun-fog-bloom", 0.0,
 PRC_DESC("Undocumented SpeedTree parameter."));

ConfigVariableColor speedtree_specular_color
("speedtree-specular-color", "1 1 1",
 PRC_DESC("Specifies the r g b color of the specular reflections on SpeedTree "
          "surfaces."));

ConfigVariableColor speedtree_emissive_color
("speedtree-emissive-color", "0 0 0",
 PRC_DESC("Specifies the r g b color of the emissive light effect on SpeedTree "
          "surfaces."));

ConfigVariableInt speedtree_shadow_map_resolution
("speedtree-shadow-map-resolution", 0, //1024,
 PRC_DESC("Specifies the resolution for rendering shadow maps.  Should "
          "be a power of 2.  Specify 0 to disable shadowing in SpeedTree."));

ConfigVariableDouble speedtree_cascading_shadow_splits
("speedtree-cascading-shadow-splits", "200 400 600",
 PRC_DESC("Specifies the shadow split distance, in spatial units, for "
          "each shadow map to be rendered.  The number of values also "
          "implies the number of shadow maps, to a maximum value compiled "
          "within SpeedTree (typically 4)."));

ConfigVariableBool speedtree_smooth_shadows
("speedtree-smooth-shadows", false,
 PRC_DESC("True to enable a smoothing pass on the shadow maps."));

ConfigVariableBool speedtree_show_shadow_splits_on_terrain
("speedtree-show-shadow-splits-on-terrain", false,
 PRC_DESC("Currently unsupported."));

ConfigVariableBool speedtree_wind_enabled
("speedtree-wind-enabled", true,
 PRC_DESC("True to enable global wind in the SpeedTree world."));

ConfigVariableBool speedtree_frond_rippling
("speedtree-frond-rippling", true,
 PRC_DESC("True to allow fronds to respond to the global wind."));

ConfigVariableInt speedtree_terrain_num_lods
("speedtree-terrain-num-lods", 5,
 PRC_DESC("Specifies the number of different LOD stages to use in "
          "rendering terrain.  Each stage has 75% few polygons than the "
          "stage before.  Increasing this number decreases the total "
          "number of polygons onscreen, at the cost of visual quality of "
          "distant terrain."));

ConfigVariableInt speedtree_terrain_resolution
("speedtree-terrain-resolution", 33,
 PRC_DESC("Specifies the number of vertices per edge of each terrain grid "
          "cell at the highest LOD.  This must be a power of two plus 1."));

ConfigVariableInt speedtree_terrain_cell_size
("speedtree-terrain-cell-size", 800,
 PRC_DESC("Specifies the size in spatial units of one edge of a terrain "
          "grid cell.  This effects culling and resolution, but has nothing "
          "to do with the scale of the heightmap or other maps."));

ConfigVariableDouble speedtree_shadow_fade
("speedtree-shadow-fade", 0.25,
 PRC_DESC("Specifies the rate at which shadows fade to transparency in the "
          "distance."));

ConfigVariableBool speedtree_show_overlays
("speedtree-show-overlays", false,
 PRC_DESC("True to draw onscreen overlays showing the generated "
          "shadow map(s)."));

ConfigVariableInt speedtree_max_num_visible_cells
("speedtree-max-num-visible-cells", 75,
 PRC_DESC("Specifies the maximum number of cells in a single SpeedTree forest "
          "frustum.  This is used internally by SpeedTree's billboard system."));

ConfigVariableDouble speedtree_cull_cell_size
("speedtree-cull-cell-size", 1200,
 PRC_DESC("Specifies the size of a single SpeedTree cull cell, in Panda "
          "units.  Increasing this number decreases the number of "
          "individual calls that must be made to render geometry, "
          "while increasing the number of trees that are rendered "
          "per call."));

ConfigVariableDouble speedtree_area_scale
("speedtree-area-scale", 3280.839895013,
 PRC_DESC("This is the factor by which the \"area\" value in a SpeedTree "
          "terrain.txt file is scaled (after taking its square root) "
          "when loading a new terrain.  The default is about 3281, which "
          "scales from kilometers to feet.  You should set a different "
          "scale if you use units other than feet."));

ConfigVariableBool speedtree_follow_terrain
("speedtree-follow-terrain", true,
 PRC_DESC("Set this true to have trees automatically snap to the terrain "
          "height when loaded into a SpeedTree node with a configured "
          "terrain.  If this is false, you may still call "
          "SpeedTreeNode::snap_to_terrain() afterwards."));

ConfigVariableInt speedtree_max_random_try_count
("speedtree-max-random-try-count", 1000,
 PRC_DESC("This is a cheesy limit to detect bad parameters passed to "
          "SpeedTreeNode::add_random_instances().  If this number of attempts "
          "to find a legal place for a tree fail in a row, the parameters "
          "are deemed to be in error, and the function fails."));

ConfigVariableBool speedtree_5_2_stf
("speedtree-5-2-stf",
#if SPEEDTREE_VERSION_MAJOR > 5 || (SPEEDTREE_VERSION_MAJOR == 5 && SPEEDTREE_VERSION_MINOR >= 2)
 true,
#else
 false,
#endif
 PRC_DESC("The format of the STF file changed in SpeedTree version 5.2.  "
          "Specify true here to read STF files in the new file format, or "
          "false to read STF files in the pre-5.2 file format."));


/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libspeedtree() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  SpeedTreeNode::init_type();
  STBasicTerrain::init_type();
  STTerrain::init_type();
  STTree::init_type();
  LoaderFileTypeSrt::init_type();
  LoaderFileTypeStf::init_type();

  SpeedTreeNode::register_with_read_factory();

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_global_ptr();
  reg->register_type(new LoaderFileTypeSrt);
  reg->register_type(new LoaderFileTypeStf);
}

// We need a SpeedTree custom allocator to integrate with Panda's memory
// management.
class STCustomAllocator : public SpeedTree::CAllocator {
public:
  void *Alloc(size_t block_size) {
    return PANDA_MALLOC_ARRAY(block_size);
  }

  void Free(void *block) {
    if (block != nullptr) {
      PANDA_FREE_ARRAY(block);
    }
  }
};

// Hook our custom allocator into SpeedTree.
#ifndef CPPPARSER
static STCustomAllocator custom_allocator;
static SpeedTree::CAllocatorInterface allocator_interface(&custom_allocator);
#endif // CPPPARSER
