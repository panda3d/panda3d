// Filename: eaglView.h
// Created by:  drose (10Apr09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

class IPhoneGraphicsWindow;

/*
This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
The view content is basically an EAGL surface you render your OpenGL scene into.
Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
*/
@interface EAGLView : UIView {

@public
  IPhoneGraphicsWindow *_window;
@private

  /* The pixel dimensions of the backbuffer */
  GLint backingWidth;
 GLint backingHeight;
 
 EAGLContext *context;
 
 /* OpenGL names for the renderbuffer and framebuffers used to render to this view */
 GLuint viewRenderbuffer, viewFramebuffer;
 
 /* OpenGL name for the depth buffer that is attached to viewFramebuffer, if it exists (0 if it does not exist) */
 GLuint depthRenderbuffer;
}

- (void)presentView;

@end
