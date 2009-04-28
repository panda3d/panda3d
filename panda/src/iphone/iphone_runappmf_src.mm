// Filename: iphone_runappmf_src.mm
// Created by:  drose (26Apr09)
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
    
#import "viewController.h"
#include "dcast.h"
#include "config_iphonedisplay.h"

#ifdef LINK_ALL_STATIC
extern "C" void initlibpandaexpress();
extern "C" void initlibpanda();
extern "C" void initlibpandaphysics();
extern "C" void initlibpandafx();
extern "C" void initlibdirect();
#endif  // LINK_ALL_STATIC

@class AppMFViewController; 
@interface AppMFAppDelegate : NSObject <UIApplicationDelegate> { 
  NSTimer *animationTimer;
  NSTimeInterval animationInterval;
} 
@property (nonatomic, assign) NSTimer *animationTimer;
@property NSTimeInterval animationInterval;

- (void)startAnimation;
- (void)stopAnimation;
- (void)drawView;

@end 

@implementation AppMFAppDelegate 

@synthesize animationTimer;
@synthesize animationInterval;

int startup = 0;

- (void)applicationDidFinishLaunching: (UIApplication *)application { 
  // Ensure the IPhoneDisplay is available.
  init_libiphonedisplay();

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
    // We are still just initializing the app.  Initialize Python now.
    // We have this funny deferred-window technique, so SpringBoard
    // will see that the app has fully initialized and won't kill us
    // if we take a while starting up.

    Py_FrozenFlag = 1; /* Suppress errors from getpath.c */
    Filename app_pathname(Filename::get_app_directory(), "iphone_runmf");
    Py_SetProgramName((char *)app_pathname.c_str());
    Py_Initialize();

    int argv = 1;
    Filename script_pathname(Filename::get_app_directory(), "iphone.mf");
    char *argc[] = { (char *)script_pathname.c_str(), NULL };
    PySys_SetArgv(argv, argc);
    Py_SetPythonHome((char *)Filename::get_app_directory().c_str());

#ifdef LINK_ALL_STATIC
    // Construct the Python modules for the interrogate-generated data
    // we know we've already linked in.
    initlibpandaexpress();
    initlibpanda();
    initlibpandaphysics();
    initlibpandafx();
    initlibdirect();
#endif  // LINK_ALL_STATIC

    /*
    PyImport_ImportFrozenModule("direct");
    PyImport_ImportFrozenModule("direct.showbase");
    int n = PyImport_ImportFrozenModule("direct.showbase.RunAppMF");
    if (n == 0) {
      cerr << "RunAppMF not frozen in binary\n";
      Py_Finalize();
      exit(1);
    } else if (n < 0) {
      PyErr_Print();
      Py_Finalize();
      exit(1);
    }
    */

    PyObject *module = PyImport_ImportModule("direct");
    if (module == (PyObject *)NULL) {
      cerr << "direct not importable\n";
      Py_Finalize();
      exit(1);
    }
    Py_DECREF(module);

    module = PyImport_ImportModule("direct.showbase");
    if (module == (PyObject *)NULL) {
      cerr << "direct.showbase not importable\n";
      Py_Finalize();
      exit(1);
    }
    Py_DECREF(module);

    module = PyImport_ImportModule("direct.showbase.RunAppMF");
    if (module == (PyObject *)NULL) {
      cerr << "RunAppMF not importable\n";
      Py_Finalize();
      exit(1);
    }
    Py_DECREF(module);

    startup = 1;

  } else if (startup == 1) {
    // Now that Python is initialized, call the startup function.
    PyObject *module = PyImport_ImportModule("direct.showbase.RunAppMF");
    if (module != (PyObject *)NULL) {
      PyObject *func = PyObject_GetAttrString(module, "runPackedApp");
      if (func != (PyObject *)NULL) {
        Filename script_pathname(Filename::get_app_directory(), "iphone.mf");
        PyObject *result = PyObject_CallFunction(func, "([s])", script_pathname.c_str());
        Py_XDECREF(result);
        Py_DECREF(func);
      }
      Py_DECREF(module);
    }

    if (PyErr_Occurred()) {
      PyErr_Print();
      Py_Finalize();
      exit(1);
    }

    startup = 2;

  } else {
    // We are fully initialized and running.  Run taskMgr.step() once
    // each frame.
    PyObject *module = PyImport_ImportModule("__builtin__");
    if (module != (PyObject *)NULL) {
      PyObject *taskMgr = PyObject_GetAttrString(module, "taskMgr");
      if (taskMgr != (PyObject *)NULL) {
        PyObject *result = PyObject_CallMethod(taskMgr, "step", "()");
        Py_XDECREF(result);
        Py_DECREF(taskMgr);
      }
      Py_DECREF(module);
    }

    if (PyErr_Occurred()) {
      PyErr_Print();
      Py_Finalize();
      exit(1);
    }
  }
}

- (void)applicationWillTerminate:
(UIApplication *)application
{
  [self stopAnimation];
  Py_Finalize();
}

- (void)dealloc { 
  [super dealloc]; 
} 

@end 

extern "C" int main(int argc, char *argv[]);

int
main(int argc, char *argv[]) { 
  PyImport_FrozenModules = _PyImport_FrozenModules;
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]; 

  /* Call with the name of our application delegate class */ 
  int retVal = UIApplicationMain(argc, argv, nil, @"AppMFAppDelegate"); 
  [pool release]; 
  return retVal; 
} 
