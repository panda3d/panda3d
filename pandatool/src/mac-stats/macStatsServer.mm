/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsServer.mm
 * @author rdb
 * @date 2023-08-17
 */

#include "macStatsServer.h"
#include "macStatsMonitor.h"
#include "macStatsAppDelegate.h"
#include "pandaVersion.h"
#include "pStatGraph.h"
#include "config_pstatclient.h"

#include <unistd.h>

/**
 *
 */
MacStatsServer::
MacStatsServer() : _port(pstats_port) {
  set_program_brief("macOS PStats client");
  set_program_description
    ("This is a GUI-based PStats server that listens on a TCP port for a "
     "connection from a PStatClient in a Panda3D application.  It offers "
     "various graphs for showing the timing information sent by the client."
     "\n\n"
     "The full documentation is available online:\n  "
#ifdef HAVE_PYTHON
     "https://docs.panda3d.org/" PANDA_ABI_VERSION_STR "/python/optimization/pstats"
#else
     "https://docs.panda3d.org/" PANDA_ABI_VERSION_STR "/cpp/optimization/pstats"
#endif
     "");

  add_option
    ("p", "port", 0,
     "Specify the TCP port to listen for connections on.  By default, this "
     "is taken from the pstats-port Config variable.",
     &ProgramBase::dispatch_int, nullptr, &_port);

  add_runline("[-p 5185]");
  add_runline("session.pstats");

  _last_session = Filename::expand_from(
    "$HOME/Library/Caches/Panda3D-" PANDA_ABI_VERSION_STR "/last-session.pstats");
  _last_session.set_binary();

  create_app();
}

/**
 * Runs the server.
 */
void MacStatsServer::
run(int argc, char *argv[]) {
  if (parse_command_line(argc, argv, isatty(STDERR_FILENO)) == ProgramBase::EC_failure) {
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = @"PStats Error";
    alert.informativeText = @"Failed to parse command-line options.";
    alert.alertStyle = NSCriticalAlertStyle;
    [alert runModal];
    [alert release];
    return;
  }

  [_app run];
}

/**
 * Does something with the additional arguments on the command line (after all
 * the -options have been parsed).  Returns true if the arguments are good,
 * false otherwise.
 */
bool MacStatsServer::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    // applicationDidFinishLaunching will call new_session().
    return true;
  }
  else if (args.size() == 1) {
    // Handled by application:openFile:
    return true;
  }
  else {
    nout << "At most one filename may be specified on the command-line.\n";
    return false;
  }
}

/**
 *
 */
PStatMonitor *MacStatsServer::
make_monitor(const NetAddress &address) {
  // Enable the "New Session", "Save Session" and "Close Session" menu items.
  _new_session_menu_item.enabled = YES;
  _save_session_menu_item.enabled = YES;
  _close_session_menu_item.enabled = YES;
  _export_session_menu_item.enabled = YES;

  NSUserNotificationCenter *center = [NSUserNotificationCenter defaultUserNotificationCenter];
  if (_listening_notification != nil) {
    [center removeDeliveredNotification:_listening_notification];
    [_listening_notification release];
    _listening_notification = nil;
  }

  _monitor = new MacStatsMonitor(this);
  return _monitor;
}

/**
 * Called when connection has been lost.
 */
void MacStatsServer::
lost_connection(PStatMonitor *monitor) {
  if (_monitor != nullptr && !_monitor->_have_data) {
    // We didn't have any data yet.  Just silently restart the session.
    _monitor->close();
    _monitor = nullptr;
    if (new_session()) {
      return;
    }
  } else {
    // Store a backup now, in case PStats crashes or something.
    _last_session.make_dir();
    if (monitor->write(_last_session)) {
      nout << "Wrote to " << _last_session << "\n";
    } else {
      nout << "Failed to write to " << _last_session << "\n";
    }
  }

  stop_listening();
}

/**
 * Starts a new session.
 */
