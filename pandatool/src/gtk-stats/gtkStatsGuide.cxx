// Filename: gtkStatsGuide.cxx
// Created by:  drose (16Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "gtkStatsGuide.h"

#include "pStatStripChart.h"

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsGuide::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkStatsGuide::
GtkStatsGuide(PStatStripChart *chart) :
  _chart(chart)
{
  set_events(GDK_EXPOSURE_MASK);

  // Choose a suitable minimum width.  This requires knowing what the
  // font will be.
  Gdk_Font font = get_style()->gtkobj()->font;
  int text_width = font.string_width("0000");
  set_usize(text_width, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsGuide::configure_event_impl
//       Access: Private, Virtual
//  Description: Creates a new backing pixmap of the appropriate size.
////////////////////////////////////////////////////////////////////
gint GtkStatsGuide::
configure_event_impl(GdkEventConfigure *) {
  Gdk_Window window = get_window();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsGuide::expose_event_impl
//       Access: Private, Virtual
//  Description: Redraw the text.  We don't bother with clipping
//               regions here, but just draw the whole thing every
//               time.
////////////////////////////////////////////////////////////////////
gint GtkStatsGuide::
expose_event_impl(GdkEventExpose *event) {
  Gdk_GC fg_gc =
    get_style()->gtkobj()->fg_gc[GTK_WIDGET_STATE (GTK_WIDGET(gtkobj()))];
  Gdk_GC bg_gc =
    get_style()->gtkobj()->bg_gc[GTK_WIDGET_STATE (GTK_WIDGET(gtkobj()))];

  if (fg_gc && bg_gc) {
    Gdk_Window window = get_window();
    window.draw_rectangle(bg_gc, true, 0, 0, width(), height());
    
    Gdk_Font font = fg_gc.get_font();
    int text_ascent = font.ascent();
    
    int num_guide_bars = _chart->get_num_guide_bars();
    for (int i = 0; i < num_guide_bars; i++) {
      const PStatStripChart::GuideBar &bar = _chart->get_guide_bar(i);
      int y = _chart->height_to_pixel(bar._height);
      
      if (y >= 5) {
        // Only draw it if it's not too close to the top.
        window.draw_string(font, fg_gc, 0, y + text_ascent / 2,
                           bar._label);
      }
    }
  }

  return false;
}
