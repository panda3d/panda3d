/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsFlameGraph.mm
 * @author rdb
 * @date 2023-08-18
 */

#include "macStatsFlameGraph.h"
#include "macStatsMonitor.h"
#include "macStatsGraphView.h"
#include "macStatsScaleArea.h"
#include "pStatCollectorDef.h"
#include "cocoa_compat.h"

@interface MacStatsFlameGraphViewController : MacStatsGraphViewController
@end

static const int default_flame_graph_width = 800;
static const int default_flame_graph_height = 250;

/**
 *
 */
MacStatsFlameGraph::
MacStatsFlameGraph(MacStatsMonitor *monitor, int thread_index,
                   int collector_index, int frame_number) :
  PStatFlameGraph(monitor, thread_index, collector_index, frame_number, 0, 0),
  MacStatsGraph(monitor, [MacStatsFlameGraphViewController alloc])
{
  // Used for popup menus.
  _menu_delegate = [[MacStatsChartMenuDelegate alloc] initWithMonitor:monitor threadIndex:thread_index];

  // Set the initial size of the graph.
  int height = default_flame_graph_height + _window.frame.size.height - _window.contentLayoutRect.size.height;
  _graph_view.frame = NSMakeRect(0, 0, default_flame_graph_width, height);
  _graph_view_controller.view.frame = NSMakeRect(0, 0, default_flame_graph_width, height);

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

  //MacStatsScaleAreaController *scale_area_controller = [[MacStatsScaleAreaController alloc] initWithGraph:this];
  //scale_area_controller.layoutAttribute = NSLayoutAttributeRight;
  //_scale_area = scale_area_controller.view;
  //[_window addTitlebarAccessoryViewController:scale_area_controller];

  _window.contentViewController = _graph_view_controller;

  _graph_view_controller.backToolbarItemVisible = NO;

  // Let's show the units on the guide bar labels.  There's room.
  set_guide_bar_units(get_guide_bar_units() | GBU_show_units);

  if (get_average_mode()) {
    start_animation();
  }

  if (is_title_unknown()) {
    std::string window_title = get_title_text();
    if (!is_title_unknown()) {
      _window.title = [NSString stringWithUTF8String:window_title.c_str()];
    }
  }

  if (@available(macOS 11.0, *)) {
    std::string text = format_number(get_horizontal_scale(), get_guide_bar_units(), get_guide_bar_unit_name());
    [_total_item setTitle:[NSString stringWithUTF8String:text.c_str()]];
  }

  [_window makeKeyAndOrderFront:nil];
}

/**
 *
 */
MacStatsFlameGraph::
~MacStatsFlameGraph() {
  [_menu_delegate release];
}

/**
 * Called whenever a new Collector definition is received from the client.
 */
void MacStatsFlameGraph::
new_collector(int collector_index) {
  MacStatsGraph::new_collector(collector_index);
}

/**
 * Called as each frame's data is made available.  There is no guarantee the
 * frames will arrive in order, or that all of them will arrive at all.  The
 * monitor should be prepared to accept frames received out-of-order or
 * missing.
 */
void MacStatsFlameGraph::
new_data(int thread_index, int frame_number) {
  if (is_title_unknown()) {
    std::string window_title = get_title_text();
    if (!is_title_unknown()) {
      _window.title = [NSString stringWithUTF8String:window_title.c_str()];
    }
  }

  if (!_pause) {
    update();

    if (@available(macOS 11.0, *)) {
      std::string text = format_number(get_horizontal_scale(), get_guide_bar_units(), get_guide_bar_unit_name());
      [_total_item setTitle:[NSString stringWithUTF8String:text.c_str()]];
    }
  }
}

/**
 * Called when it is necessary to redraw the entire graph.
 */
void MacStatsFlameGraph::
force_redraw() {
  if (_ctx) {
    PStatFlameGraph::force_redraw();
  }
}

/**
 * Called when the user has resized the window, forcing a resize of the graph.
 */
