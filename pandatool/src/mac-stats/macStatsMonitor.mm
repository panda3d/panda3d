/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsMonitor.mm
 * @author rdb
 * @date 2023-08-17
 */

#include "macStatsMonitor.h"
#include "macStats.h"
#include "macStatsServer.h"
#include "macStatsStripChart.h"
#include "macStatsChartMenu.h"
#include "macStatsPianoRoll.h"
#include "macStatsFlameGraph.h"
#include "macStatsTimeline.h"
#include "pStatGraph.h"
#include "pStatCollectorDef.h"

#include "convert_srgb.h"

/**
 *
 */
MacStatsMonitor::
MacStatsMonitor(MacStatsServer *server) : PStatMonitor(server) {
  _main_menu = NSApp.mainMenu;

  // These will be filled in later when the menu is created.
  _scroll_speed = 0.0;
  _pause = false;
  _next_chart_index = 2;

  setup_speed_menu();

  if ([[NSUserDefaults standardUserDefaults] boolForKey:@"ShowStatusItem"]) {
    set_show_status_item(true);
  }
}

/**
 *
 */
MacStatsMonitor::
~MacStatsMonitor() {
  close();
}

/**
 * Closes all the graphs associated with this monitor.
 */
void MacStatsMonitor::
close() {
  PStatMonitor::close();

  if (_notification != nil) {
    NSUserNotificationCenter *center = [NSUserNotificationCenter defaultUserNotificationCenter];
    [center removeDeliveredNotification:_notification];
    [_notification release];
    _notification = nil;
  }

  close_all_graphs();

  if (_speed_menu_item != nullptr) {
    [_main_menu removeItem:_speed_menu_item];
    [_speed_menu_item release];
    _speed_menu_item = nullptr;
  }

  for (MacStatsChartMenu *chart_menu : _chart_menus) {
    chart_menu->remove_from_menu(_main_menu);
    delete chart_menu;
  }
  _chart_menus.clear();

  set_show_status_item(false);

  for (auto &item : _colors) {
    CGColorRelease(item.second._bg[0]);
    CGColorRelease(item.second._bg[1]);
  }
  _colors.clear();

  _next_chart_index = 2;
}

/**
 * Should be redefined to return a descriptive name for the type of
 * PStatsMonitor this is.
 */
std::string MacStatsMonitor::
get_monitor_name() {
  return "MacStats";
}

/**
 * Called after the monitor has been fully set up.  At this time, it will have
 * a valid _client_data pointer, and things like is_alive() and close() will
 * be meaningful.  However, we may not yet know who we're connected to
 * (is_client_known() may return false), and we may not know anything about
 * the threads or collectors we're about to get data on.
 */
void MacStatsMonitor::
initialized() {
}

/**
 * Called when the "hello" message has been received from the client.  At this
 * time, the client's hostname and program name will be known.
 */
void MacStatsMonitor::
got_hello() {
  std::string progname = get_client_progname();
  std::string hostname = get_client_hostname();

  NSUserNotificationCenter *center = [NSUserNotificationCenter defaultUserNotificationCenter];
  _notification = [[NSUserNotification alloc] init];
  _notification.title = @"PStats Server";
  _notification.informativeText = [NSString
    stringWithFormat:@"Connected to %s on %s", progname.c_str(), hostname.c_str()];

  [center deliverNotification:_notification];
}

/**
 * Like got_hello(), this is called when the "hello" message has been received
 * from the client.  At this time, the client's hostname and program name will
 * be known.  However, the client appears to be an incompatible version and
 * the connection will be terminated; the monitor should issue a message to
 * that effect.
 */
void MacStatsMonitor::
got_bad_version(int client_major, int client_minor,
                int server_major, int server_minor) {
  std::ostringstream str;
  str << "Unable to honor connection attempt from "
      << get_client_progname() << " on " << get_client_hostname()
      << ": unsupported PStats version "
      << client_major << "." << client_minor;

  if (server_minor == 0) {
    str << " (server understands version " << server_major
        << "." << server_minor << " only).";
  } else {
    str << " (server understands versions " << server_major
        << ".0 through " << server_major << "." << server_minor << ").";
  }

  std::string message = str.str();

  NSAlert *alert = [[NSAlert alloc] init];
  alert.messageText = @"PStats Error";
  alert.informativeText = [NSString stringWithUTF8String:message.c_str()];
  alert.alertStyle = NSCriticalAlertStyle;
  [alert runModal];
  [alert release];
}

/**
 * Called whenever a new Collector definition is received from the client.
 * Generally, the client will send all of its collectors over shortly after
 * connecting, but there's no guarantee that they will all be received before
 * the first frames are received.  The monitor should be prepared to accept
 * new Collector definitions midstream.
 */
