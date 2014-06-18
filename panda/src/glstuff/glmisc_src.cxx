// Filename: glmisc_src.cxx
// Created by:  drose (09Feb04)
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

#include "pandaSystem.h"

ConfigVariableBool gl_cheap_textures
  ("gl-cheap-textures", false,
   PRC_DESC("Configure this true to glHint the textures into the cheapest "
            "possible mode."));

ConfigVariableBool gl_ignore_clamp
  ("gl-ignore-clamp", false,
   PRC_DESC("Configure this true to disable texture clamp mode (all textures "
            "repeat, a little cheaper for software renderers)."));

ConfigVariableBool gl_support_clamp_to_border
  ("gl-support-clamp-to-border", true,
   PRC_DESC("Configure this true to enable the use of the clamp_to_border "
            "extension if the GL claims to support it, or false not to "
            "use it even if it appears to be available.  (On some OpenGL "
            "drivers, enabling this mode can force software rendering.)"));

ConfigVariableBool gl_support_rescale_normal
  ("gl-support-rescale-normal", true,
   PRC_DESC("Configure this true to enable the use of the rescale_normal "
            "extension if the GL claims to support it, or false not to use "
            "it even if it appears to be available.  (This appears to be "
            "buggy on some drivers.)"));

ConfigVariableBool gl_ignore_filters
  ("gl-ignore-filters", false,
   PRC_DESC("Configure this true to disable any texture filters at all (forcing "
            "point sampling)."));

ConfigVariableBool gl_ignore_mipmaps
  ("gl-ignore-mipmaps", false,
   PRC_DESC("Configure this true to disable mipmapping only."));

ConfigVariableBool gl_force_mipmaps
  ("gl-force-mipmaps", false,
   PRC_DESC("Configure this true to enable full trilinear mipmapping on every "
            "texture, whether it asks for it or not."));

ConfigVariableBool gl_show_texture_usage
  ("gl-show-texture-usage", false,
   PRC_DESC("If you set this true, the screen will flash with textures drawn "
            "in a special mode that shows the mipmap detail level and texture "
            "size for each texture.  Textures will be drawn in blue for "
            "mipmap level 0, yellow for mipmap level 1, and red for all higher "
            "mipmap levels.  Brighter colors represent larger textures."));

ConfigVariableInt gl_show_texture_usage_max_size
  ("gl-show-texture-usage-max-size", 1024,
   PRC_DESC("Specifies the texture size (along one side) of the largest "
            "texture expected to be loaded.  This controls the assignment "
            "of the texture color in gl-show-texture-usage mode; colors "
            "will be fully bright for textures of this size or larger."));

ConfigVariableBool gl_color_mask
  ("gl-color-mask", true,
   PRC_DESC("Configure this false if your GL's implementation of glColorMask() "
            "is broken (some are).  This will force the use of a (presumably) "
            "more expensive blending operation instead."));

ConfigVariableBool gl_support_occlusion_query
  ("gl-support-occlusion-query", true,
   PRC_DESC("Configure this true to enable the use of the occlusion_query "
            "extension if the GL claims to support it, or false not to "
            "use it even if it appears to be available.  (On some OpenGL "
            "drivers, enabling this mode can force software rendering.)"));

ConfigVariableBool gl_compile_and_execute
  ("gl-compile-and-execute", false,
   PRC_DESC("Configure this true if you know your GL's implementation of "
            "glNewList(n, GL_COMPILE_AND_EXECUTE) works.  It is "
            "false by default, since it is known to cause a crash with "
            "Intel 855GM driver 4.14.10.3889 at least.  Turning this on "
            "*may* reduce the chug you get for preparing display lists "
            "for the first time, by allowing the display list to be "
            "rendered at the same time it is being compiled."));

ConfigVariableBool gl_interleaved_arrays
  ("gl-interleaved-arrays", false,
   PRC_DESC("Set this true to convert OpenGL geometry such that the "
            "primary data columns vertex, normal, color, and texcoord "
            "are interleaved into one array when possible, or false to "
            "render geometry as it appears in the GeomVertexData.  See "
            "also gl-parallel-arrays."));

