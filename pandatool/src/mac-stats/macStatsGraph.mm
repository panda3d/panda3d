/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsGraph.mm
 * @author rdb
 * @date 2023-08-17
 */

#include "macStatsGraph.h"
#include "macStatsGraphView.h"
#include "macStatsMonitor.h"
#include "macStatsLabelStack.h"

const CGFloat MacStatsGraph::rgb_light_gray[4] = {
  0x9a / (CGFloat)0xff, 0x9a / (CGFloat)0xff, 0x9a / (CGFloat)0xff, 1.0,
};
const CGFloat MacStatsGraph::rgb_dark_gray[4] = {
  0xb0 / (CGFloat)0xff, 0xb0 / (CGFloat)0xff, 0xb0 / (CGFloat)0xff, 1.0,
};
const CGFloat MacStatsGraph::rgb_user_guide_bar[4] = {
  0x82 / (CGFloat)0xff, 0x96 / (CGFloat)0xff, 0xff / (CGFloat)0xff, 1.0,
};

static const NSInteger style_mask = NSWindowStyleMaskTitled
                                  | NSWindowStyleMaskClosable
                                  | NSWindowStyleMaskMiniaturizable
                                  | NSWindowStyleMaskResizable;

/**
 *
 */
MacStatsGraph::
MacStatsGraph(MacStatsMonitor *monitor, MacStatsGraphViewController *controller) :
  _monitor(monitor)
{
  _background_color = CGColorCreateGenericRGB(0.0, 0.0, 0.0, 0.0);

  _drag_mode = DM_none;
  _potential_drag_mode = DM_none;
  _drag_scale_start = 0.0f;

  _pause = false;

  NSInteger this_style_mask = style_mask;
  if (@available(macOS 11.0, *)) {
    this_style_mask |= NSWindowStyleMaskFullSizeContentView;
  }

  _window = [NSWindow alloc];
  [_window initWithContentRect:NSMakeRect(100, 500, 500, 150)
                     styleMask:this_style_mask
                       backing:NSBackingStoreBuffered
                         defer:NO];
  _window.releasedWhenClosed = NO;
  _window.titlebarAppearsTransparent = NO;
  _window.excludedFromWindowsMenu = NO;

  _window.contentMinSize = NSMakeSize(68, _window.contentView.frame.size.height - _window.contentLayoutRect.size.height);

  _graph_view_controller = [controller initWithGraph:this];
  _graph_view = ((MacStatsGraphViewController *)_graph_view_controller).graphView;

  // Get notified when the window closes.
  NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
  [center addObserver:_graph_view_controller
             selector:@selector(windowWillClose:)
                 name:NSWindowWillCloseNotification
               object:_window];
}

/**
 *
 */
MacStatsGraph::
~MacStatsGraph() {
  if (_animation_timer != nil) {
    [_animation_timer invalidate];
    [_animation_timer release];
    _animation_timer = nil;
  }

  _monitor = nullptr;
  release_bitmap();

  _label_stack.clear_labels();

  [_graph_view_controller release];
  [_window release];

  CGColorRelease(_background_color);
}

/**
 *
 */
void MacStatsGraph::
close() {
  [_window close];
}

/**
 * Called whenever a new Collector definition is received from the client.
 */
void MacStatsGraph::
new_collector(int new_collector) {
}

/**
 * Called whenever new data arrives.
 */
void MacStatsGraph::
new_data(int thread_index, int frame_number) {
}

/**
 * Called when the user has resized the window, forcing a resize of the graph.
 */
void MacStatsGraph::
changed_graph_size(int graph_xsize, int graph_ysize) {
}

/**
 * Called when the user selects a new time units from the monitor pulldown
 * menu, this should adjust the units for the graph to the indicated mask if
 * it is a time-based graph.
 */
void MacStatsGraph::
set_time_units(int unit_mask) {
}

/**
 * Called when the user selects a new scroll speed from the monitor pulldown
 * menu, this should adjust the speed for the graph to the indicated value.
 */
void MacStatsGraph::
set_scroll_speed(double scroll_speed) {
}

/**
 * Changes the pause flag for the graph.  When this flag is true, the graph
 * does not update in response to new data.
 */
void MacStatsGraph::
set_pause(bool pause) {
  _pause = pause;
}

/**
 * Called when the user guide bars have been changed.
 */
void MacStatsGraph::
user_guide_bars_changed() {
  if (_scale_area != nullptr) {
    _scale_area.needsDisplay = YES;
  }
  _graph_view.needsDisplay = YES;
}

