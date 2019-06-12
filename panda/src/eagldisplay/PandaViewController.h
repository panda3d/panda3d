#import <UIKit/UIKit.h>

@interface PandaViewController : UIViewController

// Whether this instance is functioning as the main GraphicsWindow.
@property BOOL mainWindow;

- (void)startPythonApp:(NSString *)modulePath;
- (void)startPythonApp:(NSString *)modulePath pythonRoot:(NSString *)pythonRoot;
- (void)startCPPApp:(void *)mainFunction;
- (void)startPandaWithThreadFunction:(SEL)mainSelector;

@end
