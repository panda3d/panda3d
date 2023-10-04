/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsLabel.h
 * @author rdb
 * @date 2023-08-18
 */

#ifndef MACSTATSLABEL_H
#define MACSTATSLABEL_H

#include "pandatoolbase.h"
#include "luse.h"

#include "cocoa_compat.h"

class MacStatsMonitor;
class MacStatsGraph;

/**
 * A text label that will draw in color appropriate for a particular
 * collector.  It also responds when the user double-clicks on it.  This is
 * handy for putting colored labels on strip charts.
 */
@interface MacStatsLabel : NSTextField<NSViewToolTipOwner> {
  @private
    MacStatsGraph *_graph;
    int _thread_index;
    int _collector_index;
    bool _highlight;
    bool _mouse_within;
    NSColor *_fg_color;
    NSColor *_highlight_fg_color;
    NSColor *_bg_color;
    NSColor *_highlight_bg_color;
}

- (id)initWithText:(NSString *)text
             graph:(MacStatsGraph *)graph
       threadIndex:(int)thread_index
    collectorIndex:(int)collector_index;

- (int)threadIndex;
- (int)collectorIndex;

- (void)updateColor;

- (BOOL)highlight;
- (void)setHighlight:(BOOL)highlight;

@end

#endif
