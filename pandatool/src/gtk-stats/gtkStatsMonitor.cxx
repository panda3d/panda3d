// Filename: gtkStatsMonitor.cxx
// Created by:  drose (14Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "gtkStatsMonitor.h"
#include "gtkStatsWindow.h"
#include "gtkStatsStripWindow.h"
#include "gtkStatsBadVersionWindow.h"

#include <luse.h>
#include <pStatCollectorDef.h>

#include <gdk--.h>

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMonitor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkStatsMonitor::
GtkStatsMonitor() {
  _destructing = false;
  _new_collector = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMonitor::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkStatsMonitor::
~GtkStatsMonitor() {
  _destructing = true;

  Windows::iterator wi;
  for (wi = _windows.begin(); wi != _windows.end(); ++wi) {
    GtkStatsWindow *window = (*wi);
    window->destruct();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMonitor::close_all_windows
//       Access: Public
//  Description: Closes all the windows associated with this client.
//               This returns a PointerTo itself, just to guarantee
//               that the monitor won't destruct until the function
//               returns (as it might, if there were no other pointers
//               to it).
////////////////////////////////////////////////////////////////////
PT(PStatMonitor) GtkStatsMonitor::
close_all_windows() {
  PT(PStatMonitor) temp = this;
  Windows::iterator wi;
  for (wi = _windows.begin(); wi != _windows.end(); ++wi) {
    GtkStatsWindow *window = (*wi);
    window->destruct();
  }
  return temp;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMonitor::get_monitor_name
//       Access: Public, Virtual
//  Description: Should be redefined to return a descriptive name for
//               the type of PStatsMonitor this is.
////////////////////////////////////////////////////////////////////
string GtkStatsMonitor::
get_monitor_name() {
  return "Gtk Stats";
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMonitor::initialized
//       Access: Public, Virtual
//  Description: Called after the monitor has been fully set up.  At
//               this time, it will have a valid _client_data pointer,
//               and things like is_alive() and close() will be
//               meaningful.  However, we may not yet know who we're
//               connected to (is_client_known() may return false),
//               and we may not know anything about the threads or
//               collectors we're about to get data on.
////////////////////////////////////////////////////////////////////
void GtkStatsMonitor::
initialized() {
  // Create a default window: a strip chart for the main thread.
  new GtkStatsStripWindow(this, 0, 0, false, 400, 100);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMonitor::got_hello
//       Access: Public, Virtual
//  Description: Called when the "hello" message has been received
//               from the client.  At this time, the client's hostname
//               and program name will be known.
////////////////////////////////////////////////////////////////////
void GtkStatsMonitor::
got_hello() {
  Windows::iterator wi;
  for (wi = _windows.begin(); wi != _windows.end(); ++wi) {
    (*wi)->update_title();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMonitor::got_bad_version
//       Access: Public, Virtual
//  Description: Like got_hello(), this is called when the "hello"
//               message has been received from the client.  At this
//               time, the client's hostname and program name will be
//               known.  However, the client appears to be an
//               incompatible version and the connection will be
//               terminated; the monitor should issue a message to
//               that effect.
////////////////////////////////////////////////////////////////////
void GtkStatsMonitor::
got_bad_version(int client_major, int client_minor,
                int server_major, int server_minor) {
  new GtkStatsBadVersionWindow(this, client_major, client_minor,
                               server_major, server_minor);
  close_all_windows();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMonitor::new_collector
//       Access: Public, Virtual
//  Description: Called whenever a new Collector definition is
//               received from the client.  Generally, the client will
//               send all of its collectors over shortly after
//               connecting, but there's no guarantee that they will
//               all be received before the first frames are received.
//               The monitor should be prepared to accept new Collector
//               definitions midstream.
////////////////////////////////////////////////////////////////////
void GtkStatsMonitor::
new_collector(int) {
  _new_collector = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMonitor::new_data
//       Access: Public, Virtual
//  Description: Called as each frame's data is made available.  There
//               is no guarantee the frames will arrive in order, or
//               that all of them will arrive at all.  The monitor
//               should be prepared to accept frames received
//               out-of-order or missing.
////////////////////////////////////////////////////////////////////
void GtkStatsMonitor::
new_data(int thread_index, int frame_number) {
}


////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMonitor::lost_connection
//       Access: Public, Virtual
//  Description: Called whenever the connection to the client has been
//               lost.  This is a permanent state change.  The monitor
//               should update its display to represent this, and may
//               choose to close down automatically.
////////////////////////////////////////////////////////////////////
void GtkStatsMonitor::
lost_connection() {
  Windows::iterator wi;
  for (wi = _windows.begin(); wi != _windows.end(); ++wi) {
    (*wi)->mark_dead();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMonitor::idle
//       Access: Public, Virtual
//  Description: If has_idle() returns true, this will be called
//               periodically to allow the monitor to update its
//               display or whatever it needs to do.
////////////////////////////////////////////////////////////////////
void GtkStatsMonitor::
idle() {
  Windows::iterator wi;
  for (wi = _windows.begin(); wi != _windows.end(); ++wi) {
    (*wi)->idle();
  }
  _new_collector = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMonitor::has_idle
//       Access: Public, Virtual
//  Description: Should be redefined to return true if you want to
//               redefine idle() and expect it to be called.
////////////////////////////////////////////////////////////////////
bool GtkStatsMonitor::
has_idle() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMonitor::is_thread_safe
//       Access: Public, Virtual
//  Description: Should be redefined to return true if this monitor
//               class can handle running in a sub-thread.
//
//               This is not related to the question of whether it can
//               handle multiple different PStatThreadDatas; this is
//               strictly a question of whether or not the monitor
//               itself wants to run in a sub-thread.
////////////////////////////////////////////////////////////////////
bool GtkStatsMonitor::
is_thread_safe() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMonitor::add_window
//       Access: Public
//  Description: Called only from the GtkStatsWindow constructor, this
//               indicates a new window that we should track.
////////////////////////////////////////////////////////////////////
void GtkStatsMonitor::
add_window(GtkStatsWindow *window) {
  nassertv(!_destructing);
  bool inserted = _windows.insert(window).second;
  nassertv(inserted);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMonitor::remove_window
//       Access: Public
//  Description: Called only from the GtkStatsWindow destructor, this
//               indicates the end of a window that we should now no
//               longer track.
//
//               When the last window is deleted, this automatically
//               closes the connection.
////////////////////////////////////////////////////////////////////
void GtkStatsMonitor::
remove_window(GtkStatsWindow *window) {
  if (!_destructing) {
    bool removed = (_windows.erase(window) != 0);
    nassertv(removed);

    if (_windows.empty()) {
      close();
    }
  }
}
