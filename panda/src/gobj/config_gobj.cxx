/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_gobj.cxx
 * @author drose
 * @date 1999-10-01
 */

#include "animateVerticesRequest.h"
#include "bufferContext.h"
#include "config_putil.h"
#include "config_gobj.h"
#include "geom.h"
#include "geomCacheEntry.h"
#include "geomMunger.h"
#include "geomPrimitive.h"
#include "geomTriangles.h"
#include "geomTrianglesAdjacency.h"
#include "geomTristrips.h"
#include "geomTristripsAdjacency.h"
#include "geomTrifans.h"
#include "geomPatches.h"
#include "geomLines.h"
#include "geomLinesAdjacency.h"
#include "geomLinestrips.h"
#include "geomLinestripsAdjacency.h"
#include "geomPoints.h"
#include "geomVertexArrayData.h"
#include "geomVertexArrayFormat.h"
#include "geomVertexData.h"
#include "geomVertexFormat.h"
#include "lens.h"
#include "material.h"
#include "occlusionQueryContext.h"
#include "orthographicLens.h"
#include "matrixLens.h"
#include "paramTexture.h"
#include "perspectiveLens.h"
#include "queryContext.h"
#include "sliderTable.h"
#include "texture.h"
#include "texturePoolFilter.h"
#include "textureReloadRequest.h"
#include "textureStage.h"
#include "textureContext.h"
#include "timerQueryContext.h"
#include "samplerContext.h"
#include "samplerState.h"
#include "shader.h"
#include "shaderContext.h"
#include "transformBlend.h"
#include "transformBlendTable.h"
#include "transformTable.h"
#include "userVertexSlider.h"
#include "userVertexTransform.h"
#include "vertexDataBuffer.h"
#include "vertexTransform.h"
#include "vertexSlider.h"
#include "videoTexture.h"
#include "geomContext.h"
#include "vertexBufferContext.h"
#include "indexBufferContext.h"
#include "internalName.h"

#include "dconfig.h"
#include "string_utils.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_GOBJ)
  #error Buildsystem error: BUILDING_PANDA_GOBJ not defined
#endif

Configure(config_gobj);
NotifyCategoryDef(gobj, "");
NotifyCategoryDef(shader, "");

ConfigVariableInt max_texture_dimension
("max-texture-dimension", -1,
 PRC_DESC("Set this to the maximum size a texture is allowed to be in either "
          "dimension.  This is generally intended as a simple way to restrict "
          "texture sizes for limited graphics cards.  When this is greater "
          "than zero, each texture image loaded from a file (but only those "
          "loaded from a file) will be automatically scaled down, if "
          "necessary, so that neither dimension is larger than this value.  "
          "If this is less than zero, the size limit is taken from the "
          "primary GSG.  If this is exactly zero, there is no limit."));

ConfigVariableDouble texture_scale
("texture-scale", 1.0,
 PRC_DESC("This is a global scale factor that is applied to each texture "
          "as it is loaded from disk.  For instance, a value of 0.5 will "
          "reduce each texture to one-half its size in each dimension.  This "
          "scale factor is applied before textures-power-2 or "
          "max-texture-dimension."));

ConfigVariableInt texture_scale_limit
("texture-scale-limit", 4,
 PRC_DESC("This specifies the limit below which texture-scale will not "
          "reduce a texture image.  This is a single dimension which applies "
          "to both X and Y."));

ConfigVariableList exclude_texture_scale
("exclude-texture-scale",
 PRC_DESC("This is a list of glob patterns for texture filenames "
          "(excluding the directory part of the filename, but including "
          "the extension); for instance, 'digits_*.png'.  Any texture "
          "filenames that match one of these patterns will not be affected "
          "by max-texture-dimension or texture-scale."));