/**
 * Called when the user single-clicks on a label.
 */
void MacStatsGraph::
on_click_label(int collector_index) {
}

/**
 * Called when the user hovers the mouse over a label.
 */
void MacStatsGraph::
on_enter_label(int collector_index) {
  if (collector_index != _highlighted_index) {
    _highlighted_index = collector_index;
    force_redraw();
  }
}

/**
 * Called when the user's mouse cursor leaves a label.
 */
void MacStatsGraph::
on_leave_label(int collector_index) {
  if (collector_index == _highlighted_index && collector_index != -1) {
    _highlighted_index = -1;
    force_redraw();
  }
}

/**
 * Called when the mouse right-clicks on a label, and should return the menu
 * that should pop up.
 */
NSMenu *MacStatsGraph::
get_label_menu(int collector_index) const {
  return nil;
}

/**
 * Called when the mouse hovers over a label, and should return the text that
 * should appear on the tooltip.
 */
std::string MacStatsGraph::
get_label_tooltip(int collector_index) const {
  return std::string();
}

/**
 * Turns on the animation timer, if it hasn't already been turned on.
 */
void MacStatsGraph::
start_animation() {
  if (_animation_timer != nil) {
    return;
  }

  _time = 0.0;
  _animation_timer = [NSTimer scheduledTimerWithTimeInterval:1 / 60.0 target:_graph_view selector:@selector(handleTimer:) userInfo:nil repeats:YES];
  [_animation_timer retain];
}

/**
 * Overridden by a derived class to implement an animation.  If it returns
 * false, the animation timer is stopped.
 */
bool MacStatsGraph::
animate(double time, double dt) {
  return false;
}

/**
 * Returns the current window dimensions.
 */
void MacStatsGraph::
get_window_state(int &x, int &y, int &width, int &height,
                 bool &maximized, bool &minimized) const {

  NSRect screen_frame = _window.screen.visibleFrame;

  NSRect frame = _window.frame;
  x = frame.origin.x - screen_frame.origin.x;
  y = (screen_frame.origin.y + screen_frame.size.height) - (frame.origin.y + frame.size.height);
  width = frame.size.width;
  height = frame.size.height;
  maximized = _window.zoomed;
  minimized = _window.miniaturized;
}

/**
 * Called to restore the graph window to its previous dimensions.
 */
void MacStatsGraph::
set_window_state(int x, int y, int width, int height,
                 bool maximized, bool minimized) {

  NSRect screen_frame = _window.screen.visibleFrame;

  NSRect frame;
  frame.origin.x = screen_frame.origin.x + x;
  frame.origin.y = (screen_frame.origin.y + screen_frame.size.height) - (y + height);
  frame.size.width = width;
  frame.size.height = height;
  [_window setFrame:frame display:NO];

  if (maximized != _window.zoomed) {
    [_window zoom:_window];
  }

  if (minimized != _window.miniaturized) {
    if (minimized) {
      [_window miniaturize:_window];
    } else {
      [_window deminiaturize:_window];
    }
  }
}

/**
 * Called when the given collector has changed colors.
 */
void MacStatsGraph::
reset_collector_color(int collector_index) {
  force_redraw();
  _label_stack.update_label_color(collector_index);
}

/**
 * Called when the mouse right-clicks on the graph, and should return the menu
 * that should pop up.
 */
NSMenu *MacStatsGraph::
get_graph_menu(int mouse_x, int mouse_y) const {
  return nil;
}

/**
 * Called when the mouse hovers over the graph, and should return the text that
 * should appear on the tooltip.
 */
std::string MacStatsGraph::
get_graph_tooltip(int mouse_x, int mouse_y) const {
  return std::string();
}

/**
 * Based on the mouse position within the graph window, look for draggable
 * things the mouse might be hovering over and return the appropriate DragMode
 * enum or DM_none if nothing is indicated.
 */
MacStatsGraph::DragMode MacStatsGraph::
consider_drag_start(int graph_x, int graph_y) {
  return DM_none;
}

/**
 * This should be called whenever the drag mode needs to change state.  It
 * provides hooks for a derived class to do something special.
 */
void MacStatsGraph::
set_drag_mode(MacStatsGraph::DragMode drag_mode) {
  _drag_mode = drag_mode;
}

/**
 *
 */
bool MacStatsGraph::
handle_key(int graph_x, int graph_y, bool pressed, UniChar c, unsigned short key_code) {
  return false;
}

