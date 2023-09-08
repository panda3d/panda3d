/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsStripChart.mm
 * @author rdb
 * @date 2023-08-18
 */

#include "macStatsStripChart.h"
#include "macStatsMonitor.h"
#include "macStatsScaleArea.h"
#include "pStatCollectorDef.h"
#include "cocoa_compat.h"

@interface MacStatsStripChartViewController : MacStatsGraphViewController
@end

static const int default_strip_chart_width = 400;
static const int default_strip_chart_height = 200;

static const int minimum_strip_chart_sidebar_width = 116;
static const int default_strip_chart_sidebar_width = 116;

/**
 *
 */
MacStatsStripChart::
MacStatsStripChart(MacStatsMonitor *monitor, int thread_index,
                   int collector_index, bool show_level) :
  PStatStripChart(monitor, thread_index, collector_index, show_level, 0, 0),
  MacStatsGraph(monitor, [MacStatsStripChartViewController alloc])
{
  // Used for popup menus.
  _menu_delegate = [[MacStatsChartMenuDelegate alloc] initWithMonitor:monitor threadIndex:thread_index];

  // Set the initial size of the graph.
  int height = default_strip_chart_height + _window.frame.size.height - _window.contentLayoutRect.size.height;
  _graph_view.frame = NSMakeRect(0, 0, default_strip_chart_width, height);
  _graph_view_controller.view.frame = NSMakeRect(0, 0, default_strip_chart_width, height);

  // It's put inside a scroll view that tracks the main scroll view.
  NSScrollView *scroll_view = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, default_strip_chart_sidebar_width, 0)];
  scroll_view.documentView = _label_stack.get_view();
  scroll_view.drawsBackground = NO;
  scroll_view.automaticallyAdjustsContentInsets = YES;
  scroll_view.hasHorizontalScroller = NO;
  scroll_view.hasVerticalScroller = NO;

  NSViewController *sidebar_controller = [[NSViewController alloc] init];
  sidebar_controller.view = scroll_view;

  // Add a view to the right of the graph, to display all of the scale units.
  // Calculate how wide it should be to display a typical label.
  CGFloat width = [NSTextField labelWithString:@"999 ms"].frame.size.width + 8;
  _scale_area = [[MacStatsScaleArea alloc] initWithGraph:this frame:NSMakeRect(0, 0, width, 0)];
  _scale_area.autoresizingMask = NSViewHeightSizable;

  NSViewController *scale_area_controller = [[NSViewController alloc] init];
  scale_area_controller.view = _scale_area;

  NSSplitViewController *svc = [[NSSplitViewController alloc] init];
  [svc addSplitViewItem:[NSSplitViewItem sidebarWithViewController:sidebar_controller]];
  [svc addSplitViewItem:[NSSplitViewItem splitViewItemWithViewController:_graph_view_controller]];
  [svc addSplitViewItem:[NSSplitViewItem splitViewItemWithViewController:scale_area_controller]];

  svc.splitViewItems[0].minimumThickness = minimum_strip_chart_sidebar_width;
  svc.splitViewItems[2].minimumThickness = width;
  svc.splitViewItems[2].maximumThickness = width;

  NSSplitView *split_view = svc.splitView;
  split_view.vertical = YES;
  split_view.dividerStyle = NSSplitViewDividerStyleThin;
  split_view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
  _split_view = split_view;

  // When sidebar collapses, show a sidebar icon in the menu bar
  NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
  [center addObserver:_graph_view_controller
             selector:@selector(handleSplitViewResize:)
                 name:NSSplitViewDidResizeSubviewsNotification
               object:split_view];

  _window.contentViewController = svc;

  _graph_view_controller.backToolbarItemVisible = NO;

  [svc release];
  [sidebar_controller release];

  _total_item = nil;
  if (@available(macOS 11.0, *)) {
    NSToolbar *toolbar = [[NSToolbar alloc] initWithIdentifier:@""];
    toolbar.delegate = _graph_view_controller;
    toolbar.displayMode = NSToolbarDisplayModeIconOnly;
    _window.toolbar = toolbar;
    [_window setToolbarStyle:NSWindowToolbarStyleUnifiedCompact];

    for (NSToolbarItem *item in toolbar.items) {
      if ([item.itemIdentifier isEqual:@"total"]) {
        _total_item = item;
        break;
      }
    }
    [toolbar release];
  }

  /*{
    _smooth_checkbox = [NSButton checkboxWithTitle:@"Smooth"
                                            target:_graph_view_controller
                                            action:@selector(handleToggleAverageMode:)];

    NSStackView *stack_view = [NSStackView stackViewWithViews:@[_smooth_checkbox]];
    stack_view.translatesAutoresizingMaskIntoConstraints = NO;

    NSTitlebarAccessoryViewController *accessory_controller = [[NSTitlebarAccessoryViewController alloc] init];
    //accessory_controller.layoutAttribute = NSLayoutAttributeLeft;
    [accessory_controller.view addSubview:stack_view];
    //accessory_controller.automaticallyAdjustsSize = NO;
    [_window addTitlebarAccessoryViewController:accessory_controller];
    [accessory_controller release];

    [stack_view.leftAnchor constraintEqualToAnchor:_graph_view.leftAnchor constant:8].active = YES;
    [stack_view.bottomAnchor constraintEqualToAnchor:((NSLayoutGuide *)_window.contentLayoutGuide).topAnchor constant:-8].active = YES;
    //[stack_view.topAnchor constraintEqualToAnchor:svc.view.topAnchor constant:-8].active = YES;
  }*/
  /*{
    _total_label = [NSTextField labelWithString:@""];

    NSStackView *stack_view = [NSStackView stackViewWithViews:@[_total_label]];
    stack_view.translatesAutoresizingMaskIntoConstraints = NO;

    NSTitlebarAccessoryViewController *accessory_controller = [[NSTitlebarAccessoryViewController alloc] init];
    accessory_controller.layoutAttribute = NSLayoutAttributeRight;
    [accessory_controller.view addSubview:stack_view];
    //accessory_controller.automaticallyAdjustsSize = NO;
    [_window addTitlebarAccessoryViewController:accessory_controller];
    [accessory_controller release];

    [stack_view.rightAnchor constraintEqualToAnchor:_graph_view.rightAnchor].active = YES;
    [stack_view.bottomAnchor constraintEqualToAnchor:((NSLayoutGuide *)_window.contentLayoutGuide).topAnchor].active = YES;
    [stack_view.topAnchor constraintEqualToAnchor:svc.view.topAnchor].active = YES;
  }*/

  if (show_level) {
    // If it's a level-type graph, show the appropriate units.
    if (_unit_name.empty()) {
      set_guide_bar_units(GBU_named);
    } else {
      set_guide_bar_units(GBU_named | GBU_show_units);
    }
  } else {
    // If it's a time-type graph, show the ms / Hz units.
    set_guide_bar_units(get_guide_bar_units() | GBU_show_units);
  }

  [_label_stack.get_view().widthAnchor constraintEqualToAnchor:scroll_view.widthAnchor].active = YES;

  // Update window title and total label.
  new_data(0, 0);

  [_window makeKeyAndOrderFront:nil];
}