ConfigVariableBool keep_texture_ram
("keep-texture-ram", false,
 PRC_DESC("Set this to true to retain the ram image for each texture after it "
          "has been prepared with the GSG.  This will allow the texture to be "
          "prepared with multiple GSG's, or to be re-prepared later after it is "
          "explicitly released from the GSG, without having to reread the "
          "texture image from disk; but it will consume memory somewhat "
          "wastefully."));

ConfigVariableBool driver_compress_textures
("driver-compress-textures", false,
 PRC_DESC("Set this true to ask the graphics driver to compress textures, "
          "rather than compressing them in-memory first.  Depending on "
          "your graphics driver, you may or may not get better performance "
          "or results by setting this true.  Setting it true may also "
          "allow you to take advantage of some exotic compression algorithm "
          "other than DXT1/3/5 that your graphics driver supports, but "
          "which is unknown to Panda.  If the libsquish library is not "
          "compiled into Panda, textures cannot be compressed in-memory, "
          "and will always be handed to the graphics driver, regardless "
          "of this setting."));

ConfigVariableBool driver_generate_mipmaps
("driver-generate-mipmaps", true,
 PRC_DESC("Set this true to use the hardware to generate mipmaps "
          "automatically in all cases, if supported.  Set it false "
          "to generate mipmaps in software when possible."));

ConfigVariableBool vertex_buffers
("vertex-buffers", true,
 PRC_DESC("Set this true to allow the use of vertex buffers (or buffer "
          "objects, as OpenGL dubs them) for rendering vertex data.  This "
          "can greatly improve rendering performance on "
          "higher-end graphics cards, at the cost of some additional "
          "graphics memory (which might otherwise be used for textures "
          "or offscreen buffers).  On lower-end graphics cards this will "
          "make little or no difference."));

ConfigVariableBool vertex_arrays
("vertex-arrays", true,
 PRC_DESC("Set this true to allow the use of vertex arrays for rendering "
          "OpenGL vertex data.  This, or vertex buffers, is the normal "
          "way of issuing vertices ever since OpenGL 1.1, and you "
          "almost always want to have this set to true.  However, some very "
          "buggy graphics drivers may have problems handling vertex arrays "
          "correctly, so if you are experiencing problems you might try "
          "setting this to false.  If this is false, Panda will fall back "
          "to using immediate-mode commands like glVertex3f(), etc., to "
          "issue the vertices, which is potentially much slower than "
          "vertex arrays.  Setting this false also disables vertex buffers, "
          "effectively ignoring the setting of the vertex-buffers variable "
          "(since vertex buffers are a special case of vertex arrays in "
          "OpenGL).  This variable is normally not enabled in a production "
          "build.  This has no effect on DirectX rendering."));

ConfigVariableBool display_lists
("display-lists", false,
 PRC_DESC("Set this true to allow the use of OpenGL display lists for "
          "rendering static geometry.  On some systems, this can result "
          "in a performance improvement over vertex buffers alone; on "
          "other systems (particularly low-end systems) it makes little to "
          "no difference.  On some systems, using display lists can actually "
          "reduce performance.  This has no effect on DirectX rendering or "
          "on dynamic geometry (e.g. soft-skinned animation)."));

ConfigVariableBool hardware_animated_vertices
("hardware-animated-vertices", false,
 PRC_DESC("Set this true to allow the transforming of soft-skinned "
          "animated vertices via hardware, if supported, or false always "
          "to perform the vertex animation via software within Panda.  "
          "If you have a card that supports this, and your scene does "
          "not contain too many vertices already, this can provide a "
          "performance boost by offloading some work from your CPU onto "
          "your graphics card.  It may also help by reducing the bandwidth "
          "necessary on your computer's bus.  However, in some cases it "
          "may actually reduce performance."));

ConfigVariableBool hardware_point_sprites
("hardware-point-sprites", true,
 PRC_DESC("Set this true to allow the use of hardware extensions when "
          "rendering perspective-scaled points and point sprites.  When "
          "false, these large points are always simulated via quads "
          "computed in software, even if the hardware claims it can "
          "support them directly."));

