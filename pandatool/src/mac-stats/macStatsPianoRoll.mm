/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsPianoRoll.mm
 * @author rdb
 * @date 2023-08-19
 */

#include "macStatsPianoRoll.h"
#include "macStatsMonitor.h"
#include "macStatsLabelStack.h"
#include "macStatsScaleArea.h"

static const int default_piano_roll_width = 800;
static const int default_piano_roll_height = 400;

static const int minimum_piano_roll_sidebar_width = 68;
static const int default_piano_roll_sidebar_width = 200;

/**
 *
 */
MacStatsPianoRoll::
MacStatsPianoRoll(MacStatsMonitor *monitor, int thread_index) :
  PStatPianoRoll(monitor, thread_index, 0, 0),
  MacStatsGraph(monitor, [MacStatsScrollableGraphViewController alloc])
{
  // Used for popup menus.
  _menu_delegate = [[MacStatsChartMenuDelegate alloc] initWithMonitor:monitor threadIndex:thread_index];

  // Let's show the units on the guide bar labels.  There's room.
  set_guide_bar_units(get_guide_bar_units() | GBU_show_units);

  const PStatClientData *client_data =
    MacStatsGraph::_monitor->get_client_data();
  std::string thread_name = client_data->get_thread_name(_thread_index);
  std::string window_title = thread_name + " thread piano roll";
  _window.title = [NSString stringWithUTF8String:window_title.c_str()];

  if (@available(macOS 11.0, *)) {
    _window.titleVisibility = NSWindowTitleHidden;
  }

  // Set the initial size of the graph.
  _graph_view.frame = NSMakeRect(0, 0, default_piano_roll_width, default_piano_roll_height);
  _graph_view_controller.view.frame = NSMakeRect(0, 0, default_piano_roll_width, default_piano_roll_height);

  // It's put inside a scroll view that tracks the main scroll view.
  NSScrollView *scroll_view = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, default_piano_roll_sidebar_width, 0)];
  scroll_view.documentView = _label_stack.get_view();
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

  svc.splitViewItems[0].minimumThickness = minimum_piano_roll_sidebar_width;
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

  [_label_stack.get_view().widthAnchor constraintEqualToAnchor:scroll_view.widthAnchor].active = YES;
  [_label_stack.get_view().heightAnchor constraintEqualToAnchor:_graph_view.heightAnchor].active = YES;

  idle();

  [_window makeKeyAndOrderFront:nil];
}

/**
 *
 */
MacStatsPianoRoll::
~MacStatsPianoRoll() {
  [_sidebar_scroll_view release];
  [_menu_delegate release];
}

/**
 * Called as each frame's data is made available.  There is no guarantee the
 * frames will arrive in order, or that all of them will arrive at all.  The
 * monitor should be prepared to accept frames received out-of-order or
 * missing.
 */
void MacStatsPianoRoll::
new_data(int thread_index, int frame_number) {
  if (!_pause) {
    update();
  }
}

/**
 * Called when it is necessary to redraw the entire graph.
 */
void MacStatsPianoRoll::
force_redraw() {
  if (_ctx) {
    PStatPianoRoll::force_redraw();
  }
}

/**
 * Called when the user has resized the window, forcing a resize of the graph.
 */
void MacStatsPianoRoll::
changed_graph_size(int graph_xsize, int graph_ysize) {
  PStatPianoRoll::changed_size(graph_xsize, graph_ysize);
}

/**
 * Called when the user selects a new time units from the monitor pulldown
 * menu, this should adjust the units for the graph to the indicated mask if
 * it is a time-based graph.
 */
void MacStatsPianoRoll::
set_time_units(int unit_mask) {
  int old_unit_mask = get_guide_bar_units();
  if ((old_unit_mask & (GBU_hz | GBU_ms)) != 0) {
    unit_mask = unit_mask & (GBU_hz | GBU_ms);
    unit_mask |= (old_unit_mask & GBU_show_units);
    set_guide_bar_units(unit_mask);
  }
}

/**
 * Called when the user single-clicks on a label.
 */
void MacStatsPianoRoll::
on_click_label(int collector_index) {
  if (collector_index >= 0) {
    MacStatsGraph::_monitor->open_strip_chart(_thread_index, collector_index, false);
  }
}

