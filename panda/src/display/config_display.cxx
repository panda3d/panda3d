/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_display.cxx
 * @author drose
 * @date 1999-10-06
 */

#include "config_display.h"
#include "callbackGraphicsWindow.h"
#include "displayRegion.h"
#include "displayRegionCullCallbackData.h"
#include "displayRegionDrawCallbackData.h"
#include "standardMunger.h"
#include "graphicsStateGuardian.h"
#include "graphicsPipe.h"
#include "graphicsOutput.h"
#include "graphicsBuffer.h"
#include "graphicsDevice.h"
#include "graphicsWindow.h"
#include "graphicsWindowInputDevice.h"
#include "graphicsWindowProcCallbackData.h"
#include "mouseAndKeyboard.h"
#include "nativeWindowHandle.h"
#include "parasiteBuffer.h"
#include "pandaSystem.h"
#include "stereoDisplayRegion.h"
#include "subprocessWindow.h"
#include "windowHandle.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_DISPLAY)
  #error Buildsystem error: BUILDING_PANDA_DISPLAY not defined
#endif

ConfigureDef(config_display);
NotifyCategoryDef(display, "");
NotifyCategoryDef(gsg, display_cat);

ConfigureFn(config_display) {
  init_libdisplay();
}

ConfigVariableBool view_frustum_cull
("view-frustum-cull", true,
 PRC_DESC("This is normally true; set it false to disable view-frustum culling "
          "(primarily useful for debugging)."));

ConfigVariableBool pstats_unused_states
("pstats-unused-states", false,
 PRC_DESC("Set this true to show the number of unused states in the pstats "
          "graph for TransformState and RenderState counts.  This adds a bit "
          "of per-frame overhead to count these things up."));


// Warning!  The code that uses this is currently experimental and incomplete,
// and will almost certainly crash!  Do not set threading-model to anything
// other than its default of a single-threaded model unless you are developing
// Panda's threading system!
ConfigVariableString threading_model
("threading-model", "",
 PRC_DESC("This is the default threading model to use for new windows.  Use "
          "empty string for single-threaded, or something like \"cull/draw\" for "
          "a 3-stage pipeline.  See GraphicsEngine::set_threading_model(). "
          "EXPERIMENTAL and incomplete, do not use this!"));

ConfigVariableBool allow_nonpipeline_threads
("allow-nonpipeline-threads", false,
 PRC_DESC("This variable should only be set true for debugging or development "
          "purposes.  When true, the threading-model variable may specify "
          "a threaded pipeline mode, even if pipelining is not compiled in.  "
          "This will certainly result in erroneous behavior, and quite likely "
          "will cause a crash.  Do not set this unless you know what you "
          "are doing."));

ConfigVariableBool auto_flip
("auto-flip", false,
 PRC_DESC("This indicates the initial setting of the auto-flip flag.  Set it "
          "true to cause render_frame() to flip all the windows "
          "before it returns (in single-threaded mode only), or false to wait "
          "until an explicit call to flip_frame() or the next "
          "render_frame().  Setting it true gets more immediate response "
          "time, while setting it false can get a better frame rate as more "
          "is achieved in parallel with the graphics card."));

ConfigVariableBool sync_flip
("sync-flip", false,
 PRC_DESC("Set this true to attempt to flip all windows at the same time, "
          "or false to flip each window as late as possible.  Setting this "
          "false can improve parallelization.  This is a temporary "
          "variable; it will later be replaced with a more explicit control "
          "over synchronizing window flip."));

ConfigVariableBool yield_timeslice
("yield-timeslice", false,
 PRC_DESC("Set this true to yield the timeslice at the end of the frame to be "
          "more polite to other applications that are trying to run."));

ConfigVariableDouble subprocess_window_max_wait
("subprocess-window-max-wait", 0.2,
 PRC_DESC("This is the amount of time, in seconds, that the SubprocessWindow will "
          "wait in begin_flip for the parent process to remove the previously-rendered "
          "frame.  When this time is exceeded, the next frame will be rendered without "
          "having flipped the previous one.  This is designed to allow the Python "
          "process some time to run even when the parent window is offscreen or minimized."));

ConfigVariableString screenshot_filename
("screenshot-filename", "%~p-%a-%b-%d-%H-%M-%S-%Y-%~f.%~e",
 PRC_DESC("This specifies the filename pattern to be used to generate "
          "screenshots captured via save_screenshot_default().  See "
          "DisplayRegion::save_screenshot()."));