ConfigVariableBool hardware_points
("hardware-points", true,
 PRC_DESC("Set this true to allow the use of hardware extensions when "
          "rendering large points.  When false, large points (even if "
          "untextured) will be simulated via quads computed in software."));

ConfigVariableBool singular_points
("singular-points", true,
 PRC_DESC("Set this true to insist that when RenderModeAttrib::M_points is "
          "used, each point appears only once in the result, even if "
          "the vertex is referenced multiple times.  This is particularly "
          "important when rendering points from a triangle mesh and you "
          "don't want the points to appear repeatedly."));

ConfigVariableBool matrix_palette
("matrix-palette", false,
 PRC_DESC("Set this true to allow the use of the matrix palette when "
          "animating vertices in hardware.  The matrix palette is "
          "not supported by all devices, but if it is, using "
          "it can allow animation of more sophisticated meshes "
          "in hardware, and it can also improve the "
          "performance of animating some simpler meshes.  Without "
          "this option, certain meshes will have to be animated in "
          "software.  However, this option is not enabled by default, "
          "because its support seems to be buggy in certain drivers "
          "(ATI FireGL T2 8.103 in particular.)"));

ConfigVariableBool display_list_animation
("display-list-animation", false,
 PRC_DESC("Set this true to allow the use of OpenGL display lists for "
          "rendering animated geometry (when the geometry is animated "
          "by the hardware).  This is not on by default because there "
          "appear to be some driver issues with this on my FireGL T2, "
          "but it should be perfectly doable in principle, and might get "
          "you a small performance boost."));

ConfigVariableBool connect_triangle_strips
("connect-triangle-strips", true,
 PRC_DESC("Set this true to send a batch of triangle strips to the graphics "
          "card as one long triangle strip, connected by degenerate "
          "triangles, or false to send them as separate triangle strips "
          "with no degenerate triangles.  On PC hardware, using one long "
          "triangle strip may help performance by reducing the number "
          "of separate graphics calls that have to be made."));

ConfigVariableBool preserve_triangle_strips
("preserve-triangle-strips", false,
 PRC_DESC("Set this true to indicate a preference for keeping triangle strips "
          "when possible, instead of decomposing them into triangles.  When "
          "this is true, flatten_strong and unify operations may be less "
          "effective at combining multiple Geoms together, but they will "
          "not implicitly decompose triangle strips."));

ConfigVariableBool dump_generated_shaders
("dump-generated-shaders", false,
 PRC_DESC("Set this true to cause all generated shaders to be written "
          "to disk.  This is useful for debugging broken shader "
          "generators."));

ConfigVariableBool cache_generated_shaders
("cache-generated-shaders", true,
 PRC_DESC("Set this true to cause all generated shaders to be cached in "
          "memory.  This is useful to prevent unnecessary recompilation."));

ConfigVariableBool vertices_float64
("vertices-float64", false,
 PRC_DESC("When this is true, the default float format for vertices "
          "will be a 64-bit double-precision float, instead "
          "of the normal 32-bit single-precision float.  This must be set "
          "at static init time to have the broadest effect.  You almost never "
          "want to set this true, since current hardware does not support "
          "double-precision vertices, and setting this will just require the "
          "driver to downsample the vertices at load time, making everything "
          "slower."));

ConfigVariableInt vertex_column_alignment
("vertex-column-alignment", 4,
 PRC_DESC("This specifies the default byte alignment for each column of "
          "data within a GeomVertexData when it is assembled using the default "
          "interfaces.  Normally, you should not change this config variable "
          "(which would change this value globally), but instead specify any "
          "alignment requirements on a per-column basis as you construct a "
          "GeomVertexFormat.  Setting this value globally could result in "
          "much needless wasted space in all vertex data objects, but it "
          "could be useful for simple experiments.  Also see "
          "vertex-animation-align-16 for a variable that controls "
          "this alignment for the vertex animation columns only."));

