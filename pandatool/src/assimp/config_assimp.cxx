/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_assimp.cxx
 * @author rdb
 * @date 2011-03-29
 */

#include "config_assimp.h"

#include "loaderFileTypeAssimp.h"

#include "dconfig.h"
#include "loaderFileTypeRegistry.h"

ConfigureDef(config_assimp);
NotifyCategoryDef(assimp, "");

ConfigureFn(config_assimp) {
  init_libassimp();
}

ConfigVariableBool assimp_calc_tangent_space
("assimp-calc-tangent-space", false,
 PRC_DESC("Calculates tangents and binormals for meshes imported via Assimp."));

ConfigVariableBool assimp_join_identical_vertices
("assimp-join-identical-vertices", true,
 PRC_DESC("Merges duplicate vertices.  Set this to false if you want each "
          "vertex to only be in use on one triangle."));

ConfigVariableBool assimp_improve_cache_locality
("assimp-improve-cache-locality", true,
 PRC_DESC("Improves rendering performance of the loaded meshes by reordering "
          "triangles for better vertex cache locality.  Set this to false if "
          "you need geometry to be loaded in the exact order that it was "
          "specified in the file, or to improve load performance."));

ConfigVariableBool assimp_remove_redundant_materials
("assimp-remove-redundant-materials", true,
 PRC_DESC("Removes redundant/unreferenced materials from assets."));

ConfigVariableBool assimp_fix_infacing_normals
("assimp-fix-infacing-normals", false,
 PRC_DESC("Determines which normal vectors are facing inward and inverts them "
          "so that they are facing outward."));

ConfigVariableBool assimp_optimize_meshes
("assimp-optimize-meshes", true,
 PRC_DESC("Removes the number of draw calls by unifying geometry with the same "
          "materials.  Especially effective in conjunction with "
          "assimp-optimize-graph and assimp-remove-redundant-materials."));

ConfigVariableBool assimp_optimize_graph
("assimp-optimize-graph", false,
 PRC_DESC("Optimizes the scene geometry by flattening the scene hierarchy.  "
          "This is very efficient (combined with assimp-optimize-meshes), but "
          "it may result the hierarchy to become lost, so it is disabled by "
          "default."));

ConfigVariableBool assimp_flip_winding_order
("assimp-flip-winding-order", false,
 PRC_DESC("Set this true to flip the winding order of all models loaded via "
          "the Assimp loader.  Note that you may need to clear the model-cache "
          "after changing this."));

ConfigVariableBool assimp_gen_normals
("assimp-gen-normals", false,
 PRC_DESC("Set this true to generate normals (if absent from file) on import. "
          "See assimp-smooth-normal-angle for more information. "
          "Note that you may need to clear the model-cache after "
          "changing this."));

ConfigVariableDouble assimp_smooth_normal_angle
("assimp-smooth-normal-angle", 0.0,
 PRC_DESC("Set this to anything other than 0.0 in degrees (so 180.0 is PI) to "
          "specify the maximum angle that may be between two face normals at "
          "the same vertex position that are smoothed together. Sometimes "
          "referred to as 'crease angle'. Only has effect if "
          "assimp-gen-normals is set to true and the file does not contain "
          "normals. Note that you may need to clear the model-cache after "
          "changing this."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libassimp() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  LoaderFileTypeAssimp::init_type();

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_global_ptr();
  reg->register_type(new LoaderFileTypeAssimp);
}
