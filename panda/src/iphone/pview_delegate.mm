// Filename: pview_delegate.mm
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
    
#import "pview_delegate.h" 
#import "viewController.h"
#include "config_iphone.h"
#include "dcast.h"
#include "pandaFramework.h"

@implementation ControllerDemoAppDelegate 

@synthesize animationTimer;
@synthesize animationInterval;

PandaFramework framework;

- (void)applicationDidFinishLaunching:(UIApplication *)application { 
  int argc = 0;
  char **argv = NULL;
  framework.open_framework(argc, argv);

  WindowFramework *window = framework.open_window();
  if (window != (WindowFramework *)NULL) {
    window->enable_keyboard();
    window->setup_trackball();
    framework.get_models().instance_to(window->get_render());

    window->load_default_model(framework.get_models());
    window->center_trackball(framework.get_models());
  }

  animationInterval = 1.0 / 60.0;
  [self startAnimation];
} 

- (void)startAnimation {
    self.animationTimer = [NSTimer scheduledTimerWithTimeInterval:animationInterval target:self selector:@selector(drawView) userInfo:nil repeats:YES];
}


- (void)stopAnimation {
    self.animationTimer = nil;
}


- (void)setAnimationTimer:(NSTimer *)newTimer {
    [animationTimer invalidate];
    animationTimer = newTimer;
}


- (void)setAnimationInterval:(NSTimeInterval)interval {
    
    animationInterval = interval;
    if (animationTimer) {
        [self stopAnimation];
        [self startAnimation];
    }
}

- (void)drawView {
  Thread *current_thread = Thread::get_current_thread();
  framework.do_frame(current_thread);
 }

- (void)dealloc { 
  [self stopAnimation];
  framework.close_framework();
  [super dealloc]; 
} 

@end 