ConfigVariableString screenshot_extension
("screenshot-extension", "jpg",
 PRC_DESC("This specifies the default filename extension (and therefore the "
          "default image type) to be used for saving screenshots."));

ConfigVariableBool prefer_texture_buffer
("prefer-texture-buffer", true,
 PRC_DESC("Set this true to make GraphicsOutput::make_texture_buffer() always "
          "try to create an offscreen buffer supporting render-to-texture, "
          "if the graphics card claims to be able to support this feature.  "
          "If the graphics card cannot support this feature, this option is "
          "ignored.  This is usually the fastest way to render "
          "to a texture, and it presumably does not consume any additional "
          "framebuffer memory over a copy-to-texture operation (since "
          "the texture and the buffer share the "
          "same memory)."));

ConfigVariableBool prefer_parasite_buffer
("prefer-parasite-buffer", false,
 PRC_DESC("Set this true to make GraphicsOutput::make_texture_buffer() try to "
          "create a ParasiteBuffer before it tries to create an offscreen "
          "buffer (assuming it could not create a direct render buffer for "
          "some reason).  This may reduce your graphics card memory "
          "requirements by sharing memory with the framebuffer, but it can "
          "cause problems if the user subsequently resizes the window "
          "smaller than the buffer."));

ConfigVariableBool force_parasite_buffer
("force-parasite-buffer", false,
 PRC_DESC("Set this true to make GraphicsOutput::make_texture_buffer() really "
          "strongly prefer ParasiteBuffers over conventional offscreen buffers.  "
          "With this set, it will create a ParasiteBuffer every time an offscreen "
          "buffer is requested, even if this means reducing the buffer size to fit "
          "within the window.  The only exceptions are for buffers that, by their "
          "nature, really cannot use ParasiteBuffers (like depth textures).  You might "
          "set this true if you don't trust your graphics driver's support for "
          "offscreen buffers."));

ConfigVariableBool prefer_single_buffer
("prefer-single-buffer", true,
 PRC_DESC("Set this true to make GraphicsOutput::make_render_texture() first "
          "try to create a single-buffered offscreen buffer, before falling "
          "back to a double-buffered one (or whatever kind the source window "
          "has).  This is true by default to reduce waste of framebuffer "
          "memory, but you might get a performance benefit by setting it to "
          "false (since in that case the buffer can share a graphics context "
          "with the window)."));

ConfigVariableInt max_texture_stages
("max-texture-stages", -1,
 PRC_DESC("Set this to a positive integer to limit the number of "
          "texture stages reported by the GSG.  This can be used to limit "
          "the amount of multitexturing Panda will attempt to use.  "
          "If this is zero or less, the GSG will report its honest number "
          "of texture stages, allowing Panda the full use of the graphics "
          "card; if it is 1 or more, then Panda will never allow more than "
          "this number of texture stages simultaneously, regardless of "
          "what the GSG says it can do."));

ConfigVariableInt max_color_targets
("max-color-targets", -1,
 PRC_DESC("Set this to a positive integer to limit the number of "
          "color targets reported by the GSG.  This can be used to limit "
          "the amount of render targets Panda will attempt to use.  "
          "If this is zero or less, the GSG will report its honest number "
          "of color targets, allowing Panda the full use of the graphics "
          "card; if it is 1 or more, then Panda will never allow more than "
          "this number of color targets simultaneously, regardless of "
          "what the GSG says it can do."));

ConfigVariableBool support_render_texture
("support-render-texture", true,
 PRC_DESC("Set this true allow use of the render-to-a-texture feature, if it "
          "is supported by your graphics card.  Without this enabled, "
          "offscreen renders will be copied to a texture instead of directly "
          "rendered there."));

ConfigVariableBool support_rescale_normal
("support-rescale-normal", true,
 PRC_DESC("Set this true to allow use of the rescale-normal feature, if it "
          "is supported by your graphics card.  This allows lighting normals "
          "to be uniformly counter-scaled, instead of re-normalized, "
          "in the presence of a uniform scale, which should in principle be "
          "a bit faster.  This feature is only supported "
          "by the OpenGL API."));