ConfigVariableBool vertex_animation_align_16
("vertex-animation-align-16",
#ifdef LINMATH_ALIGN
 true,
#else
 false,
#endif
 PRC_DESC("If this is true, then animated vertices will be created with 4-component "
          "floats and aligned to 16-byte boundaries, to allow efficient vectorization "
          "(e.g. SSE2) operations when computing animations.  If this is false, "
          "animated vertices will be packed as tightly as possible, in the normal way, "
          "which will optimize the amount of memory that must be sent to the graphics "
          "card, but prevent the use of SSE2 to calculate animation.  This does not "
          "affect unanimated vertices, which are always packed tightly.  This also "
          "impacts only vertex formats created within Panda subsystems; custom "
          "vertex formats are not affected."));

ConfigVariableEnum<AutoTextureScale> textures_power_2
("textures-power-2", ATS_down,
 PRC_DESC("Specify whether textures should automatically be constrained to "
          "dimensions which are a power of 2 when they are loaded from "
          "disk.  Set this to 'none' to disable this feature, or to "
          "'down' or 'up' to scale down or up to the nearest power of 2, "
          "respectively.  This only has effect on textures which are not "
          "already a power of 2."));

ConfigVariableEnum<AutoTextureScale> textures_square
("textures-square", ATS_none,
 PRC_DESC("Specify whether textures should automatically be constrained to "
          "a square aspect ratio when they are loaded from disk.  Set this "
          "to 'none', 'down', or 'up'.  See textures-power-2."));

ConfigVariableBool textures_auto_power_2
("textures-auto-power-2", false,
 PRC_DESC("If this is true, then panda will wait until you open a window, "
          "and then ask the window if it supports non-power-of-two textures. "
          "If so, then the config variable textures_power_2 will "
          "automatically be adjusted.  The pitfall of doing this is that if "
          "you then open a second window that doesn't support the same "
          "capabilities, it will have no choice but to print an error message."));

ConfigVariableBool textures_header_only
("textures-header-only", false,
 PRC_DESC("If this is true, texture images will not actually be loaded from "
          "disk, but the image header information will be consulted to verify "
          "number of channels and so forth.  The texture images themselves "
          "will be generated in a default blue color."));

ConfigVariableInt simple_image_size
("simple-image-size", "16 16",
 PRC_DESC("This is an x y pair that specifies the maximum size of an "
          "automatically-generated "
          "texture simple image.  The simple image can displayed before "
          "the texture has been loaded from disk."));

ConfigVariableDouble simple_image_threshold
("simple-image-threshold", 0.1,
 PRC_DESC("This is a value that indicates how closely a texture's "
          "generated simple "
          "image should approximate the original image.  The smaller the "
          "number, the closer the match; small numbers will result in "
          "simple images close to the maximum size specified by "
          "simple-image-size.  Larger numbers will result in smaller "
          "simple images.  Generally the value should be considerably "
          "less than 1."));

ConfigVariableInt geom_cache_size
("geom-cache-size", 5000,
 PRC_DESC("Specifies the maximum number of entries in the cache "
          "for storing pre-processed data for rendering "
          "vertices.  This limit is flexible, and may be "
          "temporarily exceeded if many different Geoms are "
          "pre-processed during the space of a single frame."));

ConfigVariableInt geom_cache_min_frames
("geom-cache-min-frames", 1,
 PRC_DESC("Specifies the minimum number of frames any one particular "
          "object will remain in the geom cache, even if geom-cache-size "
          "is exceeded."));

ConfigVariableInt released_vbuffer_cache_size
("released-vbuffer-cache-size", 1048576,
 PRC_DESC("Specifies the size in bytes of the cache of vertex "
          "buffers that have recently been released.  If a new vertex "
          "buffer is prepared while a recently-released one of the same "
          "size is still in the cache, that same buffer is recycled.  This "
          "cuts down on the overhead of creating and destroying vertex "
          "buffers on the graphics card."));

