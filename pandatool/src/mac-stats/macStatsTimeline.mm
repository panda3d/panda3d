/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsTimeline.mm
 * @author rdb
 * @date 2023-08-19
 */

#include "macStatsTimeline.h"
#include "macStatsMonitor.h"
#include "macStatsLabelStack.h"
#include "macStatsScaleArea.h"
#include "pStatCollectorDef.h"

@interface MacStatsTimelineViewController : MacStatsScrollableGraphViewController
@end

static const int default_timeline_width = 1000;
static const int default_timeline_height = 300;

static const int minimum_timeline_sidebar_width = 68;
static const int default_timeline_sidebar_width = 100;

/**
 *
 */
MacStatsTimeline::
MacStatsTimeline(MacStatsMonitor *monitor) :
  PStatTimeline(monitor, 0, 0),
  MacStatsGraph(monitor, [MacStatsTimelineViewController alloc])
{
  // Let's show the units on the guide bar labels.  There's room.
  set_guide_bar_units(get_guide_bar_units() | GBU_show_units);

  _window.title = @"Timeline";

  if (@available(macOS 11.0, *)) {
    _window.titleVisibility = NSWindowTitleHidden;
  }

  // Set the initial size of the graph.
  const ThreadRow &last_thread_row = _threads.back();
  int height = row_to_pixel(last_thread_row._row_offset + last_thread_row._rows.size());
  if (height < default_timeline_height) {
    height = default_timeline_height;
  }
  height += _window.frame.size.height - _window.contentLayoutRect.size.height;
  _graph_view.frame = NSMakeRect(0, 0, default_timeline_width, height);
  _graph_view_controller.view.frame = NSMakeRect(0, 0, default_timeline_width, height);

  // Add a drawing area to the left of the graph to show the thread labels.
  _thread_area = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, default_timeline_sidebar_width, 0)];
  _thread_area.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
  _thread_area.translatesAutoresizingMaskIntoConstraints = NO;

  // It's put inside a scroll view that tracks the main scroll view.
  NSScrollView *scroll_view = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, default_timeline_sidebar_width, 0)];
  scroll_view.documentView = _thread_area;
  scroll_view.drawsBackground = NO;
  scroll_view.automaticallyAdjustsContentInsets = YES;
  //scroll_view.translatesAutoresizingMaskIntoConstraints = NO;
  scroll_view.hasHorizontalScroller = NO;
  scroll_view.hasVerticalScroller = NO;
  _sidebar_scroll_view = scroll_view;

  NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
  [center addObserver:_graph_view_controller
             selector:@selector(handleSideScroll:)
                 name:NSScrollViewDidLiveScrollNotification
               object:scroll_view];

  NSViewController *sidebar_controller = [[NSViewController alloc] init];
  sidebar_controller.view = scroll_view;

  NSSplitViewController *svc = [[NSSplitViewController alloc] init];
  [svc addSplitViewItem:[NSSplitViewItem sidebarWithViewController:sidebar_controller]];
  [svc addSplitViewItem:[NSSplitViewItem splitViewItemWithViewController:_graph_view_controller]];

  svc.splitViewItems[0].minimumThickness = minimum_timeline_sidebar_width;
  svc.splitViewItems[0].canCollapse = NO;

  NSSplitView *split_view = svc.splitView;
  split_view.vertical = YES;
  split_view.dividerStyle = NSSplitViewDividerStyleThin;
  split_view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
  _split_view = split_view;

  _window.contentViewController = svc;

  [svc release];
  [sidebar_controller release];

  // Scale area goes on top, as a titlebar accessory view.
  MacStatsScaleAreaController *scale_area_controller = [[MacStatsScaleAreaController alloc] initWithGraph:this];
  scale_area_controller.fullScreenMinHeight = 20;
  scale_area_controller.layoutAttribute = NSLayoutAttributeRight;
  _scale_area = scale_area_controller.view;
  [_window addTitlebarAccessoryViewController:scale_area_controller];
  [scale_area_controller release];

  [_thread_area.widthAnchor constraintEqualToAnchor:scroll_view.widthAnchor].active = YES;
  [_thread_area.heightAnchor constraintEqualToAnchor:_graph_view.heightAnchor].active = YES;

  _graph_height_constraint = [_graph_view.heightAnchor constraintGreaterThanOrEqualToConstant:0];
  _graph_height_constraint.active = YES;

  [_window makeKeyAndOrderFront:nil];
}

