/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsGraphViewController.h
 * @author rdb
 * @date 2023-08-28
 */

#ifndef MACSTATSGRAPHVIEWCONTROLLER_H
#define MACSTATSGRAPHVIEWCONTROLLER_H

#include "macStatsGraphView.h"

#import <Cocoa/Cocoa.h>

class MacStatsGraph;

@interface MacStatsGraphViewController : NSViewController<NSToolbarDelegate> {
  @protected
    MacStatsGraph *_graph;
}

- (id)initWithGraph:(MacStatsGraph *)graph;
- (MacStatsGraphView *)graphView;
- (BOOL)backToolbarItemVisible;
- (void)setBackToolbarItemVisible:(BOOL)show;

@end

@interface MacStatsScrollableGraphViewController : MacStatsGraphViewController

- (MacStatsGraphView *)graphView;
- (NSClipView *)clipView;

@end

#endif