/**
 * Called when the mouse right-clicks on a label, and should return the menu
 * that should pop up.
 */
NSMenu *MacStatsPianoRoll::
get_label_menu(int collector_index) const {
  NSMenu *menu = [[[NSMenu alloc] init] autorelease];

  std::string label = get_label_tooltip(collector_index);
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
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"Open Strip Chart" action:@selector(handleOpenStripChart:) keyEquivalent:@""];
    item.target = _menu_delegate;
    item.tag = collector_index;
    [menu addItem:item];
    [item release];
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"Open Flame Graph" action:@selector(handleOpenFlameGraph:) keyEquivalent:@""];
    item.target = _menu_delegate;
    item.tag = collector_index;
    [menu addItem:item];
    [item release];
  }

  [menu addItem:[NSMenuItem separatorItem]];

  {
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"Change Color\u2026" action:@selector(handleChangeColor:) keyEquivalent:@""];
    item.target = _graph_view_controller;
    item.tag = collector_index;
    [menu addItem:item];
    [item release];
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"Reset Color" action:@selector(handleResetColor:) keyEquivalent:@""];
    item.target = _graph_view_controller;
    item.tag = collector_index;
    [menu addItem:item];
    [item release];
  }

  return menu;
}

/**
 * Called when the mouse hovers over a label, and should return the text that
 * should appear on the tooltip.
 */
std::string MacStatsPianoRoll::
get_label_tooltip(int collector_index) const {
  return PStatPianoRoll::get_label_tooltip(collector_index);
}

/**
 * Changes the amount of time the width of the horizontal axis represents.
 * This may force a redraw.
 */
void MacStatsPianoRoll::
set_horizontal_scale(double time_width) {
  PStatPianoRoll::set_horizontal_scale(time_width);

  _graph_view.needsDisplay = YES;
}

/**
 * Erases the chart area.
 */
void MacStatsPianoRoll::
clear_region() {
  if (_ctx) {
    CGContextSetFillColorWithColor(_ctx, _background_color);
    CGContextFillRect(_ctx, CGRectMake(0, 0, get_xsize(), get_ysize()));
  }
}

/**
 * Erases the chart area in preparation for drawing a bunch of bars.
 */
void MacStatsPianoRoll::
begin_draw() {
  clear_region();

  // Draw in the guide bars.
  CGContextSetStrokeColorWithColor(_ctx, [NSColor gridColor].CGColor);
  int num_guide_bars = get_num_guide_bars();
  for (int i = 0; i < num_guide_bars; ++i) {
    draw_guide_bar(_ctx, get_guide_bar(i), 0, get_ysize());
  }
}

/**
 * Should be overridden by the user class.  This hook will be called before
 * drawing any one row of bars.  These bars correspond to the collector whose
 * index is get_row_collector(row), and in the color get_row_color(row).
 */
void MacStatsPianoRoll::
begin_row(int row) {
  int collector_index = get_label_collector(row);
  bool is_highlighted = collector_index == _highlighted_index;
  CGContextSetFillColorWithColor(_ctx,
    MacStatsGraph::_monitor->get_collector_color(collector_index, is_highlighted));
}

/**
 * Draws a single bar on the chart.
 */
void MacStatsPianoRoll::
draw_bar(int row, int from_x, int to_x) {
  if (row >= 0 && row < _label_stack.get_num_labels()) {
    int y = _label_stack.get_label_y(row, _graph_view);
    int height = _label_stack.get_label_height(row);

    CGContextFillRect(_ctx, CGRectMake(from_x, (y - height + 2), to_x - from_x, (height - 4)));
  }
}

/**
 * Called after all the bars have been drawn, this triggers a refresh event to
 * draw it to the window.
 */
void MacStatsPianoRoll::
end_draw() {
  _graph_view.needsDisplay = YES;

  if (_guide_bars_changed) {
    _scale_area.needsDisplay = YES;
    _guide_bars_changed = false;
  }
}

/**
 * Called at the end of the draw cycle.
 */
void MacStatsPianoRoll::
idle() {
  if (_labels_changed) {
    update_labels();
  }
}

/**
 * Returns the current window dimensions.
 */
bool MacStatsPianoRoll::
get_window_state(int &x, int &y, int &width, int &height,
                 bool &maximized, bool &minimized) const {
  MacStatsGraph::get_window_state(x, y, width, height, maximized, minimized);
  return true;
}