/**
 *
 */
MacStatsTimeline::
~MacStatsTimeline() {
  [_thread_area release];
  [_sidebar_scroll_view release];
}

/**
 * Called as each frame's data is made available.  There is no guarantee the
 * frames will arrive in order, or that all of them will arrive at all.  The
 * monitor should be prepared to accept frames received out-of-order or
 * missing.
 */
void MacStatsTimeline::
new_data(int thread_index, int frame_number) {
  PStatTimeline::new_data(thread_index, frame_number);
}

/**
 * Called when it is necessary to redraw the entire graph.
 */
void MacStatsTimeline::
force_redraw() {
  if (_ctx) {
    PStatTimeline::force_redraw();
  }
}

/**
 * Called when the user has resized the window, forcing a resize of the graph.
 */
void MacStatsTimeline::
changed_graph_size(int graph_xsize, int graph_ysize) {
  PStatTimeline::changed_size(graph_xsize, graph_ysize);
}

/**
 * Erases the chart area.
 */
void MacStatsTimeline::
clear_region() {
  if (_ctx) {
    CGContextSetFillColorWithColor(_ctx, _background_color);
    CGContextFillRect(_ctx, CGRectMake(0, 0, get_xsize(), get_ysize()));

    CGContextSetTextMatrix(_ctx, CGAffineTransformMakeScale(1, -1));
    //draw_guide_labels(_ctx);
  }
}

/**
 * Erases the chart area in preparation for drawing a bunch of bars.
 */
void MacStatsTimeline::
begin_draw() {
}

/**
 * Draws a horizontal separator.
 */
void MacStatsTimeline::
draw_separator(int row) {
  if (_ctx) {
    CGContextSetFillColorWithColor(_ctx, [NSColor gridColor].CGColor);
    CGContextFillRect(_ctx, CGRectMake(0, (row_to_pixel(row) + row_to_pixel(row + 1)) / 2.0, get_xsize(), 4.0 / 3.0));
  }
}

/**
 * Draws a vertical guide bar.  If the row is -1, draws it in all rows.
 */
void MacStatsTimeline::
draw_guide_bar(int x, GuideBarStyle style) {
  draw_guide_bar(_ctx, style, x, 0, get_ysize());
}

/**
 * Draws a single bar in the chart for the indicated row, in the color for the
 * given collector, for the indicated horizontal pixel range.
 */
