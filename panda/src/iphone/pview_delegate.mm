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
  ConfigVariableString pview_args("pview-args", "");
  int argc = pview_args.get_num_words() + 1;
  typedef char *charp;
  char **argv = new charp[argc + 1];
  argv[0] = (char *)"pview";
  for (int i = 1; i < argc; ++i) {
    cerr << i << ". " << pview_args.get_word(i - 1) << "\n";
    argv[i] = strdup(pview_args.get_word(i - 1).c_str());
  }
  argv[argc] = NULL;

  framework.open_framework(argc, argv);
  
  WindowFramework *window = framework.open_window();
  if (window != (WindowFramework *)NULL) {
    window->enable_keyboard();
    window->setup_trackball();
    framework.get_models().instance_to(window->get_render());

    if (argc < 2) {
      window->load_default_model(framework.get_models());
    } else {
      window->load_models(framework.get_models(), argc, argv);
    }
    int hierarchy_match_flags = PartGroup::HMF_ok_part_extra |
                                PartGroup::HMF_ok_anim_extra;
    window->loop_animations(hierarchy_match_flags);
    
    window->center_trackball(framework.get_models());

    ConfigVariableBool want_pstats("want-pstats", false);
    if (want_pstats) {
      PStatClient::connect();
    }
  }

  ConfigVariableDouble timer_fps("timer-fps", 60.0);
  animationInterval = 1.0 / timer_fps;
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