bool MacStatsServer::
new_session() {
  if (!close_session()) {
    return false;
  }

  if (listen(_port)) {
    NSUserNotification *notification = [[NSUserNotification alloc] init];
    notification.title = @"PStats Server";
    notification.informativeText = [NSString stringWithFormat:@"Waiting for client to connect on port %d\u2026", _port];

    // Quick way to do something else if the user prefers.
    notification.additionalActions = @[
      [NSUserNotificationAction actionWithIdentifier:@"open" title:@"Open Session\u2026"],
      [NSUserNotificationAction actionWithIdentifier:@"openLast" title:@"Open Last Session"],
      [NSUserNotificationAction actionWithIdentifier:@"quit" title:@"Quit PStats"]
    ];

    NSUserNotificationCenter *center = [NSUserNotificationCenter defaultUserNotificationCenter];
    [center deliverNotification:notification];
    _listening_notification = notification;

    _new_session_menu_item.enabled = NO;
    _save_session_menu_item.enabled = NO;
    _close_session_menu_item.enabled = YES;
    _export_session_menu_item.enabled = NO;

    return true;
  }

  NSAlert *alert = [[NSAlert alloc] init];
  alert.messageText = @"PStats Error";
  alert.informativeText = [NSString stringWithFormat:@"Unable to open port %d.  Try specifying a different port number using pstats-port in your Config file or the -p option on the command-line.", _port];
  alert.alertStyle = NSCriticalAlertStyle;
  [alert runModal];
  [alert release];

  return false;
}

/**
 * Opens a session with the given filename.
 */
bool MacStatsServer::
open_session(const Filename &fn) {
  if (!close_session()) {
    return false;
  }

  MacStatsMonitor *monitor = new MacStatsMonitor(this);
  if (!monitor->read(fn)) {
    delete monitor;
    return false;
  }

  _save_filename = fn;
  _new_session_menu_item.enabled = YES;
  _save_session_menu_item.enabled = YES;
  _close_session_menu_item.enabled = YES;
  _export_session_menu_item.enabled = YES;

  _monitor = monitor;

  // If the file contained no graphs, open the default graphs.
  if (monitor->_graphs.empty()) {
    monitor->open_default_graphs();
  }

  return true;
}

/**
 * Offers to open an existing session.
 */
bool MacStatsServer::
open_session() {
  if (!close_session()) {
    return false;
  }

  NSOpenPanel *panel = [NSOpenPanel openPanel];
  panel.title = @"Open Session";

  if ([panel runModal] == NSModalResponseOK) {
    NSString *path = [[panel.URLs firstObject] path];
    Filename fn([path UTF8String]);
    fn.set_binary();
    if (open_session(fn)) {
      return true;
    }

    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = @"PStats Error";
    alert.informativeText = [NSString stringWithFormat:@"Failed to load session file: %s", fn.c_str()];
    alert.alertStyle = NSCriticalAlertStyle;
    [alert runModal];
    [alert release];
  }

  return false;
}

/**
 * Opens the last session, if any.
 */
bool MacStatsServer::
open_last_session() {
  if (open_session(_last_session)) {
    return true;
  }

  NSAlert *alert = [[NSAlert alloc] init];
  alert.messageText = @"PStats Error";
  alert.informativeText = [NSString stringWithFormat:@"Failed to load session file: %s", _last_session.c_str()];
  alert.alertStyle = NSCriticalAlertStyle;
  [alert runModal];
  [alert release];

  return false;
}

/**
 * Offers to save the current session.
 */
bool MacStatsServer::
save_session() {
  nassertr_always(_monitor != nullptr, true);

  NSSavePanel *panel = [NSSavePanel savePanel];
  panel.title = @"Save Session";

  if (_save_filename.empty()) {
    panel.nameFieldStringValue = @"session.pstats";
  } else {
    std::string dirname = _save_filename.get_dirname();
    std::string basename = _save_filename.get_basename();
    panel.nameFieldStringValue = [NSString stringWithUTF8String:basename.c_str()];
    panel.directoryURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:dirname.c_str()]];
  }

  if ([panel runModal] == NSModalResponseOK) {
    NSString *path = [panel.URL path];
    Filename fn([path UTF8String]);
    fn.set_binary();

    if (!_monitor->write(fn)) {
      NSAlert *alert = [[NSAlert alloc] init];
      alert.messageText = @"PStats Error";
      alert.informativeText = [NSString stringWithFormat:@"Failed to save session file: %s", fn.c_str()];
      alert.alertStyle = NSCriticalAlertStyle;
      [alert runModal];
      [alert release];

      return false;
    }
    _save_filename = fn;
    _monitor->get_client_data()->clear_dirty();
    return true;
  }

  return false;
}