/**
 * Called when the mouse button is depressed within the window, or any nested
 * window.
 */
void MacStatsGraph::
handle_button_press(int graph_x, int graph_y, bool double_click, int button) {
  if (_potential_drag_mode != DM_none && button == 1) {
    set_drag_mode(_potential_drag_mode);
    _drag_start_x = graph_x;
    _drag_start_y = graph_y;
    // SetCapture(_window);
  }
}

/**
 * Called when the mouse button is released within the window, or any nested
 * window.
 */
void MacStatsGraph::
handle_button_release(int graph_x, int graph_y) {
  set_drag_mode(DM_none);
  // ReleaseCapture();

  return handle_motion(graph_x, graph_y);
}

/**
 * Called when the mouse is moved within the window, or any nested window.
 */
void MacStatsGraph::
handle_motion(int graph_x, int graph_y) {
  _potential_drag_mode = consider_drag_start(graph_x, graph_y);
}

/**
 * Called when the mouse has left the graph window.
 */
void MacStatsGraph::
handle_leave() {
}

/**
 * Called when the mouse is moved within the graph window.
 */
void MacStatsGraph::
handle_scroll() {
  force_redraw();
}

/**
 *
 */
void MacStatsGraph::
handle_wheel(int graph_x, int graph_y, double dx, double dy) {
}

/**
 *
 */
void MacStatsGraph::
handle_magnify(int graph_x, int graph_y, double scale) {
}

/**
 *
 */
void MacStatsGraph::
handle_timer() {
  _time += 1.0 / 60.0;

  if (!animate(_time, 1.0 / 60.0)) {
    [_animation_timer invalidate];
    [_animation_timer release];
    _animation_timer = nil;
  }
}

/**
 * Fills in the graph window.
 */
void MacStatsGraph::
handle_draw_graph(CGContextRef ctx, NSRect rect) {
  CGContextSetBlendMode(ctx, kCGBlendModeCopy);
  CGContextSetInterpolationQuality(ctx, kCGInterpolationNone);

  // Quantize this so that changed_graph_size will always call force_redraw()
  NSRect full_rect = _graph_view.bounds;
  full_rect.size.width = (int)full_rect.size.width;
  full_rect.size.height = (int)full_rect.size.height;

  CGSize size = CGContextConvertSizeToDeviceSpace(ctx, full_rect.size);
  int width = abs((int)size.width);
  int height = abs((int)size.height);

  if (_ctx == nullptr || _bitmap_xsize != width || _bitmap_ysize != height) {
    if (_ctx == nullptr && _scale_area != nullptr) {
      _scale_area.needsDisplay = YES;
    }
    setup_bitmap(width, height, _window.backingScaleFactor);

    changed_graph_size(full_rect.size.width, full_rect.size.height);
  }

  CGImageRef image = CGBitmapContextCreateImage(_ctx);
  CGContextDrawImage(ctx, CGRectMake(0, 0, full_rect.size.width, full_rect.size.height), image);
  CGImageRelease(image);
}

/**
 * Fills in the graph window overhang, which is the area outside the graph
 * bounds that may become visible momentarily due to scroll elasticity.
 */
void MacStatsGraph::
handle_draw_graph_overhang(CGContextRef ctx, NSRect rect) {
}

/**
 * Fills in the scale area.
 */
void MacStatsGraph::
handle_draw_scale_area(CGContextRef ctx, NSRect rect) {
}

/**
 * Called when the mouse clicks the back button in the toolbar.
 */
void MacStatsGraph::
handle_back() {
}

/**
 * Sets up a backing-store bitmap of the indicated size.
 */
void MacStatsGraph::
setup_bitmap(int xsize, int ysize, double scale) {
  release_bitmap();

  _bitmap_xsize = xsize;
  _bitmap_ysize = ysize;

  // Ostensibly, a layer context is more efficient, but I tried it and the
  // performance was horrible compared to a bitmap context.
  _ctx = CGBitmapContextCreate(nullptr, xsize, ysize, 8, 0, CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB), kCGImageAlphaPremultipliedLast);
  CGContextSetBlendMode(_ctx, kCGBlendModeCopy);
  CGContextScaleCTM(_ctx, scale, scale);
}

/**
 * Frees the backing-store bitmap created by setup_bitmap().
 */
void MacStatsGraph::
release_bitmap() {
  if (_ctx != nullptr) {
    CGContextRelease(_ctx);
    _ctx = nullptr;
  }
}
