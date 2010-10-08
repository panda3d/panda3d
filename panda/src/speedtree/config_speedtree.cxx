// Filename: config_speedtree.cxx
// Created by:  drose (30Sep10)
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

#include "config_speedtree.h"
#include "speedTreeNode.h"
#include "stTree.h"
#include "loaderFileTypeSrt.h"
#include "loaderFileTypeStf.h"
#include "loaderFileTypeRegistry.h"
#include "dconfig.h"

Configure(config_speedtree);
NotifyCategoryDef(speedtree, "");

ConfigureFn(config_speedtree) {
  init_libspeedtree();
}

ConfigVariableString speedtree_license
("speedtree-license", "", 
 PRC_DESC("Specify the license string to pass to SpeedTreeNode::authorize() by default."));

ConfigVariableFilename speedtree_shaders_dir
("speedtree-shaders-dir", Filename(Filename::from_os_specific(SPEEDTREE_BIN_DIR), "Shaders"),
 PRC_DESC("Specifies the directory in which to locate SpeedTree's system "
	  "shaders at runtime.  If this is empty, the default is based on "
	  "SPEEDTREE_BIN_DIR, as provided at compile time."));

ConfigVariableBool speedtree_allow_horizontal_billboards
("speedtree-allow-horizontal-billboards", true,
 PRC_DESC("Set this true to allow the use of horizontal billboards in "
	  "SpeedTree, or false to disallow them.  Documentation on this "
	  "feature is sparse, but presumably enabling them increases "
	  "visual quality and also causes a greater performance impact."));

ConfigVariableInt speedtree_max_num_visible_cells
("speedtree-max-num-visible-cells", 75,
 PRC_DESC("Specifies the maximum number of cells in a single SpeedTree forest "
	  "frustum.  This is used internally by SpeedTree's billboard system."));

ConfigVariableInt speedtree_max_billboard_images_by_base
("speedtree-max-billboard-images-by-base", 20,
 PRC_DESC("Specifies the maximum number of billboard images used by any single "
	  "tree."));

ConfigVariableDouble speedtree_cull_cell_size
("speedtree-cull-cell-size", 1200,
 PRC_DESC("Specifies the size of a single SpeedTree cull cell, in Panda "
	  "units.  Increasing this number decreases the number of "
	  "individual calls that must be made to render geometry, "
	  "while increasing the number of trees that are rendered "
	  "per call."));

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
 

////////////////////////////////////////////////////////////////////
//     Function: init_libspeedtree
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libspeedtree() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  SpeedTreeNode::init_type();
  STTree::init_type();
  LoaderFileTypeSrt::init_type();
  LoaderFileTypeStf::init_type();

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_global_ptr();
  reg->register_type(new LoaderFileTypeSrt);
  reg->register_type(new LoaderFileTypeStf);
}

// We need a SpeedTree custom allocator to integrate with Panda's
// memory management.
class STCustomAllocator : public SpeedTree::CAllocator {
public:
  void *Alloc(size_t block_size) {
    return PANDA_MALLOC_ARRAY(block_size);
  }
  
  void Free(void *block) {
    if (block != NULL) {
      PANDA_FREE_ARRAY(block);
    }
  }
};

// Hook our custom allocator into SpeedTree.
#ifndef CPPPARSER
static STCustomAllocator custom_allocator;
static SpeedTree::CAllocatorInterface allocator_interface(&custom_allocator);
#endif // CPPPARSER
