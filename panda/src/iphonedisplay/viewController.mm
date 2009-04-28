// Filename: viewController.mm
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

#import "viewController.h" 
#include "pnotify.h"
#include "iPhoneGraphicsPipe.h"
#include "config_iphonedisplay.h"

@implementation ControllerDemoViewController 

- (id)initWithPipe:
  (IPhoneGraphicsPipe *)pipe
{ 
  self = [ super init ]; 
  _pipe = pipe;
  return self; 
} 

- (BOOL)shouldAutorotateToInterfaceOrientation:
  (UIInterfaceOrientation)interfaceOrientation 
{ 
  return iphone_autorotate_view;
} 

- (void)didRotateFromInterfaceOrientation: 
  (UIInterfaceOrientation)fromInterfaceOrientation 
{ 
  _pipe->rotate_windows();
} 

- (void)viewDidLoad { 
  [ super viewDidLoad ]; 
  /* Add custom post-load code here */ 
} 

- (void)didReceiveMemoryWarning { 
  [ super didReceiveMemoryWarning ]; 
  /* Add custom low-memory code here */ 
  nout << "low-memory handler in view controller\n";
} 

- (void)dealloc { 
  [ super dealloc ]; 
} 

@end 