/**
 * Called to restore the graph window to its previous dimensions.
 */
void MacStatsPianoRoll::
set_window_state(int x, int y, int width, int height,
                 bool maximized, bool minimized) {
  MacStatsGraph::set_window_state(x, y, width, height, maximized, minimized);
}

/**
 * Called when the mouse right-clicks on the graph, and should return the menu
 * that should pop up.
 */
NSMenu *MacStatsPianoRoll::
get_graph_menu(int mouse_x, int mouse_y) const {
  int collector_index = get_collector_under_pixel(mouse_x, mouse_y);
  if (collector_index >= 0) {
    return get_label_menu(collector_index);
  }
  return nil;
}

/**
 * Called when the mouse hovers over the graph, and should return the text that
 * should appear on the tooltip.
 */
std::string MacStatsPianoRoll::
get_graph_tooltip(int mouse_x, int mouse_y) const {
  int collector_index = get_collector_under_pixel(mouse_x, mouse_y);
  if (collector_index >= 0) {
    return get_label_tooltip(collector_index);
  }
  return std::string();
}

/**
 * Based on the mouse position within the graph window, look for draggable
 * things the mouse might be hovering over and return the appropriate DragMode
 * enum or DM_none if nothing is indicated.
 */
MacStatsGraph::DragMode MacStatsPianoRoll::
consider_drag_start(int graph_x, int graph_y) {
  if (graph_y >= 0 && graph_y < get_ysize()) {
    if (graph_x >= 0 && graph_x < get_xsize()) {
      // See if the mouse is over a user-defined guide bar.
      int x = graph_x;
      double from_height = pixel_to_height(x - 2);
      double to_height = pixel_to_height(x + 2);
      _drag_guide_bar = find_user_guide_bar(from_height, to_height);
      if (_drag_guide_bar >= 0) {
        return DM_guide_bar;
      }

    } else {
      // The mouse is left or right of the graph; maybe create a new guide
      // bar.
      return DM_new_guide_bar;
    }
  }

  return MacStatsGraph::consider_drag_start(graph_x, graph_y);
}

/**
 * Called when the mouse button is depressed within the graph window.
 */
void MacStatsPianoRoll::
handle_button_press(int graph_x, int graph_y, bool double_click, int button) {
  if (graph_x >= 0 && graph_y >= 0 && graph_x < get_xsize() && graph_y < get_ysize()) {
    if (double_click && button == 1) {
      // Double-clicking on a color bar in the graph is the same as double-
      // clicking on the corresponding label.
      on_click_label(get_collector_under_pixel(graph_x, graph_y));
      return;
    }
  }

  if (_potential_drag_mode == DM_none) {
    set_drag_mode(DM_scale);
    _drag_scale_start = pixel_to_height(graph_x);
    // SetCapture(_graph_window);
    return;

  } else if (_potential_drag_mode == DM_guide_bar && _drag_guide_bar >= 0) {
    set_drag_mode(DM_guide_bar);
    _drag_start_x = graph_x;
    // SetCapture(_graph_window);
    return;
  }

  return MacStatsGraph::handle_button_press(graph_x, graph_y,
                                            double_click, button);
}

/**
 * Called when the mouse button is released within the graph window.
 */
void MacStatsPianoRoll::
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
void MacStatsPianoRoll::
handle_motion(int graph_x, int graph_y) {
  if (_drag_mode == DM_none && _potential_drag_mode == DM_none) {
    // When the mouse is over a color bar, highlight it.
    int collector_index = get_collector_under_pixel(graph_x, graph_y);
    _label_stack.highlight_label(collector_index);
    on_enter_label(collector_index);

    /*
    // Now we want to get a WM_MOUSELEAVE when the mouse leaves the graph
    // window.
    TRACKMOUSEEVENT tme = {
      sizeof(TRACKMOUSEEVENT),
      TME_LEAVE,
      _graph_window,
      0
    };
    TrackMouseEvent(&tme);
    */
  }
  else {
    // If the mouse is in some drag mode, stop highlighting.
    _label_stack.highlight_label(-1);
    on_leave_label(_highlighted_index);
  }

  if (_drag_mode == DM_scale) {
    double ratio = (double)graph_x / (double)get_xsize();
    if (ratio > 0.0f) {
      set_horizontal_scale(_drag_scale_start / ratio);
    }
    return;
  }
  else if (_drag_mode == DM_new_guide_bar) {
    // We haven't created the new guide bar yet; we won't until the mouse
    // comes within the graph's region.
    if (graph_x >= 0 && graph_x < get_xsize()) {
      set_drag_mode(DM_guide_bar);
      _drag_guide_bar = add_user_guide_bar(pixel_to_height(graph_x));
      return;
    }

  } else if (_drag_mode == DM_guide_bar) {
    move_user_guide_bar(_drag_guide_bar, pixel_to_height(graph_x));
    return;
  }

  return MacStatsGraph::handle_motion(graph_x, graph_y);
}

