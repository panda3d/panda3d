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

#import <objc/runtime.h>

#include "eaglGraphicsStateGuardian.h"
#include "mouseButton.h"

#include "config_eagldisplay.h"
// #import "iOSNSNotificationHandler.h"

TypeHandle EAGLGraphicsWindow::_type_handle;

PandaViewController *EAGLGraphicsWindow::next_view_controller = nil;
TrueMutexImpl EAGLGraphicsWindow::vc_lock;
TrueConditionVarImpl EAGLGraphicsWindow::vc_condition = TrueConditionVarImpl(EAGLGraphicsWindow::vc_lock);

// See https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ObjectiveC/Chapters/ocAssociativeReferences.html
// for what this is doing. Since UITouches don't come with an ID (or any member
// that can meaningfully be converted to one), we can attach our own using the
// Objective-C runtime.
static char touch_id_key;

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

  _input =
    GraphicsWindowInputDevice::pointer_only(this, "device");
  _input_devices.push_back(_input);

  _backing_buffer = new EAGLGraphicsBuffer(engine, pipe, "buffer", fb_prop, win_prop, flags, gsg, host);

  // Generate a set of identifiers that Panda will use to keep track of touches,
  // since UITouch does not provide an identifier. 5 is the maximum number of touches
  // an iPhone or iPad display can handle.
  int max_touches = 5;
  _touchIDPool = [NSMutableSet setWithCapacity:max_touches];
  for (int i = 0; i < max_touches; i++) {
    [_touchIDPool addObject:[NSNumber numberWithInt:i]];
  }
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
  
  return _backing_buffer->begin_frame(mode, current_thread);
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
  
  _backing_buffer->end_frame(mode, current_thread);
  
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
    glBindRenderbuffer(GL_RENDERBUFFER, _backing_buffer->_rb[RTP_color]);
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
    _backing_buffer->_gsg = _gsg;
  } else {
    // If the old gsg has the wrong pixel format, create a new one that shares
    // with the old gsg.
    DCAST_INTO_R(eaglgsg, _gsg, false);
    if (!eaglgsg->get_fb_properties().subsumes(_fb_properties)) {
      eaglgsg = new EAGLGraphicsStateGuardian(_engine, _pipe, eaglgsg);
      eaglgsg->choose_pixel_format(_fb_properties, (CAEAGLLayer *)_view.layer);
      _gsg = eaglgsg;
      _backing_buffer->_gsg = _gsg;
    }
  }
  
  if (eaglgsg->_context == nil) {
    // Could not obtain a proper context.
    _gsg.clear();
    close_window();
    return false;
  }
  
  // Make sure we grab the lock before we create the view so we can properly
  // reset the GSG before it tries to use it.
  eaglgsg->lock_context();

  // Create the view we're going to render into, and attach it to the supplied
  // view controller if given.
  PandaViewController *vc = EAGLGraphicsWindow::next_view_controller;
  dispatch_sync(dispatch_get_main_queue(), ^{
    CGRect frame = vc ? vc.view.frame : UIScreen.mainScreen.bounds;
    _view = [[PandaEAGLView alloc] initWithFrame:frame graphicsWindow:this];
    _backing_buffer->_layer = _view.layer;
    _properties.set_size(frame.size.width, frame.size.height);
    if (vc) {
      vc.view = _view;
    }
  });

  EAGLGraphicsWindow::next_view_controller = nil;
  EAGLGraphicsWindow::vc_lock.lock();
  EAGLGraphicsWindow::vc_condition.notify();
  EAGLGraphicsWindow::vc_lock.unlock();

  [EAGLContext setCurrentContext:eaglgsg->_context];
  
  eaglgsg->reset_if_new();
  
  // Usually we would create the framebuffer here, but the view will do that
  // itself when [_view layoutSubviews] is called.

  _fb_properties = eaglgsg->get_fb_properties();
  
  _properties.set_open(true);
  _properties.set_foreground(true);
  _is_valid = true;

  _backing_buffer->open_buffer();
  
  eaglgsg->unlock_context();
  
  return true;
}

void EAGLGraphicsWindow::
close_window() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    EAGLGraphicsStateGuardian *eaglgsg = DCAST(EAGLGraphicsStateGuardian, _gsg);
    if (eaglgsg != NULL && eaglgsg->_context != nil) {
      eaglgsg->lock_context();
      _backing_buffer->close_buffer();
      eaglgsg->unlock_context();
    }
  }

  GraphicsWindow::close_window();
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
  
  WindowProperties properties;
  properties.set_size(_view.layer.bounds.size.width * _view.layer.contentsScale,
                      _view.layer.bounds.size.height * _view.layer.contentsScale);
  system_changed_properties(properties);
  
  eaglgsg->lock_context();
  [EAGLContext setCurrentContext:eaglgsg->_context];

  _backing_buffer->set_size(properties.get_x_size(), properties.get_y_size());
  _backing_buffer->rebuild_bitplanes();

  [EAGLContext setCurrentContext:nil];
  eaglgsg->unlock_context();
  
  eagldisplay_cat.debug() << "View size changed\n";
}

void EAGLGraphicsWindow::
touches_began(NSSet<UITouch *> *touch_set) {
  [touch_set enumerateObjectsUsingBlock:^(UITouch *touch, BOOL *stop) {
    NSNumber *next_id = [_touchIDPool anyObject];
    [_touchIDPool removeObject:next_id];

    objc_setAssociatedObject(touch,
                             &touch_id_key,
                             next_id,
                             OBJC_ASSOCIATION_RETAIN_NONATOMIC);

    CGPoint point = [touch locationInView:_view];
    NSLog(@"%d, %d", point.x, point.y);
    float scale = _view.layer.contentsScale;

    _input->add_pointer(PointerType::finger, [next_id intValue], _primary_touch == nil);
    _input->update_pointer([next_id intValue], point.x * scale, point.y * scale, 1.0, PointerPhase::began);

    if (_primary_touch == nil) {
      _primary_touch = touch;
    }
  }];
}

void EAGLGraphicsWindow::
touches_moved(NSSet<UITouch *> *touch_set) {
  [touch_set enumerateObjectsUsingBlock:^(UITouch *touch, BOOL *stop) {
    NSNumber *touch_id = (NSNumber *)objc_getAssociatedObject(touch, &touch_id_key);

    CGPoint point = [touch locationInView:_view];
    float scale = _view.layer.contentsScale;

    _input->update_pointer([touch_id intValue], point.x * scale, point.y * scale, 1.0, PointerPhase::moved);
  }];
}

void EAGLGraphicsWindow::
touches_ended(NSSet<UITouch *> *touch_set) {
  [touch_set enumerateObjectsUsingBlock:^(UITouch *touch, BOOL *stop) {
    NSNumber *touch_id = (NSNumber *)objc_getAssociatedObject(touch, &touch_id_key);
    [_touchIDPool addObject:touch_id];
    
    objc_setAssociatedObject(touch,
                             &touch_id_key,
                             nil,
                             OBJC_ASSOCIATION_ASSIGN);

    PointerData &ptr = _input->get_pointer([touch_id intValue]);
    ptr.set_phase(PointerPhase::ended);

    _input->remove_pointer([touch_id intValue]);
    if (_primary_touch == touch) {
      _primary_touch = nil;
    }
  }];
}

void EAGLGraphicsWindow::
touches_cancelled(NSSet<UITouch *> *touch_set) {
  
}