void MacStatsTimeline::
draw_bar(int row, int from_x, int to_x, int collector_index,
         const std::string &collector_name) {
  int top = row_to_pixel(row);
  int bottom = row_to_pixel(row + 1);
  int scale = 4;

  top += 1;

  MacStatsMonitor *monitor = MacStatsGraph::_monitor;

  bool is_highlighted = row == _highlighted_row && _highlighted_x >= from_x && _highlighted_x < to_x;
  CGContextSetFillColorWithColor(_ctx,
    monitor->get_collector_color(collector_index, is_highlighted));

  if (to_x < from_x + 1) {
    // Too tiny to draw.
  }
  else if (to_x < from_x + scale) {
    // It's just a tiny sliver.  This is a more reliable way to draw it.
    CGRect rect = CGRectMake(from_x, top, to_x - from_x, bottom - top);
    CGContextFillRect(_ctx, rect);
  }
  else {
    int left = std::max(from_x, -scale - 1);
    int right = std::min(std::max(to_x, from_x + 1), get_xsize() + scale);

    double radius = std::min((double)scale, (right - left) / 2.0);
    CGContextBeginPath(_ctx);
    CGContextAddArc(_ctx, right - radius - 0.5, top + radius, radius, -0.5 * M_PI, 0.0, NO);
    CGContextAddArc(_ctx, right - radius - 0.5, bottom - radius, radius, 0.0, 0.5 * M_PI, NO);
    CGContextAddArc(_ctx, left + radius, bottom - radius, radius, 0.5 * M_PI, M_PI, NO);
    CGContextAddArc(_ctx, left + radius, top + radius, radius, M_PI, 1.5 * M_PI, NO);
    CGContextClosePath(_ctx);
    CGContextFillPath(_ctx);

    if ((to_x - from_x) >= scale * 4) {
      // Only bother drawing the text if we've got some space to draw on.
      const CFStringRef keys[] = {
        (__bridge CFStringRef)NSForegroundColorAttributeName,
        (__bridge CFStringRef)NSFontAttributeName,
      };
      const void *values[] = {
        monitor->get_collector_text_color(collector_index, is_highlighted),
        [NSFont systemFontOfSize:0.0],
      };
      CFDictionaryRef attribs = CFDictionaryCreate(kCFAllocatorDefault, (const void **)keys, (const void **)values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

      CFStringRef str = CFStringCreateWithCString(kCFAllocatorDefault, collector_name.c_str(), kCFStringEncodingUTF8);
      CFAttributedStringRef astr = CFAttributedStringCreate(kCFAllocatorDefault, str, attribs);

      CTLineRef line = CTLineCreateWithAttributedString(astr);
      CGRect bounds = CTLineGetImageBounds(line, _ctx);
      CFRelease(astr);
      CFRelease(str);

      int text_width = bounds.size.width;
      int text_height = bounds.size.height;

      double center = (from_x + to_x) / 2.0;
      double text_left = std::max(from_x, 0) + scale / 2.0;
      double text_right = std::min(to_x, get_xsize()) - scale / 2.0;
      double text_top = top + (bottom - top - text_height) / 2.0 + text_height;

      if (text_width >= text_right - text_left) {
        size_t c = collector_name.rfind(':');
        if (text_right - text_left < scale * 6) {
          // It's a really tiny space.  Draw a single letter.
          UniChar ch = *(collector_name.data() + (c != std::string::npos ? c + 1 : 0));

          CFStringRef str = CFStringCreateWithCharacters(kCFAllocatorDefault, &ch, 1);
          CFAttributedStringRef astr = CFAttributedStringCreate(kCFAllocatorDefault, str, attribs);

          CTLineRef new_line = CTLineCreateWithAttributedString((CFAttributedStringRef)astr);
          bounds = CTLineGetImageBounds(new_line, _ctx);
          text_width = bounds.size.width;

          CFRelease(line);
          CFRelease(astr);
          CFRelease(str);
          line = new_line;
        }
        else {
          // Maybe just use everything after the last colon.
          if (c != std::string::npos) {
            const char *short_name = collector_name.data() + c + 1;
            CFStringRef str = CFStringCreateWithCString(kCFAllocatorDefault, short_name, kCFStringEncodingUTF8);
            CFAttributedStringRef astr = CFAttributedStringCreate(kCFAllocatorDefault, str, attribs);

            CTLineRef new_line = CTLineCreateWithAttributedString((CFAttributedStringRef)astr);
            bounds = CTLineGetImageBounds(new_line, _ctx);
            text_width = bounds.size.width;

            CFRelease(line);
            CFRelease(astr);
            CFRelease(str);
            line = new_line;
          }
        }
      }

      if (text_width >= text_right - text_left) {
        // Have CoreText truncate to the correct length.
        static CFStringRef token_str = CFSTR("\u2026");
        CFAttributedStringRef token_astr = CFAttributedStringCreate(kCFAllocatorDefault, token_str, attribs);
        CTLineRef token_line = CTLineCreateWithAttributedString(token_astr);
        CTLineRef trunc_line = CTLineCreateTruncatedLine(line, text_right - text_left, kCTLineTruncationEnd, token_line);
        CFRelease(line);
        CFRelease(token_astr);
        CFRelease(token_line);
        line = trunc_line;
        CGContextSetTextPosition(_ctx, text_left, text_top);
      }
      else if (center - text_width / 2.0 < 0.0) {
        // Put it against the left-most edge.
        CGContextSetTextPosition(_ctx, scale, text_top);
      }
      else if (center + text_width / 2.0 >= get_xsize()) {
        // Put it against the right-most edge.
        CGContextSetTextPosition(_ctx, get_xsize() - scale - text_width, text_top);
      }
      else {
        // It fits just fine, center it.
        CGContextSetTextPosition(_ctx, center - text_width / 2.0, text_top);
      }

      if (line != nullptr) {
        CTLineDraw(line, _ctx);
        CFRelease(line);
      }
      CFRelease(attribs);
    }
  }
}

/**
 * Called after all the bars have been drawn, this triggers a refresh event to
 * draw it to the window.
 */
void MacStatsTimeline::
end_draw() {
  // Recalculate the size of the graph.
  if (!_threads.empty()) {
    const ThreadRow &last_thread_row = _threads.back();
    int new_height = row_to_pixel(last_thread_row._row_offset + last_thread_row._rows.size());
    _graph_height_constraint.constant = new_height;
  }

  _graph_view.needsDisplay = YES;

  // If we scroll sideways while we're also scrolling vertically such that the
  // overhang becomes visible due to elasticity, the overhang doesn't update.
  // I could only find this private method for fixing this problem.
  // Not needed as of macOS 14 and up, since the normal drawRect DTRT there.
#if __MAC_OS_X_VERSION_MAX_ALLOWED < 140000
  NSClipView *clip_view = ((MacStatsScrollableGraphViewController *)_graph_view_controller).clipView;
  if ([clip_view respondsToSelector:@selector(_setNeedsDisplayInOverhang:)]) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-method-access"
    [clip_view _setNeedsDisplayInOverhang:YES];
#pragma clang diagnostic pop
  }
#endif

  if (_threads_changed) {
    while (_thread_labels.size() < _threads.size()) {
      NSTextField *label = [NSTextField labelWithString:@"Thread"];
      label.translatesAutoresizingMaskIntoConstraints = NO;
      [_thread_area addSubview:label];

      [label.rightAnchor constraintEqualToAnchor:_thread_area.rightAnchor constant:-8].active = YES;
      [_thread_area.widthAnchor constraintGreaterThanOrEqualToAnchor:label.widthAnchor constant:16].active = YES;

      NSLayoutConstraint *constraint;
      if (@available(macOS 11.0, *)) {
        constraint = [label.topAnchor constraintEqualToAnchor:_thread_area.topAnchor];
      } else {
        constraint = [label.topAnchor constraintEqualToAnchor:_thread_area.topAnchor];
      }
      constraint.active = YES;

      _thread_labels.push_back(std::make_pair(label, constraint));
    }

    for (size_t i = 0; i < _threads.size(); ++i) {
      const ThreadRow &thread_row = _threads[i];
      NSTextField *label = _thread_labels[i].first;
      NSLayoutConstraint *label_constraint = _thread_labels[i].second;

      label.stringValue = [NSString stringWithUTF8String:thread_row._label.c_str()];
      label_constraint.constant = row_to_pixel(thread_row._row_offset);
    }

    _thread_area.needsDisplay = YES;
    _threads_changed = false;
  }

  if (_guide_bars_changed) {
    _scale_area.needsDisplay = YES;
    _guide_bars_changed = false;
  }
}

