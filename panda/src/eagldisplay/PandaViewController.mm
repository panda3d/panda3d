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
- (void)queueForGraphicsWindowAttachmentWithCreation:(BOOL)createWindow;
@end

@implementation PandaViewController

- (void)startPythonApp:(NSString *)modulePath {
  [self startPythonApp:modulePath pythonRoot:@"Frameworks/python"];
}

- (void)startPythonApp:(NSString *)modulePath pythonRoot:(NSString *)pythonRoot {
  nassertv(modulePath != nil);
  nassertv(pythonRoot != nil);

  NSURL *url = [NSBundle.mainBundle bundleURL];

  const char *python_home = [NSURL URLWithString:pythonRoot relativeToURL:url].fileSystemRepresentation;
  wchar_t *wpython_home = Py_DecodeLocale(python_home, NULL);
  Py_SetPythonHome(wpython_home);

  // iOS provides a specific directory for temp files.
  NSString *tmp_path = [NSString stringWithFormat:@"TMP=%@", NSTemporaryDirectory(), nil];
  putenv((char *)[tmp_path UTF8String]);

  _fullModulePath = [NSURL URLWithString:modulePath relativeToURL:url];

  [self startPandaWithThreadFunction:@selector(pythonModuleMain)];
}

- (void)startCPPApp:(void *)mainFunction {
  nassertv(mainFunction != NULL);
  // (not yet implemented)
}

- (void)startPandaWithThreadFunction:(SEL)mainSelector {
  // When starting up the engine, a GraphicsWindow is created for us, so no need
  // to do so here.
  [self queueForGraphicsWindowAttachmentWithCreation:false];
  
  _pandaThread = [[NSThread alloc] initWithTarget:self selector:mainSelector object:nil];
  [_pandaThread start];
}

- (void)queueForGraphicsWindowAttachmentWithCreation:(BOOL)createWindow {
  EAGLGraphicsWindow::vc_lock.lock();
  while (EAGLGraphicsWindow::next_view_controller != nil) {
    EAGLGraphicsWindow::vc_condition.wait();
  }
  EAGLGraphicsWindow::next_view_controller = self;
  EAGLGraphicsWindow::vc_lock.unlock();
  if (createWindow) {
    // unimplemented
    abort();
  }
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
