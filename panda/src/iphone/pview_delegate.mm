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
#include "dcast.h"
#include "pandaFramework.h"
#include "config_iphonedisplay.h"

@implementation PviewAppDelegate 

@synthesize animationTimer;
@synthesize animationInterval;

int startup = 0;

PandaFramework framework;

int argc = 0;
char **argv = NULL;

void 
signal_handler(int i) {
  nout << "Caught signal " << i << "\n";
  exit(1);
}

- (void)applicationDidFinishLaunching: (UIApplication *)application { 
  ConfigVariableBool pview_trap_signals("pview-trap-signals", false);
  if (pview_trap_signals) {
    // Set up a signal handler on every signal, so we can report this to
    // the log.
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    for (int i = 1; i < 32; ++i) {
      if (sigaction(i, &sa, NULL) < 0) {
        ostringstream strm;
        strm << "sigaction(" << i << ")";
        perror(strm.str().c_str());
      }
    }
  }

  // Parse the "command-line arguments" provided in the Config file.
  ConfigVariableString pview_args("pview-args", "");
  argc = pview_args.get_num_words() + 1;
  typedef char *charp;
  argv = new charp[argc + 1];
  argv[0] = (char *)"pview";
  for (int i = 1; i < argc; ++i) {
   argv[i] = strdup(pview_args.get_word(i - 1).c_str());
  }
  argv[argc] = NULL;

  // Ensure the IPhoneDisplay is available.
  init_libiphonedisplay();
  
  framework.open_framework(argc, argv);
  startup = 0;

  ConfigVariableDouble timer_fps("timer-fps", 60.0);
  animationInterval = 1.0 / timer_fps;
  [self startAnimation];
} 

- (void)applicationDidReceiveMemoryWarning: (UIApplication *)application { 
  nout << "applicationDidReceiveMemoryWarning\n";
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
  if (startup == 0) {
    // We are still just initializing the app.  Open the window and
    // load the models.  We have this funny deferred-window technique,
    // so SpringBoard will see that the app has fully initialized and
    // won't kill us if we take a while loading models.
    WindowFramework *window = framework.open_window();
    if (window == (WindowFramework *)NULL) {
      // Couldn't get a window.
      framework.close_framework();
      exit(0);
    }
    startup = 1;

  } else if (startup == 1) {
    if (framework.get_num_windows() > 0) {
      WindowFramework *window = framework.get_window(0);
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
      
      ConfigVariableBool pview_lighting("pview-lighting", false);
      if (pview_lighting) {
        window->set_lighting(true);
      }
      
      ConfigVariableBool want_pstats("want-pstats", false);
      if (want_pstats) {
        PStatClient::connect();
      }
    }
    startup = 2;

  } else {
    // We are fully initialized and running.  Render frames.
    Thread *current_thread = Thread::get_current_thread();
    framework.do_frame(current_thread);
  }
}

- (void)applicationWillTerminate:
(UIApplication *)application
{
  [self stopAnimation];
  framework.close_framework();
}

- (void)dealloc { 
  [super dealloc]; 
} 

@end 