ConfigVariableInt released_ibuffer_cache_size
("released-ibuffer-cache-size", 102400,
 PRC_DESC("Specifies the size in bytes of the cache of index "
          "buffers that have recently been released.  If a new index "
          "buffer is prepared while a recently-released one of the same "
          "size is still in the cache, that same buffer is recycled.  This "
          "cuts down on the overhead of creating and destroying index "
          "buffers on the graphics card."));

ConfigVariableDouble default_near
("default-near", 1.0,
 PRC_DESC("The default near clipping distance for all cameras."));

ConfigVariableDouble default_far
("default-far", 100000.0,
 PRC_DESC("The default far clipping distance for all cameras."));

ConfigVariableDouble lens_far_limit
("lens-far-limit", 0.0000001,
  PRC_DESC("This number is used to reduce the effect of numeric inaccuracies "
           "in Lens::extrude().  It should be a very small, positive number, "
           "almost zero; set it larger if Lens::extrude() returns values "
           "that appear meaningless, and set it smaller if you appear to be "
           "unable to move the far plane out far enough."));

ConfigVariableDouble default_fov
("default-fov", 30.0,
 PRC_DESC("The default field of view in degrees for all cameras.  This is "
          "defined as a min_fov; that is, it is the field-of-view for the "
          "smallest of the X and Y sizes of the window, which is usually "
          "the vertical field of view (windows are usually wider than they "
          "are tall).  For a 4x3 window, 30 degrees vertical is roughly "
          "40 degrees horizontal."));

ConfigVariableDouble default_iod
("default-iod", 0.2,
 PRC_DESC("The default interocular distance for stereo cameras."));

ConfigVariableDouble default_converge
("default-converge", 25.0,
 PRC_DESC("The default convergence distance for stereo cameras."));

ConfigVariableDouble default_keystone
("default-keystone", 0.0f,
 PRC_DESC("The default keystone correction, as an x y pair, for all cameras."));

ConfigVariableFilename vertex_save_file_directory
("vertex-save-file-directory", "",
 PRC_DESC("The directory in which the saved vertex data file is created "
          "for saving vertex buffers that have been evicted from RAM.  If "
          "this is the empty string, or an invalid directory, a system "
          "default directory will be chosen."));

ConfigVariableString vertex_save_file_prefix
("vertex-save-file-prefix", "p3d_vdata_",
 PRC_DESC("A prefix used to generate the filename for the saved vertex "
          "data file which is created for saving vertex buffers that have "
          "been evicted from RAM.  A uniquifying sequence number and "
          "filename extension will be appended to this string."));

ConfigVariableInt vertex_data_small_size
("vertex-data-small-size", 64,
 PRC_DESC("When a GeomVertexArrayData is this number of bytes or smaller, it "
          "is deemed too small to pay the overhead of paging it in and out, "
          "and it is permanently retained resident."));

ConfigVariableInt vertex_data_page_threads
("vertex-data-page-threads", 1,
 PRC_DESC("When this is nonzero (and Panda has been compiled with thread "
          "support) then this number of sub-threads will be spawned to "
          "evict vertex pages to disk and read them back again.  When this "
          "is 0, this work will be done in the main thread, which may "
          "introduce occasional random chugs in rendering."));

ConfigVariableInt graphics_memory_limit
("graphics-memory-limit", -1,
 PRC_DESC("This is a default limit that is imposed on each GSG at "
          "GSG creation time.  It limits the total amount of graphics "
          "memory, including texture memory and vertex buffer memory, "
          "that will be consumed by the GSG, regardless of whether the "
          "hardware claims to provide more graphics memory than this.  "
          "It is useful to put a ceiling on graphics memory consumed, since "
          "some drivers seem to allow the application to consume more "
          "memory than the hardware can realistically support.  "
          "Set this to -1 to have no limit other than the normal "
          "hardware-imposed limit."));

