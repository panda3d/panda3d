/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file PandaEAGLView.h
 * @author D. Lawrence
 * @date 2019-01-04
 */

#import <UIKit/UIKit.h>

class EAGLGraphicsWindow;

/**
 * A small subclass of UIView that sets up a CAEAGLLayer as the view's backing
 * layer, and sets a few options on it.
 */
@interface PandaEAGLView : UIView

- (id)initWithFrame:(CGRect)frame graphicsWindow:(EAGLGraphicsWindow *)window;

@end
