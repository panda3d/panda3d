// Filename: glmisc_src.cxx
// Created by:  drose (09Feb04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pandaSystem.h"

ConfigVariableBool CLP(cheap_textures)
  ("gl-cheap-textures", false,
   PRC_DESC("Configure this true to GLP(Hint) the textures into the cheapest "
            "possible mode."));

ConfigVariableBool CLP(ignore_clamp)
  ("gl-ignore-clamp", false,
   PRC_DESC("Configure this true to disable texture clamp mode (all textures "
            "repeat, a little cheaper for software renderers)."));

ConfigVariableBool CLP(ignore_filters)
  ("gl-ignore-filters", false,
   PRC_DESC("Configure this true to disable any texture filters at all (forcing "
            "point sampling)."));

ConfigVariableBool CLP(ignore_mipmaps)
  ("gl-ignore-mipmaps", false,
   PRC_DESC("Configure this true to disable mipmapping only."));

ConfigVariableBool CLP(force_mipmaps)
  ("gl-force-mipmaps", false,
   PRC_DESC("Configure this true to enable full trilinear mipmapping on every "
            "texture, whether it asks for it or not."));

ConfigVariableBool CLP(show_mipmaps)
  ("gl-show-mipmaps", false,
   PRC_DESC("Configure this true to cause mipmaps to be rendered with phony "
            "colors, using mipmap_level_*.rgb if they are available."));

ConfigVariableBool CLP(save_mipmaps)
  ("gl-save-mipmaps", false,
   PRC_DESC("Configure this true to cause the generated mipmap images to be "
            "written out to image files on the disk as they are generated."));

ConfigVariableBool CLP(color_mask)
  ("gl-color-mask", true,
   PRC_DESC("Configure this false if your GL's implementation of glColorMask() "
            "is broken (some are).  This will force the use of a (presumably) "
            "more expensive blending operation instead."));


void CLP(init_classes)() {
  CLP(GraphicsStateGuardian)::init_type();
  CLP(TextureContext)::init_type();
  CLP(GeomContext)::init_type();
  CLP(DataContext)::init_type();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system(GLSYSTEM_NAME);

  // We can't add any tags defining the available OpenGL capabilities,
  // since we won't know those until we create a graphics context (and
  // the answer may be different for different contexts).
}