ConfigVariableInt sampler_object_limit
("sampler-object-limit", 2048,
 PRC_DESC("This is a default limit that is imposed on each GSG at "
          "GSG creation time.  It limits the total amount of sampler "
          "objects that will be k.ept by the GSG, regardless of whether "
          "the hardware claims to provide more sampler objects than this. "
          "Direct3D 10-capable hardware supports at least 4096 distinct "
          "sampler objects, but we provide a slightly more conservative "
          "limit by default."));

ConfigVariableDouble adaptive_lru_weight
("adaptive-lru-weight", 0.2,
 PRC_DESC("Specifies the weight factor used to compute the AdaptiveLru's "
          "exponential moving average."));

ConfigVariableInt adaptive_lru_max_updates_per_frame
("adaptive-lru-max-updates-per-frame", 40,
 PRC_DESC("The number of pages the AdaptiveLru class will update per "
          "frame.  Do not set this too high or it will degrade "
          "performance."));

ConfigVariableDouble async_load_delay
("async-load-delay", 0.0,
PRC_DESC("If this is nonzero, it represents an artificial delay, "
         "in seconds, that is imposed on every asynchronous load attempt "
         "(within the thread).  Its purpose is to help debug errors that "
         "may occur when an asynchronous load is delayed.  The "
         "delay is per-model, and all aync loads will be queued "
         "up behind the delay--it is as if the time it takes to read a "
         "file is increased by this amount per read."));

ConfigVariableInt lens_geom_segments
("lens-geom-segments", 50,
 PRC_DESC("This is the number of times to subdivide the visualization "
          "wireframe created when Lens::make_geometry() (or "
          "LensNode::show_frustum()) is called, for representing accurate "
          "curves.  Note that this is only "
          "used for a nonlinear lens such as a cylindrical or fisheye "
          "lens; for a normal perspective or orthographic lens, the "
          "wireframe is not subdivided."));

ConfigVariableBool stereo_lens_old_convergence
("stereo-lens-old-convergence", false,
 PRC_DESC("In Panda3D 1.8 and below, when using a stereo lens, Panda "
          "generate an incorrect frustum skew for a given convergence "
          "distance, meaning that the left-right images wouldn't "
          "overlap at the configured distance.  This calculation has "
          "since been corrected, but if your application relies on the "
          "old, incorrect behavior, this may be set to 'true' to switch "
          "back to the old calculation."));

ConfigVariableBool basic_shaders_only
("basic-shaders-only", false,
 PRC_DESC("Set this to true if you aren't interested in shader model three "
          "and beyond.  Setting this flag will cause panda to disable "
          "bleeding-edge shader functionality which tends to be unreliable "
          "or broken.  At some point, when functionality that is currently "
          "flaky becomes reliable, we may expand the definition of what "
          "constitutes 'basic' shaders."));

ConfigVariableString cg_glsl_version
("cg-glsl-version", "",
 PRC_DESC("If this is set, it forces the Cg compiler to generate GLSL "
          "code conforming to the given GLSL version when using the "
          "glslv, glslf or glslg profiles.  Use this when you are having "
          "problems with these profiles.  Example values are 120 or 150."));

ConfigVariableBool glsl_preprocess
("glsl-preprocess", true,
 PRC_DESC("If this is enabled, Panda looks for lines starting with "
          "#pragma include when loading a GLSL shader and processes "
          "it appropriately.  This can be useful if you have code that "
          "is shared between multiple shaders.  Set this to false if "
          "you have no need for this feature or if you do your own "
          "preprocessing of GLSL shaders."));

ConfigVariableInt glsl_include_recursion_limit
("glsl-include-recursion-limit", 10,
 PRC_DESC("This sets a limit on how many nested #pragma include "
          "directives that Panda will follow when glsl-preprocess is "
          "enabled.  This is used to prevent infinite recursion when "
          "two shader files include each other."));

