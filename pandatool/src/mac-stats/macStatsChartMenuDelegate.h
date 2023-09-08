/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsChartMenuDelegate.h
 * @author rdb
 * @date 2023-08-18
 */

#ifndef MACSTATSCHARTMENUDELEGATE_H
#define MACSTATSCHARTMENUDELEGATE_H

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

class MacStatsMonitor;

@interface MacStatsChartMenuDelegate : NSObject<NSMenuDelegate> {
  @private
    MacStatsMonitor *_monitor;
    int _thread_index;
}

- (id)initWithMonitor:(MacStatsMonitor *)monitor threadIndex:(int)index;

@end

#endif