void MacStatsFlameGraph::
changed_graph_size(int graph_xsize, int graph_ysize) {
  PStatFlameGraph::changed_size(graph_xsize, graph_ysize);
}

/**
 * Called when the user selects a new time units from the monitor pulldown
 * menu, this should adjust the units for the graph to the indicated mask if
 * it is a time-based graph.
 */
void MacStatsFlameGraph::
set_time_units(int unit_mask) {
  int old_unit_mask = get_guide_bar_units();
  if ((old_unit_mask & (GBU_hz | GBU_ms)) != 0) {
    unit_mask = unit_mask & (GBU_hz | GBU_ms);
    unit_mask |= (old_unit_mask & GBU_show_units);
    set_guide_bar_units(unit_mask);

    if (@available(macOS 11.0, *)) {
      std::string text = format_number(get_horizontal_scale(), get_guide_bar_units(), get_guide_bar_unit_name());
      [_total_item setTitle:[NSString stringWithUTF8String:text.c_str()]];
    }

    //_scale_area.needsDisplay = YES;
  }
}

/**
 * Called when the user single-clicks on a label.
 */
void MacStatsFlameGraph::
on_click_label(int collector_index) {
  int current = get_collector_index();
  if (collector_index != current) {
    if (get_history_depth() == 0) {
      _graph_view_controller.backToolbarItemVisible = YES;
    }
    push_collector_index(collector_index);

    std::string window_title = get_title_text();
    if (!is_title_unknown()) {
      _window.title = [NSString stringWithUTF8String:window_title.c_str()];
    }
  }
}

/**
 * Called when the user hovers the mouse over a label.
 */
void MacStatsFlameGraph::
on_enter_label(int collector_index) {
  if (collector_index != _highlighted_index) {
    _highlighted_index = collector_index;

    if (!get_average_mode()) {
      PStatFlameGraph::force_redraw();
    }
  }
}

/**
 * Called when the user's mouse cursor leaves a label.
 */
void MacStatsFlameGraph::
on_leave_label(int collector_index) {
  if (collector_index == _highlighted_index && collector_index != -1) {
    _highlighted_index = -1;

    if (!get_average_mode()) {
      PStatFlameGraph::force_redraw();
    }
  }
}

/**
 * Called when the mouse right-clicks on a label, and should return the menu
 * that should pop up.
 */
NSMenu *MacStatsFlameGraph::
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
 * Calls update_guide_bars with parameters suitable to this kind of graph.
 */
void MacStatsFlameGraph::
normal_guide_bars() {
  // We want vaguely 100 pixels between guide bars.
  int num_bars = get_xsize() / 100;

  _guide_bars.clear();

  double dist = get_horizontal_scale() / num_bars;

  for (int i = 1; i < num_bars; ++i) {
    _guide_bars.push_back(make_guide_bar(i * dist));
  }

  _guide_bars_changed = true;

  //nassertv_always(_scale_area != nullptr);
  //_scale_area.needsDisplay = YES;
}

/**
 * Erases the chart area.
 */
void MacStatsFlameGraph::
clear_region() {
  if (_ctx) {
    CGContextSetFillColorWithColor(_ctx, _background_color);
    CGContextFillRect(_ctx, CGRectMake(0, 0, get_xsize(), get_ysize()));
  }
}

/**
 * Erases the chart area in preparation for drawing a bunch of bars.
 */
void MacStatsFlameGraph::
begin_draw() {
  if (!_ctx) {
    return;
  }

  clear_region();

  // isFlipped is true in the NSView, so flip the text again
  CGContextSetTextMatrix(_ctx, CGAffineTransformMakeScale(1, -1));

  // Draw in the guide bars.
  int num_guide_bars = get_num_guide_bars();
  for (int i = 0; i < num_guide_bars; i++) {
    const GuideBar &bar = get_guide_bar(i);
    draw_guide_bar(_ctx, bar);
    draw_guide_label(_ctx, bar);
  }
}

