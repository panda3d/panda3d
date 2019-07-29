#import "AppDelegate.h"

#include "PandaViewController.h"

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
  
  PandaViewController *vc = [[PandaViewController alloc] init];
  self.window.rootViewController = vc;
  
  [vc startPythonApp:[NSString stringWithFormat:@"${py_module_main}"]];
  
  return YES;
}

@end