/**
 *
 */
MacStatsStripChart::
~MacStatsStripChart() {
  [_menu_delegate release];
}

/**
 * Called whenever a new Collector definition is received from the client.
 */
void MacStatsStripChart::
new_collector(int collector_index) {
  MacStatsGraph::new_collector(collector_index);
}

/**
 * Called as each frame's data is made available.  There is no guarantee the
 * frames will arrive in order, or that all of them will arrive at all.  The
 * monitor should be prepared to accept frames received out-of-order or
 * missing.
 */
void MacStatsStripChart::
new_data(int thread_index, int frame_number) {
  if (is_title_unknown()) {
    std::string window_title = get_title_text();
    if (!is_title_unknown()) {
      _window.title = [NSString stringWithUTF8String:window_title.c_str()];
    }
  }

  if (!_pause) {
    update();

    if (@available(macOS 10.15, *)) {
      std::string text = get_total_text();
      [_total_item setTitle:[NSString stringWithUTF8String:text.c_str()]];
    }
  }
}

/**
 * Called when it is necessary to redraw the entire graph.
 */
void MacStatsStripChart::
force_redraw() {
  if (_ctx) {
    PStatStripChart::force_redraw();
  }

  _scale_area.needsDisplay = YES;
}

/**
 * Called when the user has resized the window, forcing a resize of the graph.
 */
