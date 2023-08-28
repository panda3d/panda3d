/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsLabel.mm
 * @author rdb
 * @date 2023-08-18
 */

#include "macStatsLabel.h"
#include "macStatsMonitor.h"
#include "macStatsGraph.h"

@implementation MacStatsLabel

- (id)initWithText:(NSString *)text
             graph:(MacStatsGraph *)graph
       threadIndex:(int)thread_index
    collectorIndex:(int)collector_index {
  if (self = [super init]) {
    _graph = graph;
    _thread_index = thread_index;
    _collector_index = collector_index;
    _bg_color = nil;
    _highlight_bg_color = nil;

    [self setStringValue:text];
    self.bezeled = NO;
    self.drawsBackground = YES;
    self.selectable = NO;
    self.editable = NO;
    self.lineBreakMode = NSLineBreakByTruncatingTail;
    //self.autoresizingMask = NSViewWidthSizable | NSViewMaxXMargin | NSViewMinXMargin;
    //self.translatesAutoresizingMaskIntoConstraints = NO;

    NSTrackingArea *area = [[NSTrackingArea alloc] initWithRect:NSZeroRect options:(NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect) owner:self userInfo:nil];
    [self addTrackingArea:area];
    [area release];

    [self addToolTipRect:NSMakeRect(0, 0, 1000, 1000) owner:self userData:nil];

    [self updateColor];
  }

  return self;
}

- (void)dealloc {
  [_bg_color release];
  [_highlight_bg_color release];
  [super dealloc];
}

- (NSSize)intrinsicContentSize {
  // Allow resizing down.
  NSSize size = [super intrinsicContentSize];
  return NSMakeSize(NSViewNoIntrinsicMetric, size.height);
}

- (int)threadIndex {
  return _thread_index;
}

- (int)collectorIndex {
  return _collector_index;
}

- (void)updateColor {
  if (_bg_color != nil) {
    [_bg_color release];
  }
  if (_highlight_bg_color != nil) {
    [_highlight_bg_color release];
  }

  _bg_color = [NSColor colorWithCGColor:_graph->get_monitor()->get_collector_color(_collector_index, false)];
  _highlight_bg_color = [NSColor colorWithCGColor:_graph->get_monitor()->get_collector_color(_collector_index, true)];

  [_bg_color retain];
  [_highlight_bg_color retain];

  _fg_color = _graph->get_monitor()->get_collector_text_color(_collector_index, false);
  _highlight_fg_color = _graph->get_monitor()->get_collector_text_color(_collector_index, true);

  [self setHighlight:_highlight];
}

- (BOOL)highlight {
  return _highlight;
}

- (void)setHighlight:(BOOL)highlight {
  _highlight = highlight;
  if (highlight || _mouse_within) {
    self.backgroundColor = _highlight_bg_color;
    self.textColor = _highlight_fg_color;
  } else {
    self.backgroundColor = _bg_color;
    self.textColor = _fg_color;
  }
}

- (void)mouseEntered:(NSEvent *)event {
  _mouse_within = true;
  self.backgroundColor = _highlight_bg_color;
  self.textColor = _highlight_fg_color;
}

- (void)mouseExited:(NSEvent *)event {
  _mouse_within = false;
  [self setHighlight:_highlight];
}

- (void)mouseDown:(NSEvent *)event {
  if (event.buttonNumber == 0 && event.clickCount == 2) {
    _graph->on_click_label(_collector_index);
  }
}

- (NSMenu *)menuForEvent:(NSEvent *)event {
  return _graph->get_label_menu(_collector_index);
}

- (NSString *)view:(NSView *)view
  stringForToolTip:(NSToolTipTag)tag
             point:(NSPoint)point
          userData:(void *)data {

  std::string text = _graph->get_label_tooltip(_collector_index);
  return [NSString stringWithUTF8String:text.c_str()];
}

@end
