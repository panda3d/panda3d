/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glmisc_src.cxx
 * @author drose
 * @date 2004-02-09
 */

#include "pandaSystem.h"

ConfigVariableInt gl_version
  ("gl-version", "",
   PRC_DESC("Set this to get an OpenGL context with a specific version."));

ConfigVariableBool gl_forward_compatible
  ("gl-forward-compatible", false,
   PRC_DESC("Setting this to true will request a forward-compatible OpenGL "
            "context, which will not support the fixed-function pipeline."));

ConfigVariableBool gl_support_fbo
  ("gl-support-fbo", true,
   PRC_DESC("Configure this false if your GL's implementation of "
            "EXT_framebuffer_object is broken.  The system might still be "
            "able to create buffers using pbuffers or the like."));

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

ConfigVariableBool gl_support_texture_lod
  ("gl-support-texture-lod", true,
   PRC_DESC("Configure this true to enable the use of minmax LOD settings "
            "and texture LOD bias settings.  Set this to false if you "
            "suspect a driver bug."));

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

ConfigVariableBool gl_debug
  ("gl-debug", false,
   PRC_DESC("Setting this to true will cause OpenGL to emit more useful "
            "error and debug messages, at a slight runtime performance cost.  "
            "notify-level-glgsg controls which severity levels are shown."));

ConfigVariableBool gl_debug_synchronous
  ("gl-debug-synchronous", false,
   PRC_DESC("Set this true to make sure that the errors generated by "
            "gl-debug are reported as soon as they happen.  This is "
            "highly recommended if you want to attach a debugger since "
            "the call stack may otherwise not point to the GL call "
            "where the error originated."));

ConfigVariableEnum<NotifySeverity> gl_debug_abort_level
  ("gl-debug-abort-level", NS_fatal,
   PRC_DESC("Set this to a setting other than 'fatal' to cause an "
            "abort to be triggered when an error of the indicated "
            "severity level (or a more severe one) occurs.  This is "
            "useful if you want to attach a debugger.  If you set this, "
            "it is highly recommended to also set gl-debug-synchronous, "
            "since the call stack will otherwise not point to the GL call "
            "that triggered the error message.  "
            "This feature is not available when NDEBUG has been defined."));

ConfigVariableBool gl_debug_object_labels
  ("gl-debug-object-labels", true,
   PRC_DESC("When gl-debug is set to true, this will tell OpenGL the "
            "name of textures, shaders, and other objects, so that OpenGL "
            "can display those in error messages.  There's usually no "
            "reason to disable this."));

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
            "more accurately reflect where the graphics bottlenecks are, "
            "although it is better to use timer queries when available. "
            "This variable is enabled only if PStats is compiled in."));

ConfigVariableBool gl_force_depth_stencil
  ("gl-force-depth-stencil", false,
   PRC_DESC("Temporary hack variable 7x00 vs 8x00 nVidia bug.  See glGraphicsStateGuardian_src.cxx."));

ConfigVariableBool gl_force_fbo_color
  ("gl-force-fbo-color", true,
   PRC_DESC("This is set to true to force all FBOs to have at least one "
            "color attachment.  This is to work around an Intel driver "
            "issue.  Set to false to allow depth-only FBOs."));

ConfigVariableBool gl_check_errors
  ("gl-check-errors", false,
   PRC_DESC("Regularly call glGetError() to check for OpenGL errors.  "
            "This will slow down rendering significantly.  If your "
            "video driver supports it, you should use gl-debug instead."));

ConfigVariableBool gl_force_flush
  ("gl-force-flush", false,
   PRC_DESC("Call this to force a call to glFlush() after rendering a "
            "frame, even when using a double-buffered framebuffer.  "
            "This can incur a significant performance penalty."));

ConfigVariableBool gl_separate_specular_color
  ("gl-separate-specular-color", true,
   PRC_DESC("When separate specular mode is on, the specular component "
            "will be written to the secondary instead of the primary "
            "color, which is added after the texturing stage.  In other "
            "words, the specular highlight will be unmodulated by the "
            "color of the texture."));

ConfigVariableBool gl_cube_map_seamless
  ("gl-cube-map-seamless", true,
   PRC_DESC("This configures Panda to try and enable seamless cube map "
            "sampling when supported.  This will help to remove seams "
            "that show up at cube map edges, especially at lower "
            "resolutions.  On by default; disable if you suspect that "
            "this is causing problems or if you simply don't need the "
            "functionality."));

ConfigVariableBool gl_dump_compiled_shaders
  ("gl-dump-compiled-shaders", false,
   PRC_DESC("This configures Panda to dump the binary content of GLSL "
            "programs to disk with a filename like glsl_program0.dump "
            "into the current directory."));

ConfigVariableBool gl_validate_shaders
  ("gl-validate-shaders", true,
   PRC_DESC("Set this to true to enable glValidateShader the first time "
            "a shader is bound.  This may cause helpful information about "
            "shaders to be printed."));

