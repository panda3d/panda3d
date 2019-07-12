#import "PandaViewController.h"

#include "Python.h"

#include "EAGLGraphicsWindow.h"
#include "graphicsEngine.h"
#include "asyncTaskManager.h"
#include "load_prc_file.h"
#include "throw_event.h"

@interface PandaViewController () {
  NSThread *_pandaThread;
  NSURL *_fullModulePath;
}
- (void)pythonModuleMain;
@end

@implementation PandaViewController

- (void)startPythonApp:(NSString *)modulePath {
  [self startPythonApp:modulePath pythonRoot:@"Python"]; 
}

- (void)startPythonApp:(NSString *)modulePath pythonRoot:(NSString *)pythonRoot {
  nassertv(modulePath != nil);
  nassertv(pythonRoot != nil);

  NSURL *resourceURL = [NSBundle.mainBundle resourceURL];

  const char *python_home = [NSURL URLWithString:pythonRoot relativeToURL:resourceURL].fileSystemRepresentation;
  wchar_t *wpython_home = Py_DecodeLocale(python_home, NULL);
  Py_SetPythonHome(wpython_home);

  // iOS provides a specific directory for temp files.
  NSString *tmp_path = [NSString stringWithFormat:@"TMP=%@", NSTemporaryDirectory(), nil];
  putenv((char *)[tmp_path UTF8String]);

  _fullModulePath = [NSURL URLWithString:modulePath relativeToURL:resourceURL];

  [self startPandaWithThreadFunction:@selector(pythonModuleMain)];
}

- (void)startCPPApp:(void *)mainFunction {
  nassertv(mainFunction != NULL);
  // (not yet implemented)
}

- (void)startPandaWithThreadFunction:(SEL)mainSelector {
  // TODO: Make the notification name a constant
  NSNotificationCenter *__weak notificationCenter = [NSNotificationCenter defaultCenter];
  id __block notificationToken = [notificationCenter addObserverForName:@"ViewCreatedNotification" object:nil queue:[NSOperationQueue mainQueue] usingBlock:^(NSNotification * _Nonnull note) {
    NSLog(@"Found the view!");
    self.view = note.userInfo[@"view"];
    self.view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [notificationCenter removeObserver:notificationToken];
  }];
  
  _pandaThread = [[NSThread alloc] initWithTarget:self selector:mainSelector object:nil];
  [_pandaThread start];
}

- (void)viewWillDisappear:(BOOL)animated {
  PyGILState_STATE gstate = PyGILState_Ensure();
  Py_Finalize();
}

- (void)pythonModuleMain {
  PT(Thread) panda_thread = Thread::bind_thread("ios_app", "ios_app");

  // TODO: Load a .prc file from bundle resources instead of hardocoding this.
  const char *model_path_config = [NSString stringWithFormat:@"model-path %@", [_fullModulePath.path stringByDeletingLastPathComponent]].UTF8String;
  
  load_prc_file_data("", "notify-level-gles2gsg debug");
  load_prc_file_data("", "notify-level-eagldisplay debug");
//  load_prc_file_data("", "notify-level-thread debug");
//  load_prc_file_data("", "notify-level-pipeline debug");
  load_prc_file_data("", "color-bits 8 8 8");
  load_prc_file_data("", "alpha-bits 8");
  load_prc_file_data("", "depth-bits 32");
  load_prc_file_data("", "notify-level-task debug");
  load_prc_file_data("", "framebuffer-srgb true");
  load_prc_file_data("", "threading-model Cull/Draw");
  load_prc_file_data("", "default-model-extension .egg");
  load_prc_file_data("", "assert-abort true");
//  load_prc_file_data("", "gl-check-errors true");
//  load_prc_file_data("", "basic-shaders-only true");
  load_prc_file_data("", model_path_config);
  
  Py_Initialize();
  
  FILE *fp = fopen(_fullModulePath.fileSystemRepresentation, "r");
  nassertv(fp != NULL);
  
  PyRun_SimpleFile(fp, _fullModulePath.lastPathComponent.UTF8String);
  
  Py_Finalize();
}

@end
