// Filename: viewController.h
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

#include "pandabase.h"

#import <UIKit/UIKit.h> 
#import <UIKit/UITextView.h> 

class IPhoneGraphicsPipe;

@interface ControllerDemoViewController : UIViewController { 

@private
  IPhoneGraphicsPipe *_pipe;
} 

- (id)initWithPipe: (IPhoneGraphicsPipe *)pipe;
@end 