ConfigVariableBool gl_parallel_arrays
  ("gl-parallel-arrays", false,
   PRC_DESC("Set this true to convert OpenGL geometry such that each "
            "data column is a separate array, or false to "
            "render geometry as it appears in the GeomVertexData.  See "
            "also gl-interleaved-arrays."));

ConfigVariableInt gl_max_errors
  ("gl-max-errors", 20,
   PRC_DESC("This is the limit on the number of OpenGL errors Panda will "
            "detect and report before it shuts down rendering.  Set it to "
            "-1 for no limit."));

ConfigVariableEnum<GeomEnums::UsageHint> gl_min_buffer_usage_hint
  ("gl-min-buffer-usage-hint", GeomEnums::UH_stream,
   PRC_DESC("This specifies the first usage hint value that will be "
            "loaded as a vertex buffer, instead of directly from the "
            "client.  Normally, this should be \"stream\", which means "
            "to load the vertex buffer using GL_STREAM_DRAW.  If this "
            "is set to \"dynamic\", or \"static\", then only usage hints "
            "at that level or higher will be loaded as a vertex buffer, "
            "and stream or lower will be rendered directly from the "
            "client array.  If changing this results in a remarkable "
            "performance improvement, you may have code that is "
            "creating and destroying vertex buffers every frame, instead "
            "of reusing the same buffers.  Consider increasing "
            "released-vbuffer-cache-size instead."));

ConfigVariableBool gl_debug_buffers
  ("gl-debug-buffers", false,
   PRC_DESC("Set this true, in addition to enabling debug notify for "
            "glgsg, to enable debug messages about the creation and "
            "destruction of OpenGL vertex buffers."));

ConfigVariableBool gl_finish
  ("gl-finish", false,
   PRC_DESC("Set this true to force a call to glFinish() after every major "
            "graphics operation.  This is likely to slow down rendering "
            "performance substantially, but it will make PStats graphs "
            "more accurately reflect where the graphics bottlenecks are.  "
            "This variable is enabled only if PStats is compiled in."));

ConfigVariableBool gl_force_depth_stencil
  ("gl-force-depth-stencil", false, 
   PRC_DESC("Temporary hack variable 7x00 vs 8x00 nVidia bug.  See glGraphicsStateGuardian_src.cxx."));

ConfigVariableBool gl_matrix_palette
  ("gl-matrix-palette", false, 
   PRC_DESC("Temporary hack variable protecting untested code.  See glGraphicsStateGuardian_src.cxx."));

ConfigVariableBool gl_force_no_error
  ("gl-force-no-error", false,
   PRC_DESC("Avoid reporting OpenGL errors, for a small performance benefit."));

ConfigVariableBool gl_force_no_flush
  ("gl-force-no-flush", false, 
   PRC_DESC("Avoid calling glFlush(), for a potential performance benefit.  This may be a little dangerous."));

ConfigVariableBool gl_separate_specular_color
  ("gl-separate-specular-color", true, 
   PRC_DESC("When separate specular mode is on, the specular component "
            "will be written to the secondary instead of the primary "
            "color, which is added after the texturing stage.  In other "
            "words, the specular highlight will be unmodulated by the "
            "color of the texture."));

extern ConfigVariableBool gl_parallel_arrays;

void CLP(init_classes)() {
  CLP(GeomContext)::init_type();
  CLP(GeomMunger)::init_type();
  CLP(GraphicsStateGuardian)::init_type();
  CLP(IndexBufferContext)::init_type();
#ifndef OPENGLES_1
  CLP(ShaderContext)::init_type();
#endif
  CLP(TextureContext)::init_type();
  CLP(VertexBufferContext)::init_type();
  CLP(GraphicsBuffer)::init_type();

#ifndef OPENGLES
  CLP(OcclusionQueryContext)::init_type();
#endif

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system(GLSYSTEM_NAME);

  // We can't add any tags defining the available OpenGL capabilities,
  // since we won't know those until we create a graphics context (and
  // the answer may be different for different contexts).
}

