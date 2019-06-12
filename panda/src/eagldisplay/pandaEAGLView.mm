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
 * Initialize the view and specify some options for the backing layer.
 */
- (id)initWithFrame:(CGRect)frame graphicsWindow:(EAGLGraphicsWindow *)window {
  if ((self = [super initWithFrame:frame])) {

    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
    
    eaglLayer.opaque = YES;
    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSNumber numberWithBool:NO],
                                    kEAGLDrawablePropertyRetainedBacking,
                                    kEAGLColorFormatRGBA8,
                                    kEAGLDrawablePropertyColorFormat, nil];
    eaglLayer.contentsScale = 3.0;
    
    _window = window;
  }
  return self;
}

- (void)layoutSubviews {
  NSLog(@"ARC enabled: %d", __has_feature(objc_arc));
  _window->screen_size_changed();
}

/**
 * Specify we want a CAEAGLLayer to be the backing layer for this view. For
 * some reason you have to subclass UIView to do this.
 */
+ (Class) layerClass {
  return [CAEAGLLayer class];
}

@end