ConfigVariableBool support_stencil
("support-stencil", true,
 PRC_DESC("Set this true to allow use of the stencil buffer, if it "
          "is supported by your graphics card.  If this is false, stencil "
          "buffer support will not be enabled, even if it is supported.  "
          "Generally, only very old cards do not support some kind of "
          "stencil buffer operations; but it is also not supported by "
          "our tinydisplay renderer.  "
          "The main reason to set this false is to test your code in "
          "the absence of stencil buffer support."));

ConfigVariableBool copy_texture_inverted
("copy-texture-inverted", false,
 PRC_DESC("Set this true to indicate that the GSG in use will invert textures when "
          "it performs a framebuffer-to-texture copy operation, or false to indicate "
          "that it does the right thing.  If this is not set, the default behavior is "
          "determined by the GSG's internal logic."));

ConfigVariableBool window_inverted
("window-inverted", false,
 PRC_DESC("Set this true to create all windows with the inverted flag set, so that "
          "they will render upside-down and backwards.  Normally this is useful only "
          "for debugging."));

ConfigVariableBool red_blue_stereo
("red-blue-stereo", false,
 PRC_DESC("Set this true to create windows with red-blue stereo mode enabled "
          "by default, if the framebuffer does not support true stereo "
          "rendering."));

ConfigVariableString red_blue_stereo_colors
("red-blue-stereo-colors", "red cyan",
 PRC_DESC("This defines the color channels that are used for the left and "
          "right eye, respectively, for red-blue-stereo mode.  This should "
          "be a two-word string, where each word is one of 'red', 'blue', "
          "'green', 'cyan', 'magenta', 'yellow', or 'alpha', or a union "
          "of two or more words separated by a vertical pipe (|)."));

ConfigVariableBool side_by_side_stereo
("side-by-side-stereo", false,
 PRC_DESC("Set this true to create windows with side-by-side stereo mode enabled "
          "by default, if the framebuffer does not support true stereo "
          "rendering."));

ConfigVariableDouble sbs_left_dimensions
("sbs-left-dimensions", "0.0 0.5 0.0 1.0",
 PRC_DESC("Defines the default region of the window that is used for the "
          "left eye, when side-by-side stereo is enabled.  This is a set of "
          "four numbers, in the form left right top bottom, similar to a "
          "normal DisplayRegion layout."));

ConfigVariableDouble sbs_right_dimensions
("sbs-right-dimensions", "0.5 1.0 0.0 1.0",
 PRC_DESC("Defines the default region of the window that is used for the "
          "right eye, when side-by-side stereo is enabled.  This is a set of "
          "four numbers, in the form left right top bottom, similar to a "
          "normal DisplayRegion layout."));

ConfigVariableBool swap_eyes
("swap-eyes", false,
 PRC_DESC("Set this true to reverse the left and right channel output of "
          "stereo DisplayRegions."));

ConfigVariableBool default_stereo_camera
("default-stereo-camera", true,
 PRC_DESC("When this is true, the default DisplayRegion created for "
          "a window or buffer with the stereo property will be a "
          "StereoDisplayRegion, which activates the stereo properties of "
          "the camera lens, and enables stereo.  Set this false to "
          "require StereoDisplayRegions to be created explicitly."));

ConfigVariableBool color_scale_via_lighting
("color-scale-via-lighting", true,
 PRC_DESC("When this is true, Panda will try to implement ColorAttribs and "
          "ColorScaleAttribs using the lighting interface, by "
          "creating a default material and/or an ambient light if "
          "necessary, even if lighting is ostensibly disabled.  This "
          "avoids the need to munge the vertex data to change each vertex's "
          "color.  Set this false to avoid this trickery, so that lighting "
          "is only enabled when the application specifically enables "
          "it.  See also alpha-scale-via-texture."));

ConfigVariableBool alpha_scale_via_texture
("alpha-scale-via-texture", true,
 PRC_DESC("When this is true, Panda will try to implement "
          "ColorScaleAttribs that affect alpha by "
          "creating an additional Texture layer over the geometry "
          "with a uniform alpha scale applied everywhere, if there "
          "is at least one available Texture slot available on the "
          "multitexture pipeline.  Set this false to avoid this "
          "trickery, so that texturing is only enabled when the "
          "application specifically enables it.  See also "
          "color-scale-via-lighting."));