void MacStatsMonitor::
new_collector(int collector_index) {
  for (MacStatsGraph *graph : _graphs) {
    graph->new_collector(collector_index);
  }
}

/**
 * Called whenever a new Thread definition is received from the client.
 * Generally, the client will send all of its threads over shortly after
 * connecting, but there's no guarantee that they will all be received before
 * the first frames are received.  The monitor should be prepared to accept
 * new Thread definitions midstream.
 */
void MacStatsMonitor::
new_thread(int thread_index) {
  MacStatsChartMenu *chart_menu = new MacStatsChartMenu(this, thread_index);
  chart_menu->add_to_menu(_main_menu, _next_chart_index);
  ++_next_chart_index;
  _chart_menus.push_back(chart_menu);
}

/**
 * Called as each frame's data is made available.  There is no guarantee the
 * frames will arrive in order, or that all of them will arrive at all.  The
 * monitor should be prepared to accept frames received out-of-order or
 * missing.
 */
void MacStatsMonitor::
new_data(int thread_index, int frame_number) {
  PStatMonitor::new_data(thread_index, frame_number);

  for (MacStatsGraph *graph : _graphs) {
    graph->new_data(thread_index, frame_number);
  }

  if (!_have_data) {
    open_default_graphs();
    _have_data = true;
  }
}

/**
 * Called when a thread should be removed from the list of threads.
 */
void MacStatsMonitor::
remove_thread(int thread_index) {
  for (ChartMenus::iterator it = _chart_menus.begin(); it != _chart_menus.end(); ++it) {
    MacStatsChartMenu *chart_menu = *it;
    if (chart_menu->get_thread_index() == thread_index) {
      chart_menu->remove_from_menu(_main_menu);
      delete chart_menu;
      _chart_menus.erase(it);
      --_next_chart_index;
      return;
    }
  }
}

/**
 * Called whenever the connection to the client has been lost.  This is a
 * permanent state change.  The monitor should update its display to represent
 * this, and may choose to close down automatically.
 */
void MacStatsMonitor::
lost_connection() {
  NSUserNotificationCenter *center = [NSUserNotificationCenter defaultUserNotificationCenter];

  if (_notification != nil) {
    [center removeDeliveredNotification:_notification];
    [_notification release];
  }

  std::string hostname = get_client_hostname();

  _notification = [[NSUserNotification alloc] init];
  _notification.title = @"PStats Server";
  _notification.informativeText = [NSString
    stringWithFormat:@"Lost connection to %s", hostname.c_str()];

  _notification.additionalActions = @[
    [NSUserNotificationAction actionWithIdentifier:@"new" title:@"New Session"],
    [NSUserNotificationAction actionWithIdentifier:@"quit" title:@"Quit PStats"]
  ];

  [center deliverNotification:_notification];
}

/**
 * If has_idle() returns true, this will be called periodically to allow the
 * monitor to update its display or whatever it needs to do.
 */
void MacStatsMonitor::
idle() {
  // Check if any of our chart menus need updating.
  for (MacStatsChartMenu *chart_menu : _chart_menus) {
    chart_menu->check_update();
  }

  // Update the frame rate label from the main thread (thread 0).
  if (_frame_rate_status_item != nil) {
    const PStatThreadData *thread_data = get_client_data()->get_thread_data(0);
    double frame_rate = thread_data->get_frame_rate();
    if (frame_rate != 0.0f) {
      _frame_rate_status_item.button.title =
        [NSString stringWithFormat:@"%0.1f ms / %0.1f Hz", 1000.0f / frame_rate, frame_rate];
    }
  }
}

/**
 * Should be redefined to return true if you want to redefine idle() and
 * expect it to be called.
 */
bool MacStatsMonitor::
has_idle() {
  return true;
}

/**
 * Called when the user guide bars have been changed.
 */
void MacStatsMonitor::
user_guide_bars_changed() {
  for (MacStatsGraph *graph : _graphs) {
    graph->user_guide_bars_changed();
  }
}

/**
 * Opens a new timeline.
 */
PStatGraph *MacStatsMonitor::
open_timeline() {
  MacStatsTimeline *graph = new MacStatsTimeline(this);
  add_graph(graph);
  return graph;
}

/**
 * Opens a new flame graph showing the indicated data.
 */
PStatGraph *MacStatsMonitor::
open_flame_graph(int thread_index, int collector_index, int frame_number) {
  MacStatsFlameGraph *graph =
    new MacStatsFlameGraph(this, thread_index, collector_index, frame_number);
  add_graph(graph);
  return graph;
}

