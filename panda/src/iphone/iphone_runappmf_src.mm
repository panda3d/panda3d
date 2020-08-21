/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file iphone_runappmf_src.mm
 * @author drose
 * @date 2009-04-26
 */

#import <UIKit/UIKit.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#include "pnotify.h"

#ifdef LINK_ALL_STATIC
extern "C" void initlibpandaexpress();
extern "C" void initlibpanda();
extern "C" void initlibpandaphysics();
extern "C" void initlibpandafx();
extern "C" void initlibdirect();
#endif  // LINK_ALL_STATIC

// @class AppMFViewController;
@interface AppMFAppDelegate : NSObject <UIApplicationDelegate> {
  NSString *app_directory;
  NSTimer *animationTimer;
  NSTimeInterval animationInterval;
}
@property (nonatomic, assign) NSString *app_directory;
@property (nonatomic, assign) NSTimer *animationTimer;
@property NSTimeInterval animationInterval;

- (void)startAnimation;
- (void)stopAnimation;
- (void)drawView;

@end

@implementation AppMFAppDelegate

@synthesize app_directory;
@synthesize animationTimer;
@synthesize animationInterval;

int startup = 0;

- (void)applicationDidFinishLaunching: (UIApplication *)application {
  // Get the App bundle directory.
  NSBundle *bundle = [NSBundle mainBundle];
  if (bundle != nil) {
    app_directory = [bundle bundlePath];
  }

  // Set this as the current directory.  Not only is this convenient, but it
  // also makes shared-library linking work.
  const char *app_directory_cstr = [app_directory cStringUsingEncoding: NSASCIIStringEncoding];
  int cd = chdir(app_directory_cstr);
  if (cd < 0) {
    perror("chdir");
  }

#ifdef LINK_ALL_STATIC
  // Ensure that all the relevant Panda modules are initialized.
  extern void init_libpanda();
  init_libpanda();

  // Ensure the IPhoneDisplay is available.
  extern void init_libiphonedisplay();
  init_libiphonedisplay();
#endif

  animationInterval = 1.0 / 60.0;
  [self startAnimation];
}

- (void)applicationDidReceiveMemoryWarning: (UIApplication *)application {
  cerr << "applicationDidReceiveMemoryWarning\n";
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
    // We are still just initializing the app.  Initialize Python now.  We
    // have this funny deferred-window technique, so SpringBoard will see that
    // the app has fully initialized and won't kill us if we take a while
    // starting up.

    Py_FrozenFlag = 1; /* Suppress errors from getpath.c */
    NSString *app_pathname = [app_directory stringByAppendingString: @"/iphone_runappmf" ];
    const char *app_pathname_cstr = [app_pathname cStringUsingEncoding: NSASCIIStringEncoding];
    Py_SetProgramName((char *)app_pathname_cstr);
    Py_Initialize();

    int argv = 1;
    NSString *script_pathname = [app_directory stringByAppendingString: @"/iphone.p3d" ];
    const char *script_pathname_cstr = [script_pathname cStringUsingEncoding: NSASCIIStringEncoding];
    char *argc[] = { (char *)script_pathname_cstr, NULL };
    PySys_SetArgv(argv, argc);

    const char *app_directory_cstr = [app_directory cStringUsingEncoding: NSASCIIStringEncoding];
    Py_SetPythonHome((char *)app_directory_cstr);

#ifdef LINK_ALL_STATIC
    // Construct the Python modules for the interrogate-generated data we know
    // we've already linked in.
    initlibpandaexpress();
    initlibpanda();
    initlibpandaphysics();
    initlibpandafx();
    initlibdirect();
#endif  // LINK_ALL_STATIC

    PyObject *module = PyImport_ImportModule("direct");
    if (module == (PyObject *)NULL) {
      cerr << "direct not importable\n";
      PyErr_Print();
      Py_Finalize();
      exit(1);
    }
    Py_DECREF(module);

    module = PyImport_ImportModule("direct.p3d");
    if (module == (PyObject *)NULL) {
      cerr << "direct.p3d not importable\n";
      PyErr_Print();
      Py_Finalize();
      exit(1);
    }
    Py_DECREF(module);

    module = PyImport_ImportModule("direct.p3d.runp3d");
    if (module == (PyObject *)NULL) {
      cerr << "runp3d not importable\n";
      PyErr_Print();
      Py_Finalize();
      exit(1);
    }
    Py_DECREF(module);

    startup = 1;

  } else if (startup == 1) {
    // Now that Python is initialized, call the startup function.
    PyObject *module = PyImport_ImportModule("direct.p3d.runp3d");
    if (module != (PyObject *)NULL) {
      PyObject *func = PyObject_GetAttrString(module, "runPackedApp");
      if (func != (PyObject *)NULL) {
        NSString *script_pathname = [app_directory stringByAppendingString: @"/iphone.p3d" ];
        const char *script_pathname_cstr = [script_pathname cStringUsingEncoding: NSASCIIStringEncoding];
        PyObject *result = PyObject_CallFunction(func, "(s)", script_pathname_cstr);
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
    // We are fully initialized and running.  Run taskMgr.step() once each
    // frame.
    PyObject *module = PyImport_ImportModule("direct.p3d.runp3d");
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
  /*
  int logfile_fd = open("/tmp/foo.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (logfile_fd >= 0) {
    dup2(logfile_fd, STDOUT_FILENO);
    dup2(logfile_fd, STDERR_FILENO);
    close(logfile_fd);
  }
  */

  PyImport_FrozenModules = _PyImport_FrozenModules;
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

  /* Call with the name of our application delegate class */
  int retVal = UIApplicationMain(argc, argv, nil, @"AppMFAppDelegate");

  [pool release];
  return retVal;
}
