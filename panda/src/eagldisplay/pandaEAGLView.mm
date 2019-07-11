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
  EAGLGraphicsWindow *_window;
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
    
    _window = window;
  }
  return self;
}

/**
 * This is triggered by the OS any time the size of the view changes.
 */
- (void)layoutSubviews {
  _window->screen_size_changed();
}

// TODO: Handle multi-touch.

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  _window->emulated_mouse_move([touches anyObject]);
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  _window->emulated_mouse_down([touches anyObject]);
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  _window->emulated_mouse_up([touches anyObject]);
}

/**
 * Triggered when the app loses focus while touching (an alert box from
 * the OS, for example).
 */
- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  // FIXME: Should there be separate handling for ended vs. cancelled touches?
  _window->emulated_mouse_up([touches anyObject]);
}


/**
 * Specify we want a CAEAGLLayer to be the backing layer for this view. For
 * some reason you have to subclass UIView to do this.
 */
+ (Class) layerClass {
  return [CAEAGLLayer class];
}

@end
