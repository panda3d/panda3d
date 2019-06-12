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
  nassertr(eaglgsg->_context != nil, false);
  nassertr(_view != nil, false);
  
  eaglgsg->_context_lock.lock();
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
  eaglgsg->_context_lock.unlock();
  
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
    
    eaglgsg->_context_lock.lock();
    
    // Presents the primary renderbuffer to the screen.
    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
    [eaglgsg->_context presentRenderbuffer:GL_RENDERBUFFER];
    
    eaglgsg->_context_lock.unlock();
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
    eaglgsg->choose_pixel_format(_fb_properties, false);
    _gsg = eaglgsg;
  } else {
    // If the old gsg has the wrong pixel format, create a new one that shares
    // with the old gsg.
    DCAST_INTO_R(eaglgsg, _gsg, false);
    if (!eaglgsg->get_fb_properties().subsumes(_fb_properties)) {
      eaglgsg = new EAGLGraphicsStateGuardian(_engine, _pipe, eaglgsg);
      eaglgsg->choose_pixel_format(_fb_properties, false);
      _gsg = eaglgsg;
    }
  }
  
  if (eaglgsg->_context == nil) {
    // Could not obtain a proper context.
    _gsg.clear();
    close_window();
    return false;
  }
  
  // Just create the view. The application developer will have to attach it
  // to something themselves.
  // TODO: Make this more flexible
  dispatch_sync(dispatch_get_main_queue(), ^{
    _view = [[PandaEAGLView alloc] initWithFrame:UIScreen.mainScreen.bounds graphicsWindow:this];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"ViewCreatedNotification" object:nil userInfo:@{@"view": _view}];
  });

  [EAGLContext setCurrentContext:eaglgsg->_context];
  
  eaglgsg->reset_if_new();
  
  // Usually we would create the framebuffer here, but the view will do that itself when [_view layoutSubviews] is called.
  
  _fb_properties = eaglgsg->get_fb_properties();
  
  _properties.set_size(UIScreen.mainScreen.bounds.size.width,
                       UIScreen.mainScreen.bounds.size.height);
  _properties.set_open(true);
  _properties.set_foreground(true);
  _is_valid = true;
  
  return true;
}

/**
 * Handles both screen autorotation and the size of the app changing
 * due to iPad's split screen view.
 */
void EAGLGraphicsWindow::
screen_size_changed() {
  EAGLGraphicsStateGuardian *eaglgsg;
  DCAST_INTO_V(eaglgsg, _gsg);
  
  eaglgsg->_context_lock.lock();
  [EAGLContext setCurrentContext:eaglgsg->_context];

  destroy_framebuffer();
  create_framebuffer(eaglgsg);

  [EAGLContext setCurrentContext:nil];
  eaglgsg->_context_lock.unlock();

  WindowProperties properties;
  properties.set_size(_view.frame.size.width * 3, _view.frame.size.height * 3);
  system_changed_properties(properties);
  
  eagldisplay_cat.debug() << "View size changed\n";
}

/**
 * Creates new buffers using _view's CAEAGLLayer. Assumes a lock on the context is already held.
 */
void EAGLGraphicsWindow::
create_framebuffer(EAGLGraphicsStateGuardian *guardian) {
  glGenRenderbuffers(1, &_colorRenderbuffer);
  glGenFramebuffers(1, &_colorFramebuffer);
  
  glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, _colorFramebuffer);
  
  [guardian->_context renderbufferStorage:GL_RENDERBUFFER
                        fromDrawable:(CAEAGLLayer *)_view.layer];
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_RENDERBUFFER, _colorRenderbuffer);
  
  glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH,
                                         &_backingWidth);
  glGetRenderbufferParameteriv(GL_RENDERBUFFER,
                                         GL_RENDERBUFFER_HEIGHT,
                                         &_backingHeight);
  
  glGenRenderbuffers(1, &_depthRenderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16_OES,
                        _backingWidth, _backingHeight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, _depthRenderbuffer);
  
  glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
}

/**
 * Destroy any buffers created by create_framebuffer(). Called mostly when
 * _view gets resized. Assumes a lock on the context is already held.
 */
void EAGLGraphicsWindow::
destroy_framebuffer() {
  if (_colorFramebuffer != 0) {
    glDeleteFramebuffers(1, &_colorFramebuffer);
    _colorFramebuffer = 0;
  }
  if (_colorRenderbuffer != 0) {
    glDeleteRenderbuffers(1, &_colorRenderbuffer);
    _colorRenderbuffer = 0;
  }
  if (_depthRenderbuffer != 0) {
    glDeleteRenderbuffers(1, &_depthRenderbuffer);
    _depthRenderbuffer = 0;
  }
}

void EAGLGraphicsWindow::
close_window() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    EAGLGraphicsStateGuardian *eaglgsg = DCAST(EAGLGraphicsStateGuardian, _gsg);
    if (eaglgsg != NULL && eaglgsg->_context != nil) {
      eaglgsg->_context_lock.lock();
      destroy_framebuffer();
      eaglgsg->_context_lock.unlock();
    }
  }

  GraphicsWindow::close_window();
}
