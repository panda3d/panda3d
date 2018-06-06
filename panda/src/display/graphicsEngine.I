/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsEngine.I
 * @author drose
 * @date 2002-02-24
 */

/**
 * Returns a ReMutex object that is held by the GraphicsEngine during the
 * entire call to render_frame().  While you hold this lock you can be
 * confident that no part of the frame will be rendered (at least by the app
 * thread).
 */
INLINE const ReMutex &GraphicsEngine::
get_render_lock() const {
  return _public_lock;
}

/**
 * Set this flag true to indicate the GraphicsEngine should automatically
 * cause windows to sync and flip as soon as they have finished drawing,
 * rather than waiting for all of the windows to finish drawing first so they
 * can flip together.
 *
 * This only affects the timing of when the flip occurs.  If this is true (the
 * default), the flip occurs before render_frame() returns.  If this is false,
 * the flip occurs whenever flip_frame() is called, or at the beginning of the
 * next call to render_frame(), if flip_frame() is never called.
 */
INLINE void GraphicsEngine::
set_auto_flip(bool auto_flip) {
  // We don't bother with the mutex here.  It's just a bool, after all.
  _auto_flip = auto_flip;
}

/**
 * Returns the current setting for the auto-flip flag.  See set_auto_flip.
 */
INLINE bool GraphicsEngine::
get_auto_flip() const {
  // We don't bother with the mutex here.  It's just a bool, after all.
  return _auto_flip;
}

/**
 * Set this flag true to indicate the GraphicsEngine should start portal
 * culling
 */
INLINE void GraphicsEngine::
set_portal_cull(bool value) {
  // We don't bother with the mutex here.  It's just a bool, after all.
  _portal_enabled = value;
}

/**
 * Returns the current setting for the portal culling flag.
 */
INLINE bool GraphicsEngine::
get_portal_cull() const {
  // We don't bother with the mutex here.  It's just a bool, after all.
  return _portal_enabled;
}

/**
 * Sets the Loader object that will be assigned to every GSG created with this
 * GraphicsEngine.  See GraphicsStateGuardian::set_loader().
 */
INLINE void GraphicsEngine::
set_default_loader(Loader *loader) {
  _default_loader = loader;
}

/**
 * Returns the Loader object that will be assigned to every GSG created with
 * this GraphicsEngine.  See GraphicsStateGuardian::set_loader().
 */
INLINE Loader *GraphicsEngine::
get_default_loader() const {
  return _default_loader;
}

/**
 * Calls GraphicsPipe::close_gsg() on the indicated pipe and GSG.  This
 * function mainly exists to allow GraphicsEngine::WindowRenderer to call the
 * protected method GraphicsPipe::close_gsg().
 */
INLINE void GraphicsEngine::
close_gsg(GraphicsPipe *pipe, GraphicsStateGuardian *gsg) {
  pipe->close_gsg(gsg);
}

/**
 * Syntactic shorthand for make_output.  This is the preferred way to create
 * an offscreen buffer, when you already have an onscreen window or another
 * buffer to start with.  For the first parameter, pass an existing
 * GraphicsOutput object, e.g.  the main window; this allows the buffer to
 * adapt itself to that window's framebuffer properties, and allows maximum
 * sharing of resources.
 */
INLINE GraphicsOutput *GraphicsEngine::
make_buffer(GraphicsOutput *host, const std::string &name,
            int sort, int x_size, int y_size) {
  GraphicsOutput *result = make_output(host->get_pipe(), name, sort,
                                       FrameBufferProperties(),
                                       WindowProperties::size(x_size, y_size),
                                       GraphicsPipe::BF_refuse_window |
                                       GraphicsPipe::BF_fb_props_optional,
                                       host->get_gsg(), host);
  return result;
}

/**
 * Syntactic shorthand for make_output.  This flavor accepts a GSG rather than
 * a GraphicsOutput as the first parameter, which is too limiting and
 * disallows the possibility of creating a ParasiteBuffer if the user's
 * graphics hardware prefers that.  It also attempts to request specific
 * framebuffer properties and may therefore do a poorer job of sharing the GSG
 * between the old buffer and the new.
 *
 * For these reasons, this variant is a poor choice unless you are creating an
 * offscreen buffer for the first time, without an onscreen window already in
 * existence.  If you already have an onscreen window, you should use the
 * other flavor of make_buffer() instead, which accepts a GraphicsOutput as
 * the first parameter.
 */
INLINE GraphicsOutput *GraphicsEngine::
make_buffer(GraphicsStateGuardian *gsg, const std::string &name,
            int sort, int x_size, int y_size) {
  FrameBufferProperties fb_props = FrameBufferProperties::get_default();
  fb_props.set_back_buffers(0);
  fb_props.set_stereo(0);
  fb_props.set_accum_bits(0);
  fb_props.set_multisamples(0);
  fb_props.set_force_hardware(0);
  fb_props.set_force_software(0);
  GraphicsOutput *result = make_output(gsg->get_pipe(), name, sort,
                                       fb_props,
                                       WindowProperties::size(x_size, y_size),
                                       GraphicsPipe::BF_refuse_window |
                                       GraphicsPipe::BF_fb_props_optional,
                                       gsg, nullptr);
  return result;
}

/**
 * Syntactic shorthand for make_buffer.
 */
INLINE GraphicsOutput *GraphicsEngine::
make_parasite(GraphicsOutput *host, const std::string &name,
              int sort, int x_size, int y_size) {
  GraphicsOutput *result = make_output(host->get_pipe(), name, sort,
                                       FrameBufferProperties(),
                                       WindowProperties::size(x_size, y_size),
                                       GraphicsPipe::BF_require_parasite |
                                       GraphicsPipe::BF_fb_props_optional,
                                       host->get_gsg(), host);
  return result;
}