/**
 * Called at the end of the draw cycle.
 */
void MacStatsTimeline::
idle() {
}

/**
 * Overridden by a derived class to implement an animation.  If it returns
 * false, the animation timer is stopped.
 */
bool MacStatsTimeline::
animate(double time, double dt) {
  return PStatTimeline::animate(time, dt);
}

/**
 * Returns the current window dimensions.
 */
bool MacStatsTimeline::
get_window_state(int &x, int &y, int &width, int &height,
                 bool &maximized, bool &minimized) const {
  MacStatsGraph::get_window_state(x, y, width, height, maximized, minimized);
  return true;
}

/**
 * Called to restore the graph window to its previous dimensions.
 */
void MacStatsTimeline::
set_window_state(int x, int y, int width, int height,
                 bool maximized, bool minimized) {
  MacStatsGraph::set_window_state(x, y, width, height, maximized, minimized);
}

/**
 * Called when the mouse right-clicks on the graph, and should return the menu
 * that should pop up.
 */
NSMenu *MacStatsTimeline::
get_graph_menu(int graph_x, int graph_y) const {
  int row = pixel_to_row(graph_y);
  ColorBar bar;
  if (!find_bar(row, graph_x, bar)) {
    return nil;
  }

  _popup_bar = bar;

  NSMenu *menu = [[[NSMenu alloc] init] autorelease];

  std::string label = get_bar_tooltip(row, graph_x);
  if (!label.empty()) {
    if (@available(macOS 14.0, *)) {
      [menu addItem:[NSMenuItem sectionHeaderWithTitle:[NSString stringWithUTF8String:label.c_str()]]];
    } else {
      NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:label.c_str()] action:nil keyEquivalent:@""];
      item.enabled = NO;
      [menu addItem:item];
      [item release];
    }
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"Zoom To" action:@selector(handleZoomTo:) keyEquivalent:@""];
    item.target = _graph_view_controller;
    [menu addItem:item];
    [item release];
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"Open Strip Chart" action:@selector(handleOpenStripChart:) keyEquivalent:@""];
    item.target = _graph_view_controller;
    [menu addItem:item];
    [item release];
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"Open Flame Graph" action:@selector(handleOpenFlameGraph:) keyEquivalent:@""];
    item.target = _graph_view_controller;
    [menu addItem:item];
    [item release];
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"Open Piano Roll" action:@selector(handleOpenPianoRoll:) keyEquivalent:@""];
    item.target = _graph_view_controller;
    [menu addItem:item];
    [item release];
  }

  [menu addItem:[NSMenuItem separatorItem]];

  {
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"Change Color\u2026" action:@selector(handleChangeColor:) keyEquivalent:@""];
    item.target = _graph_view_controller;
    item.tag = bar._collector_index;
    [menu addItem:item];
    [item release];
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"Reset Color" action:@selector(handleResetColor:) keyEquivalent:@""];
    item.target = _graph_view_controller;
    item.tag = bar._collector_index;
    [menu addItem:item];
    [item release];
  }

  return menu;
}