ConfigVariableBool gl_immutable_texture_storage
  ("gl-immutable-texture-storage", false,
   PRC_DESC("This configures Panda to pre-allocate immutable storage "
            "for each texture.  This improves runtime performance, but "
            "changing the size or type of a texture will be slower."));

ConfigVariableBool gl_use_bindless_texture
  ("gl-use-bindless-texture", false,
   PRC_DESC("Set this to let Panda use OpenGL's bindless texture "
            "extension for all textures passed to shaders, for improved "
            "performance.  This is an experimental feature and comes "
            "with a few caveats; for one, it requires that all sampler "
            "uniforms have a layout(bindless_sampler) qualifier, and "
            "it also requires that the texture properties are not "
            "modified after the texture handle has been initialized."));

ConfigVariableBool gl_enable_memory_barriers
  ("gl-enable-memory-barriers", true,
   PRC_DESC("If this is set, Panda will make sure that every write "
            "to an image using an image2D (et al) binding will cause "
            "Panda to issue a memory barrier before the next use of "
            "said texture, to ensure that all reads and writes are "
            "properly synchronized.  This may not be strictly necessary "
            "when using the 'coherent' qualifier, but Panda has no "
            "way to detect whether you are using those.  Turning "
            "this off may give a slight performance increase, but you "
            "have to know what you're doing."));

ConfigVariableBool gl_vertex_array_objects
  ("gl-vertex-array-objects", true,
   PRC_DESC("Setting this causes Panda to make use of vertex array "
            "objects to more efficiently switch between sets of "
            "vertex arrays.  This only has effect when vertex-arrays "
            "and vertex-buffers are both set.  This should usually be "
            "true unless you suspect a bug in the implementation. "));

ConfigVariableBool gl_fixed_vertex_attrib_locations
  ("gl-fixed-vertex-attrib-locations", false,
   PRC_DESC("Experimental feature."));

ConfigVariableBool gl_support_primitive_restart_index
  ("gl-support-primitive-restart-index", true,
   PRC_DESC("Setting this causes Panda to make use of primitive "
            "restart indices to more efficiently render line "
            "segment primitives.  Set to false if you suspect a bug "
            "in the driver implementation."));

ConfigVariableBool gl_support_sampler_objects
  ("gl-support-sampler-objects", true,
   PRC_DESC("Setting this allows Panda to make use of sampler "
            "objects.  Set to false if you suspect a bug in the "
            "driver implementation."));

ConfigVariableBool gl_support_shadow_filter
  ("gl-support-shadow-filter", true,
   PRC_DESC("Disable this if you suspect a bug in the driver "
            "implementation of ARB_shadow.  Particularly, older ATI "
            "cards suffered from a broken implementation of the "
            "shadow map filtering features."));

ConfigVariableBool gl_force_image_bindings_writeonly
  ("gl-force-image-bindings-writeonly", false,
   PRC_DESC("Forces all image inputs (not textures!) to be bound as writeonly, "
            "to read from an image, rebind it as sampler."));

ConfigVariableEnum<CoordinateSystem> gl_coordinate_system
  ("gl-coordinate-system", CS_yup_right,
   PRC_DESC("Which coordinate system to use as the internal "
            "coordinate system for OpenGL operations.  If you are "
            "using features like fixed-function sphere mapping, it is "
            "best to leave this to yup-right.  However, if you are "
            "creating a shader-only application, it may be easier and "
            "more efficient to set this to default."));

ConfigVariableBool gl_depth_zero_to_one
  ("gl-depth-zero-to-one", false,
   PRC_DESC("Normally, OpenGL uses an NDC coordinate space wherein the Z "
            "ranges from -1 to 1.  This setting can be used to instead use a "
            "range from 0 to 1, matching other graphics APIs.  This setting "
            "requires OpenGL 4.5, or NVIDIA GeForce 8+ hardware."));

extern ConfigVariableBool gl_parallel_arrays;

void CLP(init_classes)() {
  CLP(GeomContext)::init_type();
  CLP(GeomMunger)::init_type();
  CLP(GraphicsStateGuardian)::init_type();
  CLP(IndexBufferContext)::init_type();
#ifndef OPENGLES_1
  CLP(ShaderContext)::init_type();
#endif
#if defined(HAVE_CG) && !defined(OPENGLES)
  CLP(CgShaderContext)::init_type();
#endif
  CLP(TextureContext)::init_type();
#ifndef OPENGLES
  CLP(SamplerContext)::init_type();
#endif
  CLP(VertexBufferContext)::init_type();
  CLP(BufferContext)::init_type();
  CLP(GraphicsBuffer)::init_type();

#ifndef OPENGLES
  CLP(OcclusionQueryContext)::init_type();
  CLP(TimerQueryContext)::init_type();
  CLP(LatencyQueryContext)::init_type();
#endif

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system(GLSYSTEM_NAME);

  // We can't add any tags defining the available OpenGL capabilities, since
  // we won't know those until we create a graphics context (and the answer
  // may be different for different contexts).
}
