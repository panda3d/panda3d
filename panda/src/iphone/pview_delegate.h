// Filename: pview_delegate.h
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

#import <UIKit/UIKit.h> 

@class PviewViewController; 
@interface PviewAppDelegate : NSObject <UIApplicationDelegate> { 
  NSTimer *animationTimer;
  NSTimeInterval animationInterval;
} 
@property (nonatomic, assign) NSTimer *animationTimer;
@property NSTimeInterval animationInterval;

- (void)startAnimation;
- (void)stopAnimation;
- (void)drawView;

@end 