ConfigVariableBool allow_incomplete_render
("allow-incomplete-render", true,
 PRC_DESC("When this is true, the frame may be rendered even if some of the "
          "geometry in the scene has been paged out, or if the textures are "
          "unavailable.  The nonresident geometry and textures will be "
          "rendered as soon as they can be read from disk, "
          "which may be several frames in the future.  When this is false, "
          "geometry is always paged in immediately when needed, holding up "
          "the frame render if necessary."));

ConfigVariableBool old_alpha_blend
("old-alpha-blend", false,
 PRC_DESC("Set this to true to enable the old alpha blending behavior from "
          "Panda 1.9 in which the alpha value written out to the framebuffer "
          "is squared.  The new behavior is more intuitive when compositing "
          "an semitransparent image produced using render-to-texture.  You "
          "should generally leave this false unless you have an effect that "
          "relies on the old behavior, or you suspect an implementation bug."));

ConfigVariableInt win_size
("win-size", "800 600",
 PRC_DESC("This is the default size at which to open a new window.  This "
          "replaces the deprecated win-width and win-height variables."));

ConfigVariableInt win_origin
("win-origin", "",
 PRC_DESC("This is the default position at which to open a new window.  This "
          "replaces the deprecated win-origin-x and win-origin-y variables. "
          "A window coordinate of -1 means to choose a default value, "
          "whereas -2 means to center the window on the screen."));

ConfigVariableBool fullscreen
("fullscreen", false);

ConfigVariableBool undecorated
("undecorated", false,
 PRC_DESC("This specifies the default value of the 'undecorated' window "
          "property.  When this is true, the default window is created "
          "without a title bar or resizable border."));

ConfigVariableBool win_fixed_size
("win-fixed-size", false,
 PRC_DESC("This specifies the default value of the 'fixed_size' window "
          "property.  When this is true, the default window is created "
          "without a resizable border."));

ConfigVariableBool cursor_hidden
("cursor-hidden", false);

ConfigVariableFilename icon_filename
("icon-filename", "");

ConfigVariableFilename cursor_filename
("cursor-filename", "");

ConfigVariableEnum<WindowProperties::ZOrder> z_order
("z-order", WindowProperties::Z_normal);

ConfigVariableString window_title
("window-title", "Panda");

ConfigVariableInt parent_window_handle
("parent-window-handle", 0,
 PRC_DESC("The window handle of the parent window to attach the Panda window "
          "to, for the purposes of creating an embedded window.  This is "
          "an HWND on Windows, or the NSWindow pointer or XWindow pointer "
          "converted to an integer, on OSX and X11."));

ConfigVariableBool win_unexposed_draw
("win-unexposed-draw", true,
 PRC_DESC("Specifies the default setting of GraphicsWindow::set_unexposed_draw().  "
          "See that method for more information."));

ConfigVariableFilename subprocess_window
("subprocess-window", "",
 PRC_DESC("The filename of a SubprocessWindowBuffer's temporary mmap file, "
          "used for opening a window in a child process and rendering "
          "to a different window in the parent process.  "
          "This is specifically used for OSX when the plugin is compiled, "
          "and is not used or needed in other environments.  See "
          "WindowProperties::set_subprocess_window()."));

ConfigVariableString framebuffer_mode
("framebuffer-mode", "",
 PRC_DESC("No longer has any effect.  Do not use."));

ConfigVariableBool framebuffer_hardware
("framebuffer-hardware", true,
 PRC_DESC("True if FM_hardware should be added to the default framebuffer "
          "properties, which requests a hardware-accelerated display."));
ConfigVariableBool framebuffer_software
("framebuffer-software", false,
 PRC_DESC("True if FM_software should be added to the default framebuffer "
          "properties, which requests a software-only display."));
ConfigVariableBool framebuffer_multisample
("framebuffer-multisample", false,
 PRC_DESC("True if FM_multisample should be added to the default framebuffer "
          "properties, which requests a multisample-capable display, if "
          "possible.  This can be used to implement full-screen "
          "antialiasing."));
ConfigVariableBool framebuffer_depth
("framebuffer-depth", true,
 PRC_DESC("True if FM_depth should be added to the default framebuffer "
          "properties, which requests a depth buffer."));
ConfigVariableBool framebuffer_alpha
("framebuffer-alpha", true,
 PRC_DESC("True if FM_alpha should be added to the default framebuffer "
          "properties, which requests an alpha channel if possible."));