/**
 * Opens a new strip chart showing the indicated data.
 */
PStatGraph *MacStatsMonitor::
open_strip_chart(int thread_index, int collector_index, bool show_level) {
  MacStatsStripChart *graph =
    new MacStatsStripChart(this, thread_index, collector_index, show_level);
  add_graph(graph);
  return graph;
}

/**
 * Opens a new piano roll showing the indicated data.
 */
PStatGraph *MacStatsMonitor::
open_piano_roll(int thread_index) {
  MacStatsPianoRoll *graph = new MacStatsPianoRoll(this, thread_index);
  add_graph(graph);
  return graph;
}

/**
 * Returns a color suitable for drawing the indicated collector.
 */
CGColorRef MacStatsMonitor::
get_collector_color(int collector_index, bool highlight) {
  Colors::iterator bi;
  bi = _colors.find(collector_index);
  if (bi != _colors.end()) {
    return (*bi).second._bg[highlight];
  }

  LRGBColor rgb = PStatMonitor::get_collector_color(collector_index);
  rgb[0] = encode_sRGB_float((float)rgb[0]);
  rgb[1] = encode_sRGB_float((float)rgb[1]);
  rgb[2] = encode_sRGB_float((float)rgb[2]);

  PN_stdfloat bright = rgb.dot(LRGBColor(0.2126, 0.7152, 0.0722));
  CGColorRef color = CGColorCreateGenericRGB(rgb[0], rgb[1], rgb[2], 1.0);

  rgb *= 0.75;
  CGColorRef hcolor = CGColorCreateGenericRGB(rgb[0], rgb[1], rgb[2], 1.0);

  ColorSet &set = _colors[collector_index];
  set._bg[0] = color;
  set._bg[1] = hcolor;

  if (bright >= 0.5) {
    set._fg[0] = [NSColor blackColor];
  } else {
    set._fg[0] = [NSColor whiteColor];
  }

  if (bright * 0.75 >= 0.5) {
    set._fg[1] = [NSColor blackColor];
  } else {
    set._fg[1] = [NSColor whiteColor];
  }

  return highlight ? hcolor : color;
}

/**
 * Returns a color suitable for drawing text on the indicated collector.
 */
NSColor *MacStatsMonitor::
get_collector_text_color(int collector_index, bool highlight) {
  get_collector_color(collector_index, false);
  return _colors[collector_index]._fg[highlight];
}

/**
 * Opens a dialog to change the given collector color.
 */
void MacStatsMonitor::
choose_collector_color(int collector_index) {
  _choosing_color_collector_index = collector_index;

  const LRGBColor &rgb = PStatMonitor::get_collector_color(collector_index);
  NSColor *color = [NSColor colorWithSRGBRed:rgb[0] green:rgb[1] blue:rgb[2] alpha:1.0];

  NSColorPanel *panel = [NSColorPanel sharedColorPanel];
  panel.target = [NSApp delegate];
  panel.action = @selector(handleChooseCollectorColor:);
  panel.showsAlpha = NO;
  panel.color = color;
  [NSApp orderFrontColorPanel:panel];
}

/**
 * Sets a custom color associated with the given collector.
 */
void MacStatsMonitor::
handle_choose_collector_color(const LRGBColor &color) {
  int collector_index = _choosing_color_collector_index;

  PStatMonitor::set_collector_color(collector_index, color);

  if (_colors.erase(collector_index)) {
    for (MacStatsGraph *graph : _graphs) {
      graph->reset_collector_color(collector_index);
    }
  }
}

/**
 * Resets the color of the given collector to the default.
 */
void MacStatsMonitor::
reset_collector_color(int collector_index) {
  PStatMonitor::clear_collector_color(collector_index);

  if (_colors.erase(collector_index)) {
    for (MacStatsGraph *graph : _graphs) {
      graph->reset_collector_color(collector_index);
    }
  }
}

/**
 * Enables the frame rate label on the right end of the menu bar.  This is
 * used as a text label to display the main thread's frame rate to the user,
 * although it is implemented as a right-justified toplevel menu item that
 * doesn't open to anything.
 */
