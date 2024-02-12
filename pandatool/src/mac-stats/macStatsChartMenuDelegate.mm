/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsChartMenuDelegate.mm
 * @author rdb
 * @date 2023-08-18
 */

#include "macStatsChartMenuDelegate.h"
#include "macStatsMonitor.h"

@implementation MacStatsChartMenuDelegate

- (id)initWithMonitor:(MacStatsMonitor *)monitor threadIndex:(int)index {
  if (self = [super init]) {
    _monitor = monitor;
    _thread_index = index;
  }

  return self;
}

- (void)handleOpenTimeline:(NSMenuItem *)item {
  _monitor->open_timeline();
}

- (void)handleOpenStripChart:(NSMenuItem *)item {
  _monitor->open_strip_chart(_thread_index, item.tag, NO);
}

- (void)handleOpenStripChartLevel:(NSMenuItem *)item {
  _monitor->open_strip_chart(_thread_index, item.tag, YES);
}

- (void)handleOpenFlameGraph:(NSMenuItem *)item {
  _monitor->open_flame_graph(_thread_index, item.tag);
}

- (void)handleOpenPianoRoll:(NSMenuItem *)item {
  _monitor->open_piano_roll(_thread_index);
}

- (void)handleCloseAllGraphs:(NSMenuItem *)item {
  _monitor->close_all_graphs();
}

- (void)handleReopenDefaultGraphs:(NSMenuItem *)item {
  _monitor->close_all_graphs();
  _monitor->open_default_graphs();
}

- (void)handleSaveDefaultGraphs:(NSMenuItem *)item {
  _monitor->save_default_graphs();
}

@end
