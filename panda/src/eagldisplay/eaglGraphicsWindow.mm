/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file EAGLGraphicsWindow.mm
 * @author D. Lawrence
 * @date 2019-01-03
 */

#include "eaglGraphicsWindow.h"
#include "eaglGraphicsStateGuardian.h"
#include "mouseButton.h"
// #import "iOSNSNotificationHandler.h"

TypeHandle EAGLGraphicsWindow::_type_handle;

/**
 *
 */
EAGLGraphicsWindow::
EAGLGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const std::string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host) :
GraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  _view = nil;

  _emulated_mouse_input =
    GraphicsWindowInputDevice::pointer_only(this, "touch_mouse");
  _input_devices.push_back(_emulated_mouse_input);
}

/**
 *
 */
EAGLGraphicsWindow::
~EAGLGraphicsWindow() {
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool EAGLGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  begin_frame_spam(mode);
  PStatTimer timer(_make_current_pcollector, current_thread);
  
  EAGLGraphicsStateGuardian *eaglgsg;
  DCAST_INTO_R(eaglgsg, _gsg, false);
  nassertr(_view != nil, false);
  
  eaglgsg->lock_context();
  [EAGLContext setCurrentContext:eaglgsg->_context];
  
  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void EAGLGraphicsWindow::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);
  
  EAGLGraphicsStateGuardian *eaglgsg;
  DCAST_INTO_V(eaglgsg, _gsg);
  
  if (mode == FM_render) {
    copy_to_textures();
  }
  
  _gsg->end_frame(current_thread);
  
  [EAGLContext setCurrentContext:nil];
  eaglgsg->unlock_context();
  
  if (mode == FM_render) {
    trigger_flip();
    clear_cube_map_selection();
  }
}

/**
 * This function will be called within the draw thread after begin_flip() has
 * been called on all windows, to finish the exchange of the front and back
 * buffers.
 *
 * This should cause the window to wait for the flip, if necessary.
 */
void EAGLGraphicsWindow::
end_flip() {
  if (_gsg != (GraphicsStateGuardian *)NULL && _flip_ready) {
    EAGLGraphicsStateGuardian *eaglgsg;
    DCAST_INTO_V(eaglgsg, _gsg);
    
    eaglgsg->lock_context();
    
    // Presents the primary renderbuffer to the screen.
    glBindRenderbuffer(GL_RENDERBUFFER, _color_rb);
    [eaglgsg->_context presentRenderbuffer:GL_RENDERBUFFER];
    
    eaglgsg->unlock_context();
  }
  GraphicsWindow::end_flip();
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool EAGLGraphicsWindow::
open_window() {
  EAGLGraphicsPipe *eagl_pipe;
  DCAST_INTO_R(eagl_pipe, _pipe, false);
  
  EAGLGraphicsStateGuardian *eaglgsg;
  if (_gsg == 0) {
    // There is no old gsg.  Create a new one.
    eaglgsg = new EAGLGraphicsStateGuardian(_engine, _pipe, NULL);
    eaglgsg->choose_pixel_format(_fb_properties, (CAEAGLLayer *)_view.layer);
    _gsg = eaglgsg;
  } else {
    // If the old gsg has the wrong pixel format, create a new one that shares
    // with the old gsg.
    DCAST_INTO_R(eaglgsg, _gsg, false);
    if (!eaglgsg->get_fb_properties().subsumes(_fb_properties)) {
      eaglgsg = new EAGLGraphicsStateGuardian(_engine, _pipe, eaglgsg);
      eaglgsg->choose_pixel_format(_fb_properties, (CAEAGLLayer *)_view.layer);
      _gsg = eaglgsg;
    }
  }
  
  if (eaglgsg->_context == nil) {
    // Could not obtain a proper context.
    _gsg.clear();
    close_window();
    return false;
  }
  
  eaglgsg->lock_context();
  
  // Just create the view. The application developer will have to attach it
  // to something themselves.
  // TODO: Make this more flexible
  dispatch_sync(dispatch_get_main_queue(), ^{
    _view = [[PandaEAGLView alloc] initWithFrame:UIScreen.mainScreen.bounds graphicsWindow:this];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"ViewCreatedNotification" object:nil userInfo:@{@"view": _view}];
  });

  [EAGLContext setCurrentContext:eaglgsg->_context];
  
  eaglgsg->reset_if_new();
  
  // Usually we would create the framebuffer here, but the view will do that
  // itself when [_view layoutSubviews] is called.

  _fb_properties = eaglgsg->get_fb_properties();
  
  _properties.set_open(true);
  _properties.set_foreground(true);
  _is_valid = true;
  
  eaglgsg->unlock_context();
  
  return true;
}

