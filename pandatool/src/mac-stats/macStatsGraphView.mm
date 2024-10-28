/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsGraphView.mm
 * @author rdb
 * @date 2023-08-18
 */

#include "macStatsGraphView.h"
#include "macStatsGraph.h"
#include "macStatsStripChart.h"
#include "macStatsFlameGraph.h"
#include "macStatsTimeline.h"
#include "macStatsScaleArea.h"

@implementation MacStatsGraphView

- (id)initWithGraph:(MacStatsGraph *)graph {
  if (self = [super init]) {
    _graph = graph;

    self.translatesAutoresizingMaskIntoConstraints = NO;

    NSTrackingArea *area = [[NSTrackingArea alloc] initWithRect:NSZeroRect options:(NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect) owner:self userInfo:nil];
    [self addTrackingArea:area];
    [area release];

    [self addToolTipRect:NSMakeRect(0, 0, 1000, 1000) owner:self userData:nil];
  }

  return self;
}

- (BOOL)isFlipped {
  return YES;
}

- (BOOL)acceptsFirstResponder {
  return YES;
}

- (void)keyDown:(NSEvent *)event {
  NSPoint pos = [self convertPoint:event.locationInWindow fromView:nil];
  UniChar c = 0;
  NSString *str = [event charactersIgnoringModifiers];
  if (str != nil && str.length == 1) {
    c = [str characterAtIndex:0];
  }
  _graph->handle_key(pos.x, pos.y, true, c, event.keyCode);
}

- (void)keyUp:(NSEvent *)event {
  NSPoint pos = [self convertPoint:event.locationInWindow fromView:nil];
  UniChar c = 0;
  NSString *str = [event charactersIgnoringModifiers];
  if (str != nil && str.length == 1) {
    c = [str characterAtIndex:0];
  }
  _graph->handle_key(pos.x, pos.y, false, c, event.keyCode);
}

- (void)mouseDown:(NSEvent *)event {
  NSPoint pos = [self convertPoint:event.locationInWindow fromView:nil];
  _graph->handle_button_press(pos.x, pos.y, event.clickCount > 1, event.buttonNumber);
}

- (void)mouseUp:(NSEvent *)event {
  NSPoint pos = [self convertPoint:event.locationInWindow fromView:nil];
  _graph->handle_button_release(pos.x, pos.y);
}

- (void)mouseDragged:(NSEvent *)event {
  NSPoint pos = [self convertPoint:event.locationInWindow fromView:nil];
  _graph->handle_motion(pos.x, pos.y);
}

- (void)mouseMoved:(NSEvent *)event {
  NSPoint pos = [self convertPoint:event.locationInWindow fromView:nil];
  _graph->handle_motion(pos.x, pos.y);
}

- (void)mouseExited:(NSEvent *)event {
  _graph->handle_leave();
}

- (void)scrollWheel:(NSEvent *)event {
  [super scrollWheel:event];
  if (event.deltaX != 0) {
    NSPoint pos = [self convertPoint:event.locationInWindow fromView:nil];
    _graph->handle_wheel(pos.x, pos.y, event.deltaX, event.deltaY);
  }
}

- (void)magnifyWithEvent:(NSEvent *)event {
  NSPoint pos = [self convertPoint:event.locationInWindow fromView:nil];
  _graph->handle_magnify(pos.x, pos.y, event.magnification);
}

- (void)handleTimer:(NSTimer *)timer {
  _graph->handle_timer();
}

- (void)viewDidChangeEffectiveAppearance {
  if (_graph != nullptr) {
    // Don't call this initially
    if (self.window != nil) {
      [NSAppearance setCurrentAppearance:self.effectiveAppearance];
      _graph->force_redraw();
    }
  }
}

- (void)drawRect:(NSRect)dirtyRect {
  if (_graph != nullptr) {
    CGContextRef ctx = [NSGraphicsContext currentContext].CGContext;
    _graph->handle_draw_graph(ctx, dirtyRect);
  }
}

// Not called when building with macOS 14 SDK due to change in clipsToBounds
// (but there it simply calls drawRect with the overhang region)
#if __MAC_OS_X_VERSION_MAX_ALLOWED < 140000
- (void)drawBackgroundOverhangInRect:(NSRect)dirtyRect {
  if (_graph != nullptr) {
    CGContextRef ctx = [NSGraphicsContext currentContext].CGContext;
    _graph->handle_draw_graph_overhang(ctx, dirtyRect);
  }
}
#endif

- (NSMenu *)menuForEvent:(NSEvent *)event {
  if (_graph != nullptr) {
    NSPoint pos = [self convertPoint:event.locationInWindow fromView:nil];
    return _graph->get_graph_menu(pos.x, pos.y);
  }
  return nil;
}

- (NSString *)view:(NSView *)view
  stringForToolTip:(NSToolTipTag)tag
             point:(NSPoint)point
          userData:(void *)data {

  if (_graph != nullptr) {
    std::string text = _graph->get_graph_tooltip(point.x, point.y);
    return [NSString stringWithUTF8String:text.c_str()];
  }
  return @"";
}

@end