ConfigVariableBool framebuffer_stencil
("framebuffer-stencil", false,
 PRC_DESC("True if FM_stencil should be added to the default framebuffer "
          "properties, which requests an stencil buffer if possible."));
ConfigVariableBool framebuffer_accum
("framebuffer-accum", false,
 PRC_DESC("True if FM_accum should be added to the default framebuffer "
          "properties, which requests an accumulator buffer if possible."));
ConfigVariableBool framebuffer_stereo
("framebuffer-stereo", false,
 PRC_DESC("True if FM_stereo should be added to the default framebuffer "
          "properties, which requests a stereo-capable display, if "
          "supported by the graphics driver."));
ConfigVariableBool framebuffer_srgb
("framebuffer-srgb", false,
 PRC_DESC("Set this to request an sRGB framebuffer, which will "
          "convert all values to linear space before blending.  This "
          "means that the output will be properly gamma-corrected, as "
          "long as all the input textures are either converted from "
          "original sRGB to linear or sRGB textures are used."));
ConfigVariableBool framebuffer_float
("framebuffer-float", false,
 PRC_DESC("Set this to request a framebuffer that uses floating-point "
          "storage for the color channel."));

ConfigVariableInt depth_bits
("depth-bits", 0,
 PRC_DESC("The minimum number of depth buffer bits requested."));
ConfigVariableInt color_bits
("color-bits", "",
 PRC_DESC("The minimum number of total color buffer bits requested.  If you "
          "specify only one value, it will represent the total value for the "
          "red, green and blue channels, and indicates you don't care how the "
          "bits are divided up among the red, green and blue channels.  If "
          "you specify three values, it represents three separate red, green "
          "and blue bit requirements."));
ConfigVariableInt alpha_bits
("alpha-bits", 0,
 PRC_DESC("The minimum number of alpha buffer bits requested."));
ConfigVariableInt stencil_bits
("stencil-bits", 0,
 PRC_DESC("The minimum number of stencil buffer bits requested."));
ConfigVariableInt accum_bits
("accum-bits", 0,
 PRC_DESC("The minimum number of accumulator buffer bits requested."));
ConfigVariableInt multisamples
("multisamples", 0,
 PRC_DESC("The minimum number of samples requested."));
ConfigVariableInt back_buffers
("back-buffers", 1,
 PRC_DESC("The default number of back buffers requested."));

ConfigVariableDouble pixel_zoom
("pixel-zoom", 1.0,
 PRC_DESC("The default pixel_zoom factor for new windows."));

ConfigVariableInt shadow_depth_bits
("shadow-depth-bits", 24,
 PRC_DESC("The minimum number of depth buffer bits requested when rendering "
          "shadow maps.  Set this to 32 for more depth resolution in shadow "
          "maps."));

ConfigVariableColor background_color
("background-color", "0.41 0.41 0.41 0.0",
 PRC_DESC("Specifies the rgb(a) value of the default background color for a "
          "new window or offscreen buffer."));

ConfigVariableBool sync_video
("sync-video", true,
 PRC_DESC("Configure this true to request the rendering to sync to the video "
          "refresh, or false to let your frame rate go as high as it can, "
          "irrespective of the video refresh.  Usually you want this true, "
          "but it may be useful to set it false during development for a "
          "cheesy estimate of scene complexity.  Some drivers may ignore "
          "this request."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libdisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  CallbackGraphicsWindow::init_type();
  DisplayRegion::init_type();
  DisplayRegionCullCallbackData::init_type();
  DisplayRegionDrawCallbackData::init_type();
  DisplayRegionPipelineReader::init_type();
  GraphicsBuffer::init_type();
  GraphicsDevice::init_type();
  GraphicsOutput::init_type();
  GraphicsPipe::init_type();
  GraphicsStateGuardian::init_type();
  GraphicsWindow::init_type();
  GraphicsWindowInputDevice::init_type();
  GraphicsWindowProcCallbackData::init_type();
  MouseAndKeyboard::init_type();
  NativeWindowHandle::init_type();
  ParasiteBuffer::init_type();
  StandardMunger::init_type();
  StereoDisplayRegion::init_type();
#ifdef SUPPORT_SUBPROCESS_WINDOW
  SubprocessWindow::init_type();
#endif
  WindowHandle::init_type();
  WindowHandle::OSHandle::init_type();

#if defined(HAVE_THREADS) && defined(DO_PIPELINING)
  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("pipelining");
#endif
}