void MacStatsStripChart::
changed_graph_size(int graph_xsize, int graph_ysize) {
  PStatStripChart::changed_size(graph_xsize, graph_ysize);
}

/**
 * Called when the user selects a new time units from the monitor pulldown
 * menu, this should adjust the units for the graph to the indicated mask if
 * it is a time-based graph.
 */
void MacStatsStripChart::
set_time_units(int unit_mask) {
  int old_unit_mask = get_guide_bar_units();
  if ((old_unit_mask & (GBU_hz | GBU_ms)) != 0) {
    unit_mask = unit_mask & (GBU_hz | GBU_ms);
    unit_mask |= (old_unit_mask & GBU_show_units);
    set_guide_bar_units(unit_mask);

    if (@available(macOS 10.15, *)) {
      std::string text = get_total_text();
      [_total_item setTitle:[NSString stringWithUTF8String:text.c_str()]];
    }

    _scale_area.needsDisplay = YES;
  }
}

/**
 * Called when the user selects a new scroll speed from the monitor pulldown
 * menu, this should adjust the speed for the graph to the indicated value.
 */
void MacStatsStripChart::
set_scroll_speed(double scroll_speed) {
  // The speed factor indicates chart widths per minute.
  if (scroll_speed != 0.0f) {
    set_horizontal_scale(60.0f / scroll_speed);
  }
}

/**
 * Called when the user single-clicks on a label.
 */
void MacStatsStripChart::
on_click_label(int collector_index) {
  if (collector_index < 0) {
    // Clicking on whitespace in the graph is the same as clicking on the top
    // label.
    collector_index = get_collector_index();
  }

  if (collector_index == get_collector_index()) {
    // Clicking on the top label means to go up to the parent level.
    if (collector_index != 0) {
      const PStatClientData *client_data =
        MacStatsGraph::_monitor->get_client_data();
      if (client_data->has_collector(collector_index)) {
        const PStatCollectorDef &def =
          client_data->get_collector_def(collector_index);
        if (def._parent_index == 0 && get_view().get_show_level()) {
          // Unless the parent is "Frame", and we're not a time collector.
        }
        else if (def._parent_index != get_collector_index()) {
          // If we were previously at the parent, pop it from the stack.
          if (!_back_stack.empty() && _back_stack.back() == def._parent_index) {
            _back_stack.pop_back();
            if (_back_stack.empty()) {
              _graph_view_controller.backToolbarItemVisible = NO;
            }
          } else {
            if (_back_stack.empty()) {
              _graph_view_controller.backToolbarItemVisible = YES;
            }
            _back_stack.push_back(get_collector_index());
          }

          set_collector_index(def._parent_index);
        }
      }
    }
  }
  else {
    if (_back_stack.empty()) {
      _graph_view_controller.backToolbarItemVisible = YES;
    }
    _back_stack.push_back(get_collector_index());

    // Clicking on any other label means to focus on that.
    set_collector_index(collector_index);
  }

  // Update window title and total label.
  new_data(0, 0);

  _scale_area.needsDisplay = YES;
}

/**
 * Called when the mouse right-clicks on a label, and should return the menu
 * that should pop up.
 */
