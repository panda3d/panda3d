/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsGraphView.h
 * @author rdb
 * @date 2023-08-18
 */

#ifndef MACSTATSGRAPHVIEW_H
#define MACSTATSGRAPHVIEW_H

#include "cocoa_compat.h"

class MacStatsGraph;

@interface MacStatsGraphView : NSView<NSViewToolTipOwner> {
  @public
    MacStatsGraph *_graph;
}

- (id)initWithGraph:(MacStatsGraph *)graph;
- (void)drawRect:(NSRect)dirtyRect;

@end

#endif
