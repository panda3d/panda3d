/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsScaleArea.mm
 * @author rdb
 * @date 2023-08-18
 */

#include "macStatsScaleArea.h"
#include "macStatsGraph.h"
#include "macStatsStripChart.h"

@implementation MacStatsScaleArea

- (id)initWithGraph:(MacStatsGraph *)graph frame:(NSRect)rect {
  if (self = [super initWithFrame:rect]) {
    _graph = graph;

    [self addTrackingArea:[[NSTrackingArea alloc] initWithRect:NSZeroRect options:(NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect) owner:self userInfo:nil]];
  }

  return self;
}

- (void)drawRect:(NSRect)dirtyRect {
  _graph->handle_draw_scale_area([NSGraphicsContext currentContext].CGContext, dirtyRect);
}

@end

@implementation MacStatsScaleAreaController

- (id)initWithGraph:(MacStatsGraph *)graph {
  if (self = [super init]) {
    _graph = graph;
  }

  return self;
}

- (void)loadView {
  self.view = [[MacStatsScaleArea alloc] initWithGraph:_graph frame:NSMakeRect(0, 0, 100, 100)];
}

@end