/**
 * Offers to export the current session as a JSON file.
 */
bool MacStatsServer::
export_session() {
  nassertr_always(_monitor != nullptr, true);

  NSSavePanel *panel = [NSSavePanel savePanel];
  panel.title = @"Export Session";
  panel.nameFieldStringValue = @"session.json";

  if ([panel runModal] == NSModalResponseOK) {
    NSString *path = [panel.URL path];
    Filename fn([path UTF8String]);
    fn.set_binary();

    std::ofstream stream;
    if (!fn.open_write(stream)) {
      NSAlert *alert = [[NSAlert alloc] init];
      alert.messageText = @"PStats Error";
      alert.informativeText = [NSString stringWithFormat:@"Failed to open file for export: %s", fn.c_str()];
      alert.alertStyle = NSCriticalAlertStyle;
      [alert runModal];
      [alert release];

      return false;
    }

    int pid = _monitor->get_client_pid();
    _monitor->get_client_data()->write_json(stream, std::max(0, pid));
    stream.close();
    return true;
  }

  return false;
}

/**
 * Closes the current session.
 */
bool MacStatsServer::
close_session() {
  bool wrote_last_session = false;

  if (_monitor != nullptr) {
    const PStatClientData *client_data = _monitor->get_client_data();
    if (client_data != nullptr && client_data->is_dirty()) {
      if (!_monitor->has_read_filename()) {
        _last_session.make_dir();
        if (_monitor->write(_last_session)) {
          nout << "Wrote to " << _last_session << "\n";
          wrote_last_session = true;
        }
        else {
          nout << "Failed to write to " << _last_session << "\n";
        }
      }

      // Make sure the alert goes on top.
      [_app activateIgnoringOtherApps:YES];

      NSAlert *alert = [[NSAlert alloc] init];
      alert.messageText = @"Unsaved Data";
      alert.informativeText = @"Would you like to save the currently open session?";
      [alert addButtonWithTitle:@"Save\u2026"];
      [alert addButtonWithTitle:@"Don't Save"];
      [alert addButtonWithTitle:@"Cancel"];
      int response = [alert runModal];
      [alert release];

      if ((response != 1000 && response != 1001) ||
          (response == 1000 && !save_session())) {
        return false;
      }
    }

    _monitor->close();
    _monitor = nullptr;
  }

  _save_filename = Filename();
  stop_listening();

  if (_listening_notification != nil) {
    NSUserNotificationCenter *center = [NSUserNotificationCenter defaultUserNotificationCenter];
    [center removeDeliveredNotification:_listening_notification];
    [_listening_notification release];
    _listening_notification = nil;
  }

  _new_session_menu_item.enabled = YES;
  if (wrote_last_session) {
    _open_last_session_menu_item.enabled = YES;
  }
  _save_session_menu_item.enabled = NO;
  _close_session_menu_item.enabled = NO;
  _export_session_menu_item.enabled = NO;
  return true;
}

/**
 * Enables the frame rate label on the right end of the menu bar.  This is
 * used as a text label to display the main thread's frame rate to the user,
 * although it is implemented as a right-justified toplevel menu item that
 * doesn't open to anything.
 */
void MacStatsServer::
set_show_status_item(bool show) {
  if (_monitor != nullptr) {
    _monitor->set_show_status_item(show);
  }

  _show_status_item_menu_item.state = show ? NSOnState : NSOffState;
}

/**
 *
 */
void MacStatsServer::
set_appearance(NSString *name) {
  if (@available(macOS 10.14, *)) {
    if (name == nil || name.length == 0) {
      [_app setAppearance:nil];
      _appearance_system_menu_item.state = NSOnState;
    } else {
      [_app setAppearance:[NSAppearance appearanceNamed:name]];
      _appearance_system_menu_item.state = NSOffState;
    }

    _appearance_aqua_menu_item.state = (name != nil && [name isEqual:@"NSAppearanceNameAqua"]) ? NSOnState : NSOffState;
    _appearance_dark_aqua_menu_item.state = (name != nil && [name isEqual:@"NSAppearanceNameDarkAqua"]) ? NSOnState : NSOffState;
  }
}