/**
 * Called when the mouse hovers over the graph, and should return the text that
 * should appear on the tooltip.
 */
std::string MacStatsTimeline::
get_graph_tooltip(int mouse_x, int mouse_y) const {
  return PStatTimeline::get_bar_tooltip(pixel_to_row(mouse_y), mouse_x);
}

/**
 * Based on the mouse position within the graph window, look for draggable
 * things the mouse might be hovering over and return the appropriate DragMode
 * enum or DM_none if nothing is indicated.
 */
MacStatsGraph::DragMode MacStatsTimeline::
consider_drag_start(int graph_x, int graph_y) {
  return MacStatsGraph::consider_drag_start(graph_x, graph_y);
}

/**
 *
 */
bool MacStatsTimeline::
handle_key(int graph_x, int graph_y, bool pressed, UniChar c, unsigned short key_code) {
  // Accept WASD based on their position rather than their mapping
  int flag = 0;
  switch (key_code) {
  case 13:
    flag = F_w;
    break;
  case 0:
    flag = F_a;
    break;
  case 1:
    flag = F_s;
    break;
  case 2:
    flag = F_d;
    break;
  }
  if (flag == 0) {
    switch (c) {
    case 0x1c:
    case NSLeftArrowFunctionKey:
      flag = F_left;
      break;
    case 0x1d:
    case NSRightArrowFunctionKey:
      flag = F_right;
      break;
    case 'w':
      flag = F_w;
      break;
    case 'a':
      flag = F_a;
      break;
    case 's':
      flag = F_s;
      break;
    case 'd':
      flag = F_d;
      break;
    }
  }
  if (flag != 0) {
    if (pressed) {
      if (flag & (F_w | F_s)) {
        _zoom_center = pixel_to_timestamp(graph_x);
      }
      if (_keys_held == 0) {
        start_animation();
      }
      _keys_held |= flag;
    }
    else if (_keys_held != 0) {
      _keys_held &= ~flag;
    }
    return true;
  }
  return false;
}