NSMenu *MacStatsStripChart::
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
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"Set as Focus" action:@selector(handleSetAsFocus:) keyEquivalent:@""];
    item.target = _graph_view_controller;
    item.tag = collector_index;
    item.enabled = (collector_index != 0 || get_collector_index() != 0);
    [menu addItem:item];
    [item release];
  }

  {
    SEL action = get_view().get_show_level() ? @selector(handleOpenStripChartLevel:) : @selector(handleOpenStripChart:);
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"Open Strip Chart" action:action keyEquivalent:@""];
    item.target = _menu_delegate;
    item.tag = collector_index;
    [menu addItem:item];
    [item release];
  }

  if (!get_view().get_show_level()) {
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
std::string MacStatsStripChart::
get_label_tooltip(int collector_index) const {
  return PStatStripChart::get_label_tooltip(collector_index);
}

/**
 * Changes the value the height of the vertical axis represents.  This may
 * force a redraw.
 */
void MacStatsStripChart::
set_vertical_scale(double value_height) {
  PStatStripChart::set_vertical_scale(value_height);

  _graph_view.needsDisplay = YES;
  _scale_area.needsDisplay = YES;
}

/**
 * Sets the vertical scale to make all the data visible.
 */
void MacStatsStripChart::
set_auto_vertical_scale() {
  PStatStripChart::set_auto_vertical_scale();
  set_vertical_scale(get_vertical_scale() * 1.5);

  _graph_view.needsDisplay = YES;
  _scale_area.needsDisplay = YES;
}

/**
 * Resets the list of labels.
 */
void MacStatsStripChart::
update_labels() {
  PStatStripChart::update_labels();

  _label_stack.clear_labels();
  for (int i = 0; i < get_num_labels(); i++) {
    _label_stack.add_label(MacStatsGraph::_monitor, this, _thread_index,
                           get_label_collector(i), false);
  }
  _labels_changed = false;
}

/**
 * Erases the chart area.
 */
void MacStatsStripChart::
clear_region() {
  if (_ctx) {
    //CGContextSetFillColorWithColor(_ctx, _background_color);
    //CGContextFillRect(_ctx, CGRectMake(0, 0, get_xsize(), get_ysize()));
  }
}

/**
 * Draws a single vertical slice of the strip chart, at the given pixel
 * position, and corresponding to the indicated level data.
 */
void MacStatsStripChart::
draw_slice(int x, int w, const PStatStripChart::FrameData &fdata) {
  if (!_ctx) {
    return;
  }

  // Start by clearing the band first.
  CGContextSetFillColorWithColor(_ctx, _background_color);
  CGContextFillRect(_ctx, CGRectMake(x, 0, w, get_ysize()));

  double overall_time = 0.0;
  int y = get_ysize();

  FrameData::const_iterator fi;
  for (fi = fdata.begin(); fi != fdata.end(); ++fi) {
    const ColorData &cd = (*fi);
    overall_time += cd._net_value;

    bool is_highlighted = cd._collector_index == _highlighted_index;
    CGContextSetFillColorWithColor(_ctx,
      MacStatsGraph::_monitor->get_collector_color(cd._collector_index, is_highlighted));

    if (overall_time > get_vertical_scale()) {
      // Off the top.  Go ahead and clamp it by hand, in case it's so far off
      // the top we'd overflow the 16-bit pixel value.
      CGContextFillRect(_ctx, CGRectMake(x, 0, w, y));
      // And we can consider ourselves done now.
      return;
    }

    int top_y = height_to_pixel(overall_time);
    CGContextFillRect(_ctx, CGRectMake(x, top_y, w, y - top_y));
    y = top_y;
  }
}

/**
 * Draws a single vertical slice of background color.
 */
void MacStatsStripChart::
draw_empty(int x, int w) {
  if (!_ctx) {
    return;
  }

  CGContextSetFillColorWithColor(_ctx, _background_color);
  CGContextFillRect(_ctx, CGRectMake(x, 0, w, get_ysize()));
}

/**
 * Draws a single vertical slice of foreground color.
 */
void MacStatsStripChart::
draw_cursor(int x) {
  if (!_ctx) {
    return;
  }

  CGContextBeginPath(_ctx);
  CGContextMoveToPoint(_ctx, x, 0);
  CGContextAddLineToPoint(_ctx, x, get_ysize());
  CGContextStrokePath(_ctx);
}

/**
 * Should be overridden by the user class.  This hook will be called after
 * drawing a series of color bars in the strip chart; it gives the pixel range
 * that was just redrawn.
 */
void MacStatsStripChart::
end_draw(int from_x, int to_x) {
  _graph_view.needsDisplay = YES;
}

/**
 * Returns the current window dimensions.
 */
bool MacStatsStripChart::
get_window_state(int &x, int &y, int &width, int &height,
                 bool &maximized, bool &minimized) const {
  MacStatsGraph::get_window_state(x, y, width, height, maximized, minimized);
  return true;
}

/**
 * Called to restore the graph window to its previous dimensions.
 */
void MacStatsStripChart::
set_window_state(int x, int y, int width, int height,
                 bool maximized, bool minimized) {
  MacStatsGraph::set_window_state(x, y, width, height, maximized, minimized);
}

/**
 * Called when the mouse right-clicks on the graph, and should return the menu
 * that should pop up.
 */
NSMenu *MacStatsStripChart::
get_graph_menu(int mouse_x, int mouse_y) const {
  NSMenu *menu = nullptr;
  if (_highlighted_index != -1) {
    menu = get_label_menu(_highlighted_index);
  }
/*
  if (menu != nullptr) {
    [menu addItem:[NSMenuItem separatorItem]];
  } else {
    menu = [[[NSMenu alloc] init] autorelease];
  }

  NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"Smooth" action:@selector(handleToggleStripChartAverage:) keyEquivalent:@""];
  item.target = _graph_view_controller;
  item.state = get_average_mode() ? NSOnState : NSOffState;
  [menu addItem:item];
  [item release];*/
  return menu;
}

/**
 * Called when the mouse hovers over the graph, and should return the text that
 * should appear on the tooltip.
 */
std::string MacStatsStripChart::
get_graph_tooltip(int mouse_x, int mouse_y) const {
  if (_highlighted_index != -1) {
    return get_label_tooltip(_highlighted_index);
  }
  return std::string();
}

/**
 * Based on the mouse position within the graph window, look for draggable
 * things the mouse might be hovering over and return the appropriate DragMode
 * enum or DM_none if nothing is indicated.
 */
MacStatsGraph::DragMode MacStatsStripChart::
consider_drag_start(int graph_x, int graph_y) {
  // See if the mouse is over a user-defined guide bar.
  int y = graph_y;
  double from_height = pixel_to_height(y + 2);
  double to_height = pixel_to_height(y - 2);
  _drag_guide_bar = find_user_guide_bar(from_height, to_height);
  if (_drag_guide_bar >= 0) {
    return DM_guide_bar;
  }

  return MacStatsGraph::consider_drag_start(graph_x, graph_y);
}

/**
 * This should be called whenever the drag mode needs to change state.  It
 * provides hooks for a derived class to do something special.
 */
void MacStatsStripChart::
set_drag_mode(MacStatsGraph::DragMode drag_mode) {
  MacStatsGraph::set_drag_mode(drag_mode);
}

/**
 * Called when the mouse button is depressed within the graph window.
 */
void MacStatsStripChart::
handle_button_press(int graph_x, int graph_y, bool double_click, int button) {
  if (graph_x >= 0 && graph_y >= 0 && graph_x < get_xsize() && graph_y < get_ysize()) {
    if (double_click && button == 0) {
      // Double-clicking on a color bar in the graph is the same as double-
      // clicking on the corresponding label.
      on_click_label(get_collector_under_pixel(graph_x, graph_y));
      return;
    }

    if (_potential_drag_mode == DM_none) {
      set_drag_mode(DM_scale);
      _drag_scale_start = pixel_to_height(graph_y);
      // SetCapture(_graph_window);
      return;
    }
  }

  if (_potential_drag_mode == DM_guide_bar && _drag_guide_bar >= 0) {
    set_drag_mode(DM_guide_bar);
    _drag_start_y = graph_y;
    // SetCapture(_graph_window);
    return;
  }

  return MacStatsGraph::handle_button_press(graph_x, graph_y,
                                            double_click, button);
}

/**
 * Called when the mouse button is released within the graph window.
 */
void MacStatsStripChart::
handle_button_release(int graph_x, int graph_y) {
  if (_drag_mode == DM_scale) {
    set_drag_mode(DM_none);
    // ReleaseCapture();
    return handle_motion(graph_x, graph_y);
  }
  else if (_drag_mode == DM_guide_bar) {
    if (graph_y < 0 || graph_y >= get_ysize()) {
      remove_user_guide_bar(_drag_guide_bar);
    } else {
      move_user_guide_bar(_drag_guide_bar, pixel_to_height(graph_y));
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
void MacStatsStripChart::
handle_motion(int graph_x, int graph_y) {
  if (_drag_mode == DM_none && _potential_drag_mode == DM_none &&
      graph_x >= 0 && graph_y >= 0 && graph_x < get_xsize() && graph_y < get_ysize()) {
    // When the mouse is over a color bar, highlight it.
    int collector_index = get_collector_under_pixel(graph_x, graph_y);
    _label_stack.highlight_label(collector_index);
    on_enter_label(collector_index);
  }
  else {
    // If the mouse is in some drag mode, stop highlighting.
    _label_stack.highlight_label(-1);
    on_leave_label(_highlighted_index);
  }

  if (_drag_mode == DM_scale) {
    double ratio = 1.0 - ((double)graph_y / (double)get_ysize());
    if (ratio > 0.0) {
      double new_scale = _drag_scale_start / ratio;
      if (!IS_NEARLY_EQUAL(get_vertical_scale(), new_scale)) {
        // Disable smoothing while we do this expensive operation.
        set_vertical_scale(_drag_scale_start / ratio);
      }
    }
    return;
  }
  else if (_drag_mode == DM_new_guide_bar) {
    // We haven't created the new guide bar yet; we won't until the mouse
    // comes within the graph's region.
    if (graph_y >= 0 && graph_y < get_ysize()) {
      set_drag_mode(DM_guide_bar);
      _drag_guide_bar = add_user_guide_bar(pixel_to_height(graph_y));
      return;
    }
  }
  else if (_drag_mode == DM_guide_bar) {
    move_user_guide_bar(_drag_guide_bar, pixel_to_height(graph_y));
    return;
  }

  MacStatsGraph::handle_motion(graph_x, graph_y);
}

/**
 * Called when the mouse has left the graph window.
 */
void MacStatsStripChart::
handle_leave() {
  _label_stack.highlight_label(-1);
  on_leave_label(_highlighted_index);
  return;
}

/**
 *
 */
void MacStatsStripChart::
handle_magnify(int graph_x, int graph_y, double scale) {
  set_vertical_scale(get_vertical_scale() * (1.0 - scale));
}

/**
 * Fills in the graph window.
 */
void MacStatsStripChart::
handle_draw_graph(CGContextRef ctx, NSRect rect) {
  MacStatsGraph::handle_draw_graph(ctx, rect);

  int width = get_xsize();

  // Draw in the guide bars.
  int num_guide_bars = get_num_guide_bars();
  for (int i = 0; i < num_guide_bars; i++) {
    draw_guide_bar(ctx, 0, width, get_guide_bar(i));
  }

  int num_user_guide_bars = get_num_user_guide_bars();
  for (int i = 0; i < num_user_guide_bars; i++) {
    draw_guide_bar(ctx, 0, width, get_user_guide_bar(i));
  }
}

/**
 * Fills in the scale area.
 */
void MacStatsStripChart::
handle_draw_scale_area(CGContextRef ctx, NSRect rect) {
  MacStatsGraph::handle_draw_scale_area(ctx, rect);
  draw_guide_labels(ctx);
}

/**
 * Called when the mouse clicks the back button in the toolbar.
 */
void MacStatsStripChart::
handle_back() {
  if (!_back_stack.empty()) {
    int collector_index = _back_stack.back();
    _back_stack.pop_back();
    set_collector_index(collector_index);

    // Update window title and total label.
    new_data(0, 0);
    _scale_area.needsDisplay = YES;
  }

  if (_back_stack.empty()) {
    _graph_view_controller.backToolbarItemVisible = NO;
  }
}

/**
 * Draws the line for the indicated guide bar on the graph.
 */
void MacStatsStripChart::
draw_guide_bar(CGContextRef ctx, int from_x, int to_x,
               const PStatGraph::GuideBar &bar) {
  int y = height_to_pixel(bar._height);

  if (y > 1) {
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
    CGContextSetStrokeColorWithColor(ctx, [NSColor gridColor].CGColor);
    CGContextBeginPath(ctx);
    CGContextMoveToPoint(ctx, from_x, y);
    CGContextAddLineToPoint(ctx, to_x, y);
    CGContextStrokePath(ctx);
  }
}

/**
 * This is called during the servicing of the draw event.
 */
void MacStatsStripChart::
draw_guide_labels(CGContextRef ctx) {
  // Draw in the labels for the guide bars.
  int last_y = 0;

  int i;
  int num_guide_bars = get_num_guide_bars();
  for (i = 0; i < num_guide_bars; i++) {
    last_y = draw_guide_label(ctx, get_guide_bar(i), last_y);
  }

  GuideBar top_value = make_guide_bar(get_vertical_scale());
  draw_guide_label(ctx, top_value, last_y);

  last_y = 0;
  int num_user_guide_bars = get_num_user_guide_bars();
  for (i = 0; i < num_user_guide_bars; i++) {
    last_y = draw_guide_label(ctx, get_user_guide_bar(i), last_y);
  }
}

/**
 * Draws the text for the indicated guide bar label to the right of the graph,
 * unless it would overlap with the indicated last label, whose top pixel
 * value is given.  Returns the top pixel value of the new label.
 */
int MacStatsStripChart::
draw_guide_label(CGContextRef ctx, const PStatGraph::GuideBar &bar, int last_y) {
  NSColor *color;
  switch (bar._style) {
  case GBS_target:
    color = [NSColor secondaryLabelColor];
    break;

  case GBS_user:
    color = [NSColor tertiaryLabelColor];
    break;

  default:
    color = [NSColor labelColor];
    break;
  }

  int y = height_to_pixel(bar._height);
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
  int height = bounds.size.height;

  if (bar._style != GBS_user) {
    double from_height = pixel_to_height(y + height);
    double to_height = pixel_to_height(y - height);
    if (find_user_guide_bar(from_height, to_height) >= 0) {
      // Omit the label: there's a user-defined guide bar in the same space.
      CFRelease(line);
      return last_y;
    }
  }

  if (y > height && y < get_ysize() - height) {
    // Now convert our y to a coordinate within our drawing area.

    int this_y = y - height / 2;
    if (last_y < this_y || last_y > this_y + height) {
      CGContextSetTextPosition(ctx, 4, get_ysize() - this_y - height);
      CTLineDraw(line, ctx);

      last_y = this_y;
    }
  }

  CFRelease(line);
  return last_y;
}

@implementation MacStatsStripChartViewController

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar {
  if (@available(macOS 10.15, *)) {
    return @[@"smooth", @"sep", @"total"];
  } else {
    return @[@"smooth", @"sep"];
  }
}

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar {
  if (@available(macOS 10.15, *)) {
    return @[@"smooth", @"sep", @"total"];
  } else {
    return @[@"smooth", @"sep"];
  }
}

- (NSToolbarItem *) toolbar:(NSToolbar *)toolbar
      itemForItemIdentifier:(NSString *)ident
  willBeInsertedIntoToolbar:(BOOL)flag {

  if (@available(macOS 11.0, *)) {
    if ([ident isEqual:@"smooth"]) {
      NSButton *button = [NSButton buttonWithTitle:@"Smooth" target:self action:@selector(handleToggleSmooth:)];
      button.image = [NSImage imageWithSystemSymbolName:@"alternatingcurrent" accessibilityDescription:@""];
      button.bezelStyle = NSBezelStyleTexturedRounded;
      button.buttonType = NSButtonTypePushOnPushOff;
      button.bordered = YES;
      button.toolTip = @"Smooth";
      button.state = NSOffState;
      NSToolbarItem *item = [[[NSToolbarItem alloc] initWithItemIdentifier:ident] autorelease];
      item.label = @"Smooth";
      item.view = button;
      return item;
    }
    if ([ident isEqual:@"total"]) {
      NSToolbarItem *item = [[[NSToolbarItem alloc] initWithItemIdentifier:ident] autorelease];
      item.label = @"Total";
      item.action = @selector(handleClickTotal:);
      item.target = self;
      [item setBordered:YES];
      return item;
    }
  }

  return [super toolbar:toolbar itemForItemIdentifier:ident willBeInsertedIntoToolbar:flag];
}

- (void)handleToggleSmooth:(NSMenuItem *)item {
  MacStatsStripChart *graph = (MacStatsStripChart *)_graph;
  graph->set_average_mode(item.state == NSOnState);
}

- (void)handleClickTotal:(NSButton *)button {
  MacStatsStripChart *graph = (MacStatsStripChart *)_graph;
  graph->set_auto_vertical_scale();
}

@end