/**
 * Called when the user selects a new time units from the monitor pulldown
 * menu, this should adjust the units for all graphs to the indicated mask if
 * it is a time-based graph.
 */
void MacStatsServer::
set_time_units(int unit_mask) {
  if (_monitor != nullptr) {
    _monitor->set_time_units(unit_mask);
  }

  _units_ms_menu_item.state = (unit_mask & PStatGraph::GBU_ms) ? NSOnState : NSOffState;
  _units_hz_menu_item.state = (unit_mask & PStatGraph::GBU_hz) ? NSOnState : NSOffState;
}

/**
 * Creates the menu bar for this monitor.
 */
void MacStatsServer::
create_app() {
  _app = [NSApplication sharedApplication];

  MacStatsAppDelegate *delegate = [[MacStatsAppDelegate alloc] initWithServer:this];
  [_app setDelegate:delegate];
  [_app setActivationPolicy:NSApplicationActivationPolicyRegular];

  _main_menu = [[NSMenu alloc] init];

  NSMenu *app_menu = [[NSMenu alloc] init];

  NSMenu *settings_menu = [[NSMenu alloc] init];

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(handleToggleSettingsBool:);
    item.representedObject = @"ShowStatusItem";
    item.title = @"Show in Status Bar";
    item.state = NSOnState;
    [settings_menu addItem:item];

    _show_status_item_menu_item = item;
    [item release];
  }

  NSMenu *units_menu;
  if (@available(macOS 14.0, *)) {
    // On Sonnoma, use a section with header.
    [settings_menu addItem:[NSMenuItem separatorItem]];
    [settings_menu addItem:[NSMenuItem sectionHeaderWithTitle:@"Time Units"]];
    units_menu = settings_menu;
  } else {
    // On older versions, create a sub-menu.
    units_menu = [[NSMenu alloc] init];
    units_menu.autoenablesItems = NO;

    NSMenuItem *units_item = [[NSMenuItem alloc] init];
    units_item.title = @"Time Units";

    [settings_menu addItem:units_item];
    [settings_menu setSubmenu:units_menu forItem:units_item];
    [units_item release];
    [units_menu release];
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(handleSettingsInteger:);
    item.representedObject = @"TimeUnits";
    item.tag = PStatGraph::GBU_ms;
    item.title = @"ms";
    item.state = NSOnState;
    [units_menu addItem:item];

    _units_ms_menu_item = item;
    [item release];
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(handleSettingsInteger:);
    item.representedObject = @"TimeUnits";
    item.tag = PStatGraph::GBU_hz;
    item.title = @"Hz";
    item.state = NSOffState;
    [units_menu addItem:item];

    _units_hz_menu_item = item;
    [item release];
  }

  // Dark mode available from Mojave.
  if (@available(macOS 10.14, *)) {
    NSMenu *appearance_menu;
    if (@available(macOS 14.0, *)) {
      // On Sonnoma, use a section with header.
      [settings_menu addItem:[NSMenuItem separatorItem]];
      [settings_menu addItem:[NSMenuItem sectionHeaderWithTitle:@"Appearance"]];
      appearance_menu = settings_menu;
    } else {
      // On older versions, create a sub-menu.
      appearance_menu = [[NSMenu alloc] init];
      appearance_menu.autoenablesItems = NO;

      NSMenuItem *appearance_item = [[NSMenuItem alloc] init];
      appearance_item.title = @"Appearance";

      [settings_menu addItem:appearance_item];
      [settings_menu setSubmenu:appearance_menu forItem:appearance_item];
      [appearance_item release];
      [appearance_menu release];
    }

    {
      NSMenuItem *item = [[NSMenuItem alloc] init];
      item.action = @selector(handleSettingsAppearance:);
      item.representedObject = @"";
      item.title = @"System";
      item.state = NSOnState;
      [appearance_menu addItem:item];
      _appearance_system_menu_item = item;
      [item release];
    }

    {
      NSMenuItem *item = [[NSMenuItem alloc] init];
      item.action = @selector(handleSettingsAppearance:);
      item.representedObject = @"NSAppearanceNameAqua";
      item.title = @"Aqua";
      item.state = NSOffState;
      [appearance_menu addItem:item];
      _appearance_aqua_menu_item = item;
      [item release];
    }

    {
      NSMenuItem *item = [[NSMenuItem alloc] init];
      item.action = @selector(handleSettingsAppearance:);
      item.representedObject = @"NSAppearanceNameDarkAqua";
      item.title = @"Dark Aqua";
      item.state = NSOffState;
      [appearance_menu addItem:item];
      _appearance_dark_aqua_menu_item = item;
      [item release];
    }
  }

  NSMenuItem *settings_item = [[NSMenuItem alloc] init];
  settings_item.title = @"Settings";
  [app_menu addItem:settings_item];
  [app_menu setSubmenu:settings_menu forItem:settings_item];
  [settings_item release];
  [settings_menu release];

  [app_menu addItem:[NSMenuItem separatorItem]];

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(hide:);
    item.keyEquivalent = @"h";
    item.title = @"Hide PStats";
    [app_menu addItem:item];
    [item release];
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(hideOtherApplications:);
    item.keyEquivalent = @"h";
    item.keyEquivalentModifierMask |= NSAlternateKeyMask;
    item.title = @"Hide Others";
    [app_menu addItem:item];
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(unhideAllApplications:);
    item.title = @"Show All";
    [app_menu addItem:item];
    [item release];
  }

  [app_menu addItem:[NSMenuItem separatorItem]];

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(terminate:);
    item.keyEquivalent = @"q";
    item.title = @"Quit PStats";
    [app_menu addItem:item];
    [item release];
  }

  NSMenuItem *app_menu_item = [[NSMenuItem alloc] init];
  [_main_menu addItem:app_menu_item];
  [_main_menu setSubmenu:app_menu forItem:app_menu_item];
  [app_menu_item release];
  [app_menu release];

  setup_session_menu();

  [_app setMainMenu:_main_menu];
  [_main_menu release];

  //[_app activateIgnoringOtherApps:YES];
  //[_app finishLaunching];

  set_time_units(PStatGraph::GBU_ms);
}