void MacStatsMonitor::
set_show_status_item(bool show) {
  if (show && _frame_rate_status_item == nil) {
    _frame_rate_status_item = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
    _frame_rate_status_item.button.action = @selector(handleClickStatusItem:);
    [_frame_rate_status_item retain];

    const PStatClientData *client_data = get_client_data();
    if (client_data != nullptr) {
      const PStatThreadData *thread_data = client_data->get_thread_data(0);
      if (thread_data != nullptr) {
        double frame_rate = thread_data->get_frame_rate();
        if (frame_rate != 0.0f) {
          _frame_rate_status_item.button.title =
            [NSString stringWithFormat:@"%0.1f ms / %0.1f Hz", 1000.0f / frame_rate, frame_rate];
        }
      }
    }
  }
  if (!show && _frame_rate_status_item != nil) {
    // Careful: for some reason, this can get called recursively.
    NSStatusItem *status_item = _frame_rate_status_item;
    _frame_rate_status_item = nil;
    [[NSStatusBar systemStatusBar] removeStatusItem:status_item];
    [status_item release];
    status_item = nil;
  }
}

/**
 * Called when the user selects a new time units from the monitor pulldown
 * menu, this should adjust the units for all graphs to the indicated mask if
 * it is a time-based graph.
 */
void MacStatsMonitor::
set_time_units(int unit_mask) {
  for (MacStatsGraph *graph : _graphs) {
    graph->set_time_units(unit_mask);
  }
}

/**
 * Called when the user selects a new scroll speed from the monitor pulldown
 * menu, this should adjust the speeds for all graphs to the indicated value.
 */
void MacStatsMonitor::
set_scroll_speed(double scroll_speed) {
  _scroll_speed = scroll_speed;

  // First, change all of the open graphs appropriately.
  for (MacStatsGraph *graph : _graphs) {
    graph->set_scroll_speed(_scroll_speed);
  }

  // Then, change the state of the menu items.
  _speed_menu_item_1.state = (scroll_speed == 1.0);
  _speed_menu_item_2.state = (scroll_speed == 2.0);
  _speed_menu_item_3.state = (scroll_speed == 3.0);
  _speed_menu_item_6.state = (scroll_speed == 6.0);
  _speed_menu_item_12.state = (scroll_speed == 12.0);
}

/**
 * Called when the user selects a pause on or pause off option from the menu.
 */
void MacStatsMonitor::
set_pause(bool pause) {
  _pause = pause;

  // First, change all of the open graphs appropriately.
  for (MacStatsGraph *graph : _graphs) {
    graph->set_pause(_pause);
  }

  // Then, change the state of the menu item.
  _speed_menu_item_pause.state = pause;
}

/**
 * Adds the newly-created graph to the list of managed graphs.
 */
void MacStatsMonitor::
add_graph(MacStatsGraph *graph) {
  _graphs.insert(graph);

  int units = [[NSUserDefaults standardUserDefaults] integerForKey:@"TimeUnits"];
  graph->set_time_units(units);
  graph->set_scroll_speed(_scroll_speed);
  graph->set_pause(_pause);
}

/**
 * Deletes the indicated graph.
 */
void MacStatsMonitor::
remove_graph(MacStatsGraph *graph) {
  Graphs::iterator gi = _graphs.find(graph);
  if (gi != _graphs.end()) {
    _graphs.erase(gi);
    delete graph;
  }
}

/**
 * Asks all open graphs to close.
 */
void MacStatsMonitor::
close_all_graphs() {
  while (!_graphs.empty()) {
    (*_graphs.begin())->close();
  }
}

/**
 * Creates the "Speed" pulldown menu.
 */
void MacStatsMonitor::
setup_speed_menu() {
  NSMenu *menu = [[NSMenu alloc] init];
  menu.title = @"Speed";
  menu.autoenablesItems = NO;

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(handleSpeed:);
    item.title = @"1";
    item.tag = 1;
    [menu addItem:item];
    [item release];

    _speed_menu_item_1 = item;
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(handleSpeed:);
    item.title = @"2";
    item.tag = 2;
    [menu addItem:item];
    [item release];

    _speed_menu_item_2 = item;
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(handleSpeed:);
    item.title = @"3";
    item.tag = 3;
    item.state = NSOnState;
    [menu addItem:item];
    [item release];

    _speed_menu_item_3 = item;
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(handleSpeed:);
    item.title = @"6";
    item.tag = 6;
    [menu addItem:item];
    [item release];

    _speed_menu_item_6 = item;
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(handleSpeed:);
    item.title = @"12";
    item.tag = 12;
    [menu addItem:item];
    [item release];

    _speed_menu_item_12 = item;
  }

  [menu addItem:[NSMenuItem separatorItem]];

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(handlePause:);
    item.title = @"pause";
    [menu addItem:item];
    [item release];

    _speed_menu_item_pause = item;
  }

  NSMenuItem *item = [[NSMenuItem alloc] init];
  _speed_menu_item = item;
  [_main_menu addItem:item];
  [_main_menu setSubmenu:menu forItem:item];
  [menu release];

  set_scroll_speed(3);
  set_pause(false);

  ++_next_chart_index;
}