/**
 * Should be overridden by the user class.  Should draw a single bar at the
 * indicated location.
 */
void MacStatsFlameGraph::
draw_bar(int depth, int from_x, int to_x, int collector_index, int parent_index) {
  double bottom = get_ysize() - depth * 4.000 * 5;
  double top = bottom - 4.000 * 5;

  top += 1;

  MacStatsMonitor *monitor = MacStatsGraph::_monitor;

  bool is_highlighted = collector_index == _highlighted_index;
  CGContextSetFillColorWithColor(_ctx,
    monitor->get_collector_color(collector_index, is_highlighted));

  if (to_x < from_x + 3) {
    // It's just a tiny sliver.  This is a more reliable way to draw it.
    CGRect rect = CGRectMake(from_x, top, to_x - from_x, bottom - top);
    CGContextFillRect(_ctx, rect);
  }
  else {
    double radius = std::min((double)4.000, (to_x - from_x) / 2.0);
    CGContextBeginPath(_ctx);
    CGContextAddArc(_ctx, to_x - radius, top + radius, radius, -0.5 * M_PI, 0.0, NO);
    CGContextAddArc(_ctx, to_x - radius, bottom - radius, radius, 0.0, 0.5 * M_PI, NO);
    CGContextAddArc(_ctx, from_x + radius, bottom - radius, radius, 0.5 * M_PI, M_PI, NO);
    CGContextAddArc(_ctx, from_x + radius, top + radius, radius, M_PI, 1.5 * M_PI, NO);
    CGContextClosePath(_ctx);
    CGContextFillPath(_ctx);

    if ((to_x - from_x) >= 4.000 * 4) {
      // Only bother drawing the text if we've got some space to draw on.
      int left = std::max(from_x, 0) + 4.000 / 2;
      int right = std::min(to_x, get_xsize()) - 4.000 / 2;

      const PStatClientData *client_data = monitor->get_client_data();
      const PStatCollectorDef &def = client_data->get_collector_def(collector_index);

      const CFStringRef keys[] = {
        (__bridge CFStringRef)NSForegroundColorAttributeName,
        (__bridge CFStringRef)NSFontAttributeName,
      };
      const void *values[] = {
        monitor->get_collector_text_color(collector_index, is_highlighted),
        [NSFont systemFontOfSize:0.0],
      };
      CFDictionaryRef attribs = CFDictionaryCreate(kCFAllocatorDefault, (const void **)keys, (const void **)values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

      CFStringRef str = CFStringCreateWithCString(kCFAllocatorDefault, def._name.c_str(), kCFStringEncodingUTF8);
      CFAttributedStringRef astr = CFAttributedStringCreate(kCFAllocatorDefault, str, attribs);

      CTLineRef line = CTLineCreateWithAttributedString(astr);
      CGRect bounds = CTLineGetImageBounds(line, _ctx);
      CFRelease(astr);
      CFRelease(str);

      if (bounds.size.width < right - left) {
        // We have room for more.  Show the collector's actual parent, if it's
        // different than the block it's shown above.
        if (def._parent_index > 0 && def._parent_index != parent_index) {
          const PStatCollectorDef &parent_def = client_data->get_collector_def(def._parent_index);
          std::string long_name = parent_def._name + ":" + def._name;

          CFStringRef long_str = CFStringCreateWithCString(kCFAllocatorDefault, long_name.c_str(), kCFStringEncodingUTF8);
          CFAttributedStringRef long_astr = CFAttributedStringCreate(kCFAllocatorDefault, long_str, attribs);

          CTLineRef long_line = CTLineCreateWithAttributedString((CFAttributedStringRef)long_astr);
          CGRect long_bounds = CTLineGetImageBounds(long_line, _ctx);

          if (long_bounds.size.width < right - left) {
            CFRelease(line);
            line = long_line;
            bounds = long_bounds;
          } else {
            CFRelease(long_line);
          }
          CFRelease(long_astr);
          CFRelease(long_str);
        }
      }
      else {
        static CFStringRef token_str = CFSTR("\u2026");
        CFAttributedStringRef token_astr = CFAttributedStringCreate(kCFAllocatorDefault, token_str, attribs);
        CTLineRef token_line = CTLineCreateWithAttributedString(token_astr);
        CTLineRef trunc_line = CTLineCreateTruncatedLine(line, right - left, kCTLineTruncationEnd, token_line);
        CFRelease(line);
        CFRelease(token_astr);
        CFRelease(token_line);
        line = trunc_line;
      }

      // Center the text vertically in the bar.
      if (line != nullptr) {
        CGContextSetTextPosition(_ctx, left, top + (bottom - top + bounds.size.height) / 2);
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
void MacStatsFlameGraph::
end_draw() {
  _graph_view.needsDisplay = YES;
}

/**
 * Called at the end of the draw cycle.
 */
void MacStatsFlameGraph::
idle() {
}

/**
 * Overridden by a derived class to implement an animation.  If it returns
 * false, the animation timer is stopped.
 */
bool MacStatsFlameGraph::
animate(double time, double dt) {
  return PStatFlameGraph::animate(time, dt);
}

/**
 * Returns the current window dimensions.
 */
bool MacStatsFlameGraph::
get_window_state(int &x, int &y, int &width, int &height,
                 bool &maximized, bool &minimized) const {
  MacStatsGraph::get_window_state(x, y, width, height, maximized, minimized);
  return true;
}

/**
 * Called to restore the graph window to its previous dimensions.
 */
void MacStatsFlameGraph::
set_window_state(int x, int y, int width, int height,
                 bool maximized, bool minimized) {
  MacStatsGraph::set_window_state(x, y, width, height, maximized, minimized);
}

/**
 * Called when the mouse right-clicks on the graph, and should return the menu
 * that should pop up.
 */
NSMenu *MacStatsFlameGraph::
get_graph_menu(int mouse_x, int mouse_y) const {
  int collector_index = get_bar_collector(pixel_to_depth(mouse_y), mouse_x);
  if (collector_index != -1) {
    return get_label_menu(collector_index);
  }
  return nil;
}

/**
 * Called when the mouse hovers over the graph, and should return the text that
 * should appear on the tooltip.
 */
std::string MacStatsFlameGraph::
get_graph_tooltip(int mouse_x, int mouse_y) const {
  return get_bar_tooltip(pixel_to_depth(mouse_y), mouse_x);
}

/**
 * Based on the mouse position within the window's client area, look for
 * draggable things the mouse might be hovering over and return the
 * apprioprate DragMode enum or DM_none if nothing is indicated.
 */
MacStatsGraph::DragMode MacStatsFlameGraph::
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

  return DM_none;
}

/**
 *
 */
bool MacStatsFlameGraph::
handle_key(int graph_x, int graph_y, bool pressed, UniChar c, unsigned short key_code) {
  bool changed = false;

  if (pressed) {
    switch (c) {
    case NSLeftArrowFunctionKey:
      changed = prev_frame();
      break;

    case NSRightArrowFunctionKey:
      changed = next_frame();
      break;

    case NSHomeFunctionKey:
      changed = first_frame();
      break;

    case NSEndFunctionKey:
      changed = last_frame();
      break;
    }
  }

  if (changed) {
    std::string window_title = get_title_text();
    if (!is_title_unknown()) {
      _window.title = [NSString stringWithUTF8String:window_title.c_str()];
    }
  }

  return changed;
}

/**
 * Called when the mouse button is depressed within the graph window.
 */
void MacStatsFlameGraph::
handle_button_press(int graph_x, int graph_y, bool double_click, int button) {
  if (graph_x >= 0 && graph_y >= 0 && graph_x < get_xsize() && graph_y < get_ysize()) {
    int depth = pixel_to_depth(graph_y);
    int collector_index = get_bar_collector(depth, graph_x);
    if (double_click && button == 0) {
      // Double-clicking on a color bar in the graph will zoom the graph into
      // that collector.
      if (collector_index >= 0) {
        on_click_label(collector_index);
      } else {
        if (get_history_depth() > 0) {
          clear_history();
          _graph_view_controller.backToolbarItemVisible = NO;
        }
        set_collector_index(-1);

        std::string window_title = get_title_text();
        if (!is_title_unknown()) {
          _window.title = [NSString stringWithUTF8String:window_title.c_str()];
        }
      }
      return;
    }
  }

  if (_potential_drag_mode == DM_none) {
    set_drag_mode(DM_scale);
    _drag_scale_start = pixel_to_height(graph_x);
    // SetCapture(_graph_window);
    return;
  }
  else if (_potential_drag_mode == DM_guide_bar && _drag_guide_bar >= 0) {
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
void MacStatsFlameGraph::
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
void MacStatsFlameGraph::
handle_motion(int graph_x, int graph_y) {
  if (_drag_mode == DM_none && _potential_drag_mode == DM_none &&
      graph_x >= 0 && graph_y >= 0 && graph_x < get_xsize() && graph_y < get_ysize()) {
    // When the mouse is over a color bar, highlight it.
    int depth = pixel_to_depth(graph_y);
    int collector_index = get_bar_collector(depth, graph_x);
    on_enter_label(collector_index);
  }
  else {
    // If the mouse is in some drag mode, stop highlighting.
    _label_stack.highlight_label(-1);
    on_leave_label(_highlighted_index);
  }

  if (_drag_mode == DM_new_guide_bar) {
    // We haven't created the new guide bar yet; we won't until the mouse
    // comes within the graph's region.
    if (graph_x >= 0 && graph_x < get_xsize()) {
      set_drag_mode(DM_guide_bar);
      _drag_guide_bar = add_user_guide_bar(pixel_to_height(graph_x));
      return;
    }
  }
  else if (_drag_mode == DM_guide_bar) {
    move_user_guide_bar(_drag_guide_bar, pixel_to_height(graph_x));
    return;
  }

  return MacStatsGraph::handle_motion(graph_x, graph_y);
}

/**
 * Called when the mouse has left the graph window.
 */
void MacStatsFlameGraph::
handle_leave() {
  _label_stack.highlight_label(-1);
  on_leave_label(_highlighted_index);
  return;
}

/**
 *
 */
void MacStatsFlameGraph::
handle_wheel(int graph_x, int graph_y, double dx, double dy) {
  if (dx != 0.0) {
    if ((dx > 0.0) ? prev_frame() : next_frame()) {
      std::string window_title = get_title_text();
      if (!is_title_unknown()) {
        _window.title = [NSString stringWithUTF8String:window_title.c_str()];
      }
    }
  }
}

/**
 * Fills in the graph window.
 */
void MacStatsFlameGraph::
handle_draw_graph(CGContextRef ctx, NSRect rect) {
  MacStatsGraph::handle_draw_graph(ctx, rect);

  CGContextSetTextMatrix(ctx, CGAffineTransformMakeScale(1, -1));

  int num_user_guide_bars = get_num_user_guide_bars();
  for (int i = 0; i < num_user_guide_bars; i++) {
    const GuideBar &bar = get_user_guide_bar(i);
    draw_guide_bar(ctx, bar);
    draw_guide_label(ctx, bar);
  }
}

/**
 * Called when the mouse clicks the back button in the toolbar.
 */
void MacStatsFlameGraph::
handle_back() {
  if (pop_collector_index()) {
    if (get_history_depth() == 0) {
      _graph_view_controller.backToolbarItemVisible = NO;
    }

    std::string window_title = get_title_text();
    if (!is_title_unknown()) {
      _window.title = [NSString stringWithUTF8String:window_title.c_str()];
    }
  }
}

/**
 * Called when the mouse toggles the "Average" checkbox in the toolbar.
 */
void MacStatsFlameGraph::
handle_toggle_average(bool state) {
  set_average_mode(state);
  if (state) {
    start_animation();
  }
}


/**
 * Converts a pixel to a depth index.
 */
int MacStatsFlameGraph::
pixel_to_depth(int y) const {
  return (get_ysize() - 1 - y) / (4.000 * 5);
}

/**
 * Draws the line for the indicated guide bar on the graph.
 */
void MacStatsFlameGraph::
draw_guide_bar(CGContextRef ctx, const PStatGraph::GuideBar &bar) {
  int x = height_to_pixel(bar._height);

  if (x > 0 && x < get_xsize() - 1) {
    // Only draw it if it's not too close to the top.
    CGContextSetStrokeColorWithColor(ctx, [NSColor gridColor].CGColor);
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
    CGContextMoveToPoint(ctx, x, 0);
    CGContextAddLineToPoint(ctx, x, get_ysize());
    CGContextStrokePath(ctx);
  }
}

/**
 * This is called during the servicing of the draw event.
 */
void MacStatsFlameGraph::
draw_guide_labels(CGContextRef ctx) {
  int num_guide_bars = get_num_guide_bars();
  for (int i = 0; i < num_guide_bars; i++) {
    draw_guide_label(ctx, get_guide_bar(i));
  }

  int num_user_guide_bars = get_num_user_guide_bars();
  for (int i = 0; i < num_user_guide_bars; i++) {
    draw_guide_label(ctx, get_user_guide_bar(i));
  }
}

/**
 * Draws the text for the indicated guide bar label at the top of the graph.
 */
void MacStatsFlameGraph::
draw_guide_label(CGContextRef ctx, const PStatGraph::GuideBar &bar) {
  NSColor *color;
  color = [NSColor tertiaryLabelColor];
  /*switch (bar._style) {
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
    int y = bounds.size.height;
    if (@available(macOS 11.0, *)) {
      // Account for underlap of title bar
      y += _window.frame.size.height - _window.contentLayoutRect.size.height;
    }
    CGContextSetTextPosition(ctx, x + 6, y + 6);
    CTLineDraw(line, ctx);
  }

  CFRelease(line);
}

@implementation MacStatsFlameGraphViewController

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar {
  return @[@"back", @"average", @"total"];
}

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar {
  return @[@"average", @"total"];
}

- (NSToolbarItem *) toolbar:(NSToolbar *)toolbar
      itemForItemIdentifier:(NSString *)ident
  willBeInsertedIntoToolbar:(BOOL)flag {

  if (@available(macOS 11.0, *)) {
    if ([ident isEqual:@"average"]) {
      NSButton *button = [NSButton buttonWithTitle:@"Average" target:self action:@selector(handleToggleAverage:)];
      button.image = [NSImage imageWithSystemSymbolName:@"sum" accessibilityDescription:@""];
      button.bezelStyle = NSBezelStyleTexturedRounded;
      button.buttonType = NSButtonTypePushOnPushOff;
      button.bordered = YES;
      button.toolTip = @"Average";
      button.state = NSOffState;
      NSToolbarItem *item = [[[NSToolbarItem alloc] initWithItemIdentifier:ident] autorelease];
      item.label = @"Average";
      item.view = button;
      return item;
    }
    if ([ident isEqual:@"total"]) {
      NSToolbarItem *item = [[[NSToolbarItem alloc] initWithItemIdentifier:ident] autorelease];
      item.label = @"Total";
      item.enabled = NO;
      [item setBordered:YES];
      return item;
    }
  }

  return [super toolbar:toolbar itemForItemIdentifier:ident willBeInsertedIntoToolbar:flag];
}

- (void)handleToggleAverage:(NSButton *)button {
  MacStatsFlameGraph *graph = (MacStatsFlameGraph *)_graph;
  graph->handle_toggle_average(button.state == NSOnState);
}

@end