/**
 * Creates the "Session" pulldown menu.
 */
void MacStatsServer::
setup_session_menu() {
  NSMenu *submenu = [[NSMenu alloc] init];
  submenu.title = @"Session";
  submenu.autoenablesItems = NO;

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(handleNewSession:);
    item.keyEquivalent = @"n";
    item.title = @"New Session";
    [submenu addItem:item];

    _new_session_menu_item = item;
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(handleOpenSession:);
    item.keyEquivalent = @"o";
    item.title = @"Open Session\u2026";
    [submenu addItem:item];
    [item release];
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(handleOpenLastSession:);
    item.title = @"Open Last Session";
    item.enabled = _last_session.exists();
    [submenu addItem:item];

    _open_last_session_menu_item = item;
    [item release];
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(handleSaveSession:);
    item.keyEquivalent = @"s";
    item.title = @"Save Session\u2026";
    item.enabled = NO;
    [submenu addItem:item];

    _save_session_menu_item = item;
    [item release];
  }

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(handleCloseSession:);
    item.keyEquivalent = @"w";
    item.title = @"Close Session";
    item.enabled = NO;
    [submenu addItem:item];

    _close_session_menu_item = item;
    [item release];
  }

  [submenu addItem:[NSMenuItem separatorItem]];

  {
    NSMenuItem *item = [[NSMenuItem alloc] init];
    item.action = @selector(handleExportSession:);
    item.title = @"Export as JSON\u2026";
    item.enabled = NO;
    [submenu addItem:item];

    _export_session_menu_item = item;
    [item release];
  }

  NSMenuItem *item = [[NSMenuItem alloc] init];
  [_main_menu addItem:item];
  [_main_menu setSubmenu:submenu forItem:item];
}