/**
 * Called when the mouse has left the graph window.
 */
void MacStatsPianoRoll::
handle_leave() {
  _label_stack.highlight_label(-1);
  on_leave_label(_highlighted_index);
}

/**
 * Called when the mouse is moved within the graph window.
 */
void MacStatsPianoRoll::
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
void MacStatsPianoRoll::
handle_magnify(int graph_x, int graph_y, double scale) {
  set_horizontal_scale(get_horizontal_scale() * (1.0 - scale));
}

/**
 * Fills in the graph window.
 */
void MacStatsPianoRoll::
handle_draw_graph(CGContextRef ctx, NSRect rect) {
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 140000
  draw_guide_bars(ctx, rect.origin.y, rect.size.height);
#endif

  // Copy the drawn bars into the graph.
  MacStatsGraph::handle_draw_graph(ctx, rect);

  // Draw the scale area.
/*
  CGContextSetRGBStrokeColor(ctx, rgb_dark_gray[0], rgb_dark_gray[1], rgb_dark_gray[2], 1.0);
  CGContextSetStrokeColor(ctx, rgb_dark_gray);
  CGContextBeginPath(ctx);
  CGContextMoveToPoint(ctx, 0, header_height);
  CGContextAddLineToPoint(ctx, get_xsize(), header_height);
  CGContextStrokePath(ctx);*/

  int num_user_guide_bars = get_num_user_guide_bars();
  for (int i = 0; i < num_user_guide_bars; ++i) {
    draw_guide_bar(ctx, get_user_guide_bar(i), rect.origin.y, rect.size.height);
  }

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
void MacStatsPianoRoll::
handle_draw_graph_overhang(CGContextRef ctx, NSRect rect) {
  CGContextSetFillColorWithColor(ctx, _background_color);
  CGContextFillRect(ctx, rect);

  draw_guide_bars(ctx, rect.origin.y, rect.size.height);
}

/**
 * Fills in the scale area.
 */
void MacStatsPianoRoll::
handle_draw_scale_area(CGContextRef ctx, NSRect rect) {
/*
  CGContextSetRGBStrokeColor(ctx, rgb_dark_gray[0], rgb_dark_gray[1], rgb_dark_gray[2], 1.0);
  CGContextSetStrokeColor(ctx, rgb_dark_gray);
  CGContextBeginPath(ctx);
  CGContextMoveToPoint(ctx, 0, header_height);
  CGContextAddLineToPoint(ctx, get_xsize(), header_height);
  CGContextStrokePath(ctx);*/

  draw_guide_bars(ctx, rect.origin.y, rect.size.height);
  draw_guide_labels(ctx);
}

/**
 * Returns the collector index associated with the indicated vertical row, or
 * -1.
 */
int MacStatsPianoRoll::
get_collector_under_pixel(int xpoint, int ypoint) const {
  if (_label_stack.get_num_labels() == 0) {
    return -1;
  }

  // Assume all of the labels are the same height.
  int origin = _label_stack.get_label_y(0, _graph_view);
  int height = _label_stack.get_label_height(0);
  int row = (origin - ypoint) / height;
  if (row >= 0 && row < _label_stack.get_num_labels()) {
    return _label_stack.get_label_collector_index(row);
  } else  {
    return -1;
  }
}

/**
 * Resets the list of labels.
 */
void MacStatsPianoRoll::
update_labels() {
  _label_stack.clear_labels();
  for (int i = 0; i < get_num_labels(); ++i) {
    _label_stack.add_label(MacStatsGraph::_monitor, this,
         _thread_index,
         get_label_collector(i), true);
  }
  _labels_changed = false;
}

/**
 *
 */
void MacStatsPianoRoll::
draw_guide_bars(CGContextRef ctx, int y, int height) {
  CGContextSetStrokeColorWithColor(ctx, [NSColor gridColor].CGColor);

  int num_guide_bars = get_num_guide_bars();
  for (int i = 0; i < num_guide_bars; ++i) {
    draw_guide_bar(ctx, get_guide_bar(i), y, height);
  }

  int num_user_guide_bars = get_num_user_guide_bars();
  for (int i = 0; i < num_user_guide_bars; ++i) {
    draw_guide_bar(ctx, get_user_guide_bar(i), y, height);
  }
}

/**
 * Draws the line for the indicated guide bar on the graph.
 */
void MacStatsPianoRoll::
draw_guide_bar(CGContextRef ctx, const PStatGraph::GuideBar &bar,
               int y, int height) {
  int x = height_to_pixel(bar._height);

  if (x > 0 && x < get_xsize() - 1) {
    // Only draw it if it's not too close to the top.
    /*switch (bar._style) {
    case GBS_target:
      CGContextSetRGBStrokeColor(ctx, rgb_light_gray[0], rgb_light_gray[1], rgb_light_gray[2], 1.0);
      break;

    case GBS_user:
      CGContextSetRGBStrokeColor(ctx, rgb_user_guide_bar[0], rgb_user_guide_bar[1], rgb_user_guide_bar[2], 1.0);
      break;

    default:
      CGContextSetRGBStrokeColor(ctx, rgb_dark_gray[0], rgb_dark_gray[1], rgb_dark_gray[2], 1.0);
      break;
    }*/
    CGContextBeginPath(ctx);
    CGContextMoveToPoint(ctx, x, y);
    CGContextAddLineToPoint(ctx, x, y + height);
    CGContextStrokePath(ctx);
  }
}

/**
 * This is called during the servicing of the draw event.
 */
void MacStatsPianoRoll::
draw_guide_labels(CGContextRef ctx) {
  int i;
  int num_guide_bars = get_num_guide_bars();
  for (i = 0; i < num_guide_bars; ++i) {
    draw_guide_label(ctx, get_guide_bar(i));
  }

  int num_user_guide_bars = get_num_user_guide_bars();
  for (i = 0; i < num_user_guide_bars; ++i) {
    draw_guide_label(ctx, get_user_guide_bar(i));
  }
}

/**
 * Draws the text for the indicated guide bar label at the top of the graph.
 */
void MacStatsPianoRoll::
draw_guide_label(CGContextRef ctx, const PStatGraph::GuideBar &bar) {
  NSColor *color;
  if (@available(macOS 11.0, *)) {
    color = [NSColor tertiaryLabelColor];
  } else {
    // Otherwise it's hard to see on the dark titlebars
    color = [NSColor windowFrameTextColor];
  }
  /*
  switch (bar._style) {
  case GBS_target:
    color = [NSColor colorWithDeviceRed:rgb_light_gray[0] green:rgb_light_gray[1] blue:rgb_light_gray[2] alpha:1.0];
    break;

  case GBS_user:
    color = [NSColor colorWithDeviceRed:rgb_user_guide_bar[0] green:rgb_user_guide_bar[1] blue:rgb_user_guide_bar[2] alpha:1.0];
    break;

  default:
    color = [NSColor colorWithDeviceRed:rgb_dark_gray[0] green:rgb_dark_gray[1] blue:rgb_dark_gray[2] alpha:1.0];
    break;
  }*/

  int x = height_to_pixel(bar._height);
  const std::string &label = bar._label;

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
  int width = bounds.size.width;

  if (bar._style != GBS_user) {
    double from_height = pixel_to_height(x - width);
    double to_height = pixel_to_height(x + width);
    if (find_user_guide_bar(from_height, to_height) >= 0) {
      // Omit the label: there's a user-defined guide bar in the same space.
      CFRelease(line);
      return;
    }
  }

  if (x >= 0 && x < get_xsize()) {
    CGContextSetTextPosition(ctx, x + 6, 6);
    CTLineDraw(line, ctx);
  }

  CFRelease(line);
}
