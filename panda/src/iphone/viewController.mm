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

@implementation ControllerDemoViewController 

- (id)init { 

    self = [ super init ]; 

    /*
    if (self != nil) { 
        helloWorld = [ [ NSString alloc ] initWithString: @"Hello, World!" ]; 
        woahDizzy = [ [ NSString alloc ] initWithString: @"Woah, I'm Dizzy!" ]; 
    } 
    */

    return self; 
} 

- (void)loadView { 

    [ super loadView ]; 

    /*
    glView = [ [ EAGLView alloc ] initWithFrame: 
        [ [ UIScreen mainScreen ] applicationFrame ] 
    ];
    //    [ glView startAnimation ];

   self.view = glView; 
    */
} 

/*
-(BOOL)shouldAutorotateToInterfaceOrientation: 
(UIInterfaceOrientation)interfaceOrientation 
{ 
  return YES; 
} 
*/

-  (void)didRotateFromInterfaceOrientation: 
(UIInterfaceOrientation)fromInterfaceOrientation 
{ 
} 

- (void)viewDidLoad { 
    [ super viewDidLoad ]; 
    /* Add custom post-load code here */ 
} 

- (void)didReceiveMemoryWarning { 
    [ super didReceiveMemoryWarning ]; 
    /* Add custom low-memory code here */ 
} 

- (void)dealloc { 
    /* Here, the objects we've allocated are released */ 
  //[ helloWorld release ]; 
  //    [ woahDizzy release ]; 
  //    [ glView release ]; 
  [ super dealloc ]; 
} 

@end 
