/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsScaleArea.h
 * @author rdb
 * @date 2023-08-18
 */

#ifndef MACSTATSSCALEAREA_H
#define MACSTATSSCALEAREA_H

#import <Cocoa/Cocoa.h>

class MacStatsGraph;

@interface MacStatsScaleArea : NSView {
  @private
    MacStatsGraph *_graph;
}

- (id)initWithGraph:(MacStatsGraph *)graph frame:(NSRect)rect;
- (void)drawRect:(NSRect)dirtyRect;

@end

@interface MacStatsScaleAreaController : NSTitlebarAccessoryViewController {
  @private
    MacStatsGraph *_graph;
}

- (id)initWithGraph:(MacStatsGraph *)graph;
- (void)loadView;

@end

#endif