/**
 * Called when the mouse button is depressed within the graph window.
 */
void MacStatsTimeline::
handle_button_press(int graph_x, int graph_y, bool double_click, int button) {
  if (graph_x >= 0 && graph_y >= 0 && graph_x < get_xsize() && graph_y < get_ysize()) {
    if (double_click && button == 0) {
      // Double-clicking on a color bar in the graph will zoom the graph into
      // that collector.
      int row = pixel_to_row(graph_y);
      ColorBar bar;
      if (find_bar(row, graph_x, bar)) {
        double width = bar._end - bar._start;
        zoom_to(width * 1.5, pixel_to_timestamp(graph_x));
        scroll_to(bar._start - width / 4.0);
      } else {
        // Double-clicking the white area zooms out.
        _zoom_speed -= 100.0;
      }
      start_animation();
    }

    if (_potential_drag_mode == DM_none) {
      set_drag_mode(DM_pan);
      _drag_start_x = graph_x;
      _scroll_speed = 0.0;
      _zoom_center = pixel_to_timestamp(graph_x);
      return;
    }
  }

  return MacStatsGraph::handle_button_press(graph_x, graph_y,
                                            double_click, button);
}

/**
 * Called when the mouse button is released within the graph window.
 */
void MacStatsTimeline::
handle_button_release(int graph_x, int graph_y) {
  if (_drag_mode == DM_scale) {
    set_drag_mode(DM_none);
    // ReleaseCapture();
    return handle_motion(graph_x, graph_y);
  }
  else if (_drag_mode == DM_guide_bar) {
    if (graph_x < 0 || graph_x >= get_xsize()) {
      remove_user_guide_bar(_drag_guide_bar);
    } else {
      move_user_guide_bar(_drag_guide_bar, pixel_to_height(graph_x));
    }
    set_drag_mode(DM_none);
    // ReleaseCapture();
    return handle_motion(graph_x, graph_y);
  }

  return MacStatsGraph::handle_button_release(graph_x, graph_y);
}

/**
 * Called when the mouse is moved within the graph window.
 */
void MacStatsTimeline::
handle_motion(int graph_x, int graph_y) {
  if (_drag_mode == DM_none && _potential_drag_mode == DM_none &&
      graph_x >= 0 && graph_y >= 0 && graph_x < get_xsize() && graph_y < get_ysize()) {
    // When the mouse is over a color bar, highlight it.
    int row = pixel_to_row(graph_y);
    std::swap(_highlighted_x, graph_x);
    std::swap(_highlighted_row, row);

    if (row >= 0) {
      PStatTimeline::force_redraw(row, graph_x, graph_x);
    }
    PStatTimeline::force_redraw(_highlighted_row, _highlighted_x, _highlighted_x);

    if ((_keys_held & (F_w | F_s)) != 0) {
      // Update the zoom center if we move the mouse while zooming with the
      // keyboard.
      _zoom_center = pixel_to_timestamp(graph_x);
    }
  }
  else {
    // If the mouse is in some drag mode, stop highlighting.
    if (_highlighted_row != -1) {
      int row = _highlighted_row;
      _highlighted_row = -1;
      PStatTimeline::force_redraw(row, _highlighted_x, _highlighted_x);
    }
  }

  if (_drag_mode == DM_pan) {
    int delta = _drag_start_x - graph_x;
    _drag_start_x = graph_x;
    set_horizontal_scroll(get_horizontal_scroll() + pixel_to_height(delta));
    return;
  }

  return MacStatsGraph::handle_motion(graph_x, graph_y);
}

/**
 * Called when the mouse has left the graph window.
 */
void MacStatsTimeline::
handle_leave() {
  if (_highlighted_row != -1) {
    int row = _highlighted_row;
    _highlighted_row = -1;
    PStatTimeline::force_redraw(row, _highlighted_x, _highlighted_x);
  }
}

/**
 * Called when the mouse is moved within the graph window.
 */
