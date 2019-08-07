/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file PandaEAGLView.m
 * @author D. Lawrence
 * @date 2019-01-04
 */

#import "pandaEAGLView.h"
#include "eaglGraphicsWindow.h"

@interface PandaEAGLView () {
  EAGLGraphicsWindow *_graphics_window;
}
@end

@implementation PandaEAGLView

/**
 * Initialize the view and backing layer.
 */
- (id)initWithFrame:(CGRect)frame graphicsWindow:(EAGLGraphicsWindow *)window {
  if ((self = [super initWithFrame:frame])) {

    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
    
    eaglLayer.opaque = YES;
    eaglLayer.contentsScale = 3.0;
    
    self.multipleTouchEnabled = YES;
    self.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;

    _graphics_window = window;
  }
  return self;
}

/**
 * This is triggered by the OS any time the size of the view changes.
 */
- (void)layoutSubviews {
  _graphics_window->screen_size_changed();
}

// TODO: Handle multi-touch.

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  _graphics_window->touches_moved(touches);
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  _graphics_window->touches_began(touches);
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  _graphics_window->touches_ended(touches);
}

/**
 * Triggered when the app loses focus while touching (an alert box from
 * the OS, for example).
 */
- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  // FIXME: Should there be separate handling for ended vs. cancelled touches?
  _graphics_window->touches_cancelled(touches);
}


/**
 * Specify we want a CAEAGLLayer to be the backing layer for this view. For
 * some reason you have to subclass UIView to do this.
 */
+ (Class) layerClass {
  return [CAEAGLLayer class];
}

@end
