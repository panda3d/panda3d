#import "AppDelegate.h"

#include "PandaViewController.h"

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
  
  self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];

  PandaViewController *vc = [[PandaViewController alloc] init];
  self.window.rootViewController = vc;
  
  [vc startPythonApp:@"${py_module_main}"];

  [self.window makeKeyAndVisible];
  
  return YES;
}

@end