void MacStatsTimeline::
handle_scroll() {
  // Graph view is flipped, side bar isn't, so we need to convert coordinates
  NSPoint point;
  point.x = 0;
  point.y = _graph_view.frame.size.height - (((NSScrollView *)_graph_view_controller.view).documentVisibleRect.size.height + ((NSScrollView *)_graph_view_controller.view).documentVisibleRect.origin.y);
  [_sidebar_scroll_view.contentView scrollToPoint:point];
  [_sidebar_scroll_view reflectScrolledClipView:_sidebar_scroll_view.contentView];
}

/**
 *
 */
void MacStatsTimeline::
handle_wheel(int graph_x, int graph_y, double dx, double dy) {
  if (dx != 0.0) {
    _scroll_speed -= dx;
    start_animation();
  }
}

/**
 *
 */
void MacStatsTimeline::
handle_magnify(int graph_x, int graph_y, double scale) {
  zoom_by(scale * 4.0, pixel_to_timestamp(graph_x));
  start_animation();
}

/**
 * Fills in the graph window.
 */
void MacStatsTimeline::
handle_draw_graph(CGContextRef ctx, NSRect rect) {
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 140000
  for (const GuideBar &bar : _guide_bars) {
    int x = timestamp_to_pixel(bar._height);
    draw_guide_bar(ctx, bar._style, x, rect.origin.y, rect.size.height);
  }
#endif

  MacStatsGraph::handle_draw_graph(ctx, rect);

  NSRect scale_frame = _scale_area.frame;
  NSRect graph_frame = _graph_view.frame;
  if (scale_frame.size.width != graph_frame.size.width) {
    scale_frame.size.width = graph_frame.size.width;
    _scale_area.frame = scale_frame;
  }
}

/**
 * Fills in the graph window overhang, which is the area outside the graph
 * bounds that may become visible momentarily due to scroll elasticity.
 */
void MacStatsTimeline::
handle_draw_graph_overhang(CGContextRef ctx, NSRect rect) {
  CGContextSetFillColorWithColor(ctx, _background_color);
  CGContextFillRect(ctx, rect);

  for (const GuideBar &bar : _guide_bars) {
    int x = timestamp_to_pixel(bar._height);
    draw_guide_bar(ctx, bar._style, x, rect.origin.y, rect.size.height);
  }
}

/**
 * Fills in the scale area.
 */
void MacStatsTimeline::
handle_draw_scale_area(CGContextRef ctx, NSRect rect) {
  MacStatsGraph::handle_draw_scale_area(ctx, rect);

  draw_guide_labels(ctx);

  CGContextSetFillColorWithColor(ctx, [NSColor gridColor].CGColor);

  for (const GuideBar &bar : _guide_bars) {
    int x = timestamp_to_pixel(bar._height);
    x = [_scale_area convertPoint:NSMakePoint(x, 0) fromView:_graph_view].x;
    draw_guide_bar(ctx, bar._style, x, rect.origin.y, rect.size.height);
  }
}

/**
 *
 */
void MacStatsTimeline::
handle_zoom_to() {
  const ColorBar &bar = _popup_bar;
  double width = bar._end - bar._start;
  zoom_to(width * 1.5, (bar._end + bar._start) / 2.0);
  scroll_to(bar._start - width / 4.0);
  start_animation();
}

/**
 *
 */
void MacStatsTimeline::
handle_open_strip_chart() {
  const ColorBar &bar = _popup_bar;
  MacStatsGraph::_monitor->open_strip_chart(bar._thread_index, bar._collector_index, false);
}

/**
 *
 */
void MacStatsTimeline::
handle_open_flame_graph() {
  const ColorBar &bar = _popup_bar;
  MacStatsGraph::_monitor->open_flame_graph(bar._thread_index, bar._collector_index, bar._frame_number);
}

/**
 *
 */
void MacStatsTimeline::
handle_open_piano_roll() {
  const ColorBar &bar = _popup_bar;
  MacStatsGraph::_monitor->open_piano_roll(bar._thread_index);
}

/**
 * Draws a vertical guide bar.  If the row is -1, draws it in all rows.
 */