/**
 * Handles both screen autorotation and the size of the app changing
 * due to iPad's split screen view. This is called from PandaEAGLView, which
 * internally holds a pointer to the window.
 */
void EAGLGraphicsWindow::
screen_size_changed() {
  EAGLGraphicsStateGuardian *eaglgsg;
  DCAST_INTO_V(eaglgsg, _gsg);
  
  eaglgsg->lock_context();
  [EAGLContext setCurrentContext:eaglgsg->_context];

  destroy_framebuffer(eaglgsg);
  create_framebuffer(eaglgsg);

  [EAGLContext setCurrentContext:nil];
  eaglgsg->unlock_context();

  WindowProperties properties;
  properties.set_size(_view.layer.bounds.size.width * _view.layer.contentsScale,
                      _view.layer.bounds.size.height * _view.layer.contentsScale);
  system_changed_properties(properties);
  
  eagldisplay_cat.debug() << "View size changed\n";
}

/**
 * Creates new buffers using _view's CAEAGLLayer. Assumes a lock on the context is already held.
 */
void EAGLGraphicsWindow::
create_framebuffer(EAGLGraphicsStateGuardian *gsg) {
  gsg->_glGenRenderbuffers(1, &_color_rb);
  gsg->_glGenFramebuffers(1, &_fbo);
  
  gsg->_glBindRenderbuffer(GL_RENDERBUFFER, _color_rb);
  gsg->_glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
  
  // The desired fbprops are already set on the layer itself.
  [gsg->_context renderbufferStorage:GL_RENDERBUFFER
                 fromDrawable:(CAEAGLLayer *)_view.layer];
  gsg->_glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, _color_rb);

  gsg->_glGenRenderbuffers(1, &_depth_stencil_rb);
  gsg->_glBindRenderbuffer(GL_RENDERBUFFER, _depth_stencil_rb);
  gsg->_glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                              _view.layer.bounds.size.width * _view.layer.contentsScale,
                              _view.layer.bounds.size.height * _view.layer.contentsScale);
  gsg->_glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, _depth_stencil_rb);
  
  gsg->_glBindRenderbuffer(GL_RENDERBUFFER, _color_rb);
}

/**
 * Destroy any buffers created by create_framebuffer(). Called mostly when
 * _view gets resized. Assumes a lock on the context is already held.
 */
void EAGLGraphicsWindow::
destroy_framebuffer(EAGLGraphicsStateGuardian *gsg) {
  if (_fbo != 0) {
    gsg->_glDeleteFramebuffers(1, &_fbo);
    _fbo = 0;
  }
  if (_color_rb != 0) {
    [gsg->_context renderbufferStorage:GL_RENDERBUFFER
                   fromDrawable:nil];
    _color_rb = 0;
  }
  if (_depth_stencil_rb != 0) {
    gsg->_glDeleteRenderbuffers(1, &_depth_stencil_rb);
    _depth_stencil_rb = 0;
  }
}

void EAGLGraphicsWindow::
close_window() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    EAGLGraphicsStateGuardian *eaglgsg = DCAST(EAGLGraphicsStateGuardian, _gsg);
    if (eaglgsg != NULL && eaglgsg->_context != nil) {
      eaglgsg->lock_context();
      destroy_framebuffer(eaglgsg);
      eaglgsg->unlock_context();
    }
  }

  GraphicsWindow::close_window();
}

void EAGLGraphicsWindow::
emulated_mouse_move(UITouch *touch) {
  CGPoint location = [touch locationInView:_view];
  _emulated_mouse_input->set_pointer_in_window(location.x * _view.layer.contentsScale,
                                               location.y * _view.layer.contentsScale);
}

void EAGLGraphicsWindow::
emulated_mouse_down(UITouch *touch) {
  CGPoint location = [touch locationInView:_view];
  _emulated_mouse_input->set_pointer_in_window(location.x * _view.layer.contentsScale,
                                               location.y * _view.layer.contentsScale);
  _emulated_mouse_input->button_down(MouseButton::button(0));
}

void EAGLGraphicsWindow::
emulated_mouse_up(UITouch *touch) {
  CGPoint location = [touch locationInView:_view];
  _emulated_mouse_input->set_pointer_in_window(location.x * _view.layer.contentsScale,
                                               location.y * _view.layer.contentsScale);
  _emulated_mouse_input->button_up(MouseButton::button(0));
}