ConfigureFn(config_gobj) {
  AnimateVerticesRequest::init_type();
  BufferContext::init_type();
  Geom::init_type();
  GeomCacheEntry::init_type();
  GeomPipelineReader::init_type();
  GeomContext::init_type();
  GeomLines::init_type();
  GeomLinesAdjacency::init_type();
  GeomLinestrips::init_type();
  GeomLinestripsAdjacency::init_type();
  GeomMunger::init_type();
  GeomPoints::init_type();
  GeomPrimitive::init_type();
  GeomPrimitivePipelineReader::init_type();
  GeomTriangles::init_type();
  GeomTrianglesAdjacency::init_type();
  GeomTrifans::init_type();
  GeomTristrips::init_type();
  GeomTristripsAdjacency::init_type();
  GeomPatches::init_type();
  GeomVertexArrayData::init_type();
  GeomVertexArrayDataHandle::init_type();
  GeomVertexArrayFormat::init_type();
  GeomVertexData::init_type();
  GeomVertexDataPipelineReader::init_type();
  GeomVertexDataPipelineWriter::init_type();
  GeomVertexFormat::init_type();
  IndexBufferContext::init_type();
  InternalName::init_type();
  Lens::init_type();
  Material::init_type();
  MatrixLens::init_type();
  OcclusionQueryContext::init_type();
  OrthographicLens::init_type();
  ParamTextureImage::init_type();
  ParamTextureSampler::init_type();
  PerspectiveLens::init_type();
  PreparedGraphicsObjects::EnqueuedObject::init_type();
  QueryContext::init_type();
  SamplerContext::init_type();
  SamplerState::init_type();
  ShaderContext::init_type();
  Shader::init_type();
  SliderTable::init_type();
  Texture::init_type();
  TextureContext::init_type();
  TexturePoolFilter::init_type();
  TextureReloadRequest::init_type();
  TextureStage::init_type();
  TimerQueryContext::init_type();
  TransformBlend::init_type();
  TransformBlendTable::init_type();
  TransformTable::init_type();
  UserVertexSlider::init_type();
  UserVertexTransform::init_type();
  VertexBufferContext::init_type();
  VertexSlider::init_type();
  VertexDataBuffer::init_type();
  VertexDataPage::init_type();
  VertexTransform::init_type();
  VideoTexture::init_type();

  // Registration of writeable object's creation functions with BamReader's
  // factory
  Geom::register_with_read_factory();
  GeomLines::register_with_read_factory();
  GeomLinesAdjacency::register_with_read_factory();
  GeomLinestrips::register_with_read_factory();
  GeomLinestripsAdjacency::register_with_read_factory();
  GeomPoints::register_with_read_factory();
  GeomTriangles::register_with_read_factory();
  GeomTrianglesAdjacency::register_with_read_factory();
  GeomTrifans::register_with_read_factory();
  GeomTristrips::register_with_read_factory();
  GeomTristripsAdjacency::register_with_read_factory();
  GeomPatches::register_with_read_factory();
  GeomVertexArrayData::register_with_read_factory();
  GeomVertexArrayFormat::register_with_read_factory();
  GeomVertexData::register_with_read_factory();
  GeomVertexFormat::register_with_read_factory();
  InternalName::register_with_read_factory();
  Material::register_with_read_factory();
  MatrixLens::register_with_read_factory();
  OrthographicLens::register_with_read_factory();
  ParamTextureImage::register_with_read_factory();
  ParamTextureSampler::register_with_read_factory();
  PerspectiveLens::register_with_read_factory();
  Shader::register_with_read_factory();
  SliderTable::register_with_read_factory();
  Texture::register_with_read_factory();
  TextureStage::register_with_read_factory();
  TransformBlendTable::register_with_read_factory();
  TransformTable::register_with_read_factory();
  UserVertexSlider::register_with_read_factory();
  UserVertexTransform::register_with_read_factory();
}