void MacStatsTimeline::
draw_guide_bar(CGContextRef ctx, GuideBarStyle style, int x, int y, int height) {
  double width = 1.0;
  if (style == GBS_frame) {
    width *= 2;
  }

  CGContextSetFillColorWithColor(ctx, [NSColor gridColor].CGColor);
  CGContextFillRect(ctx, CGRectMake(x - width / 2.0, y, width, height));
}

/**
 * This is called during the servicing of the draw event.
 */
void MacStatsTimeline::
draw_guide_labels(CGContextRef ctx) {
  int num_guide_bars = get_num_guide_bars();
  for (int i = 0; i < num_guide_bars; i++) {
    draw_guide_label(ctx, get_guide_bar(i));
  }
}

/**
 * Draws the text for the indicated guide bar label at the top of the graph.
 */
void MacStatsTimeline::
draw_guide_label(CGContextRef ctx, const PStatGraph::GuideBar &bar) {
  const std::string &label = bar._label;
  if (label.empty()) {
    return;
  }

  NSColor *color;
  if (@available(macOS 11.0, *)) {
    color = [NSColor tertiaryLabelColor];
  } else {
    // Otherwise it's hard to see on the dark titlebars
    color = [NSColor windowFrameTextColor];
  }
  /*switch (bar._style) {
  case GBS_target:
    color = [NSColor colorWithDeviceRed:rgb_light_gray[0] green:rgb_light_gray[1] blue:rgb_light_gray[2] alpha:1.0];
    break;

  case GBS_user:
    color = [NSColor colorWithDeviceRed:rgb_user_guide_bar[0] green:rgb_user_guide_bar[1] blue:rgb_user_guide_bar[2] alpha:1.0];
    break;

  case GBS_normal:
    color = [NSColor colorWithDeviceRed:rgb_light_gray[0] green:rgb_light_gray[1] blue:rgb_light_gray[2] alpha:1.0];
    break;

  case GBS_frame:
    color = [NSColor colorWithDeviceRed:rgb_dark_gray[0] green:rgb_dark_gray[1] blue:rgb_dark_gray[2] alpha:1.0];
    break;
  }*/

  const CFStringRef keys[] = {
    (__bridge CFStringRef)NSForegroundColorAttributeName,
    (__bridge CFStringRef)NSFontAttributeName,
  };
  const void *values[] = {
    color,
    [NSFont systemFontOfSize:0.0],
  };
  CFDictionaryRef attribs = CFDictionaryCreate(kCFAllocatorDefault, (const void **)keys, (const void **)values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

  CFStringRef str = CFStringCreateWithCString(kCFAllocatorDefault, label.c_str(), kCFStringEncodingUTF8);
  CFAttributedStringRef astr = CFAttributedStringCreate(kCFAllocatorDefault, str, attribs);
  CFRelease(attribs);
  CFRelease(str);

  CTLineRef line = CTLineCreateWithAttributedString(astr);
  CFRelease(astr);
  CGRect bounds = CTLineGetImageBounds(line, ctx);
  //int height = bounds.size.height;
  int width = bounds.size.width;

  NSRect graph_bounds = _graph_view.bounds;

  int x = timestamp_to_pixel(bar._height);
  x = [_scale_area convertPoint:NSMakePoint(x, 0) fromView:_graph_view].x;

  if (x + width >= 0 && x + width < get_xsize()) {
    if (x + width < graph_bounds.size.width) {
      CGContextSetTextPosition(ctx, x + 6, 6);
      CTLineDraw(line, ctx);
    }
  }

  CFRelease(line);
}

@implementation MacStatsTimelineViewController

- (void)handleZoomTo:(NSMenuItem *)item {
  ((MacStatsTimeline *)_graph)->handle_zoom_to();
}

- (void)handleOpenStripChart:(NSMenuItem *)item {
  ((MacStatsTimeline *)_graph)->handle_open_strip_chart();
}

- (void)handleOpenFlameGraph:(NSMenuItem *)item {
  ((MacStatsTimeline *)_graph)->handle_open_flame_graph();
}

- (void)handleOpenPianoRoll:(NSMenuItem *)item {
  ((MacStatsTimeline *)_graph)->handle_open_piano_roll();
}

@end

