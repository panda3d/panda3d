


/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


/* Modifed for gtk-- by sac@transmeta.com */
/* Modified (slightly) for gdk-- by freyd@uni-muenster.de */
#include <gtk--/main.h>
#include <gtk--/style.h>
#include <gtk--/window.h>
#include <gtk--/button.h>
#include <gtk--/box.h>
#include <gtk--/drawingarea.h>
#include <iostream>


class ScribbleDrawingArea  : public Gtk::DrawingArea
{
  /* Backing pixmap for drawing area */

  Gdk_Pixmap pixmap;
  Gdk_GC gc;
  Gdk_Window win;
  Gdk_Visual visual;

  virtual gint configure_event_impl (GdkEventConfigure *event);
  virtual gint expose_event_impl (GdkEventExpose *event);
  virtual gint button_press_event_impl (GdkEventButton *event);
  virtual gint motion_notify_event_impl (GdkEventMotion *event);
  void draw_brush (gdouble x, gdouble y);

public:
  ScribbleDrawingArea ();

};

ScribbleDrawingArea::ScribbleDrawingArea()
    : Gtk::DrawingArea(), pixmap (0)
  {
    set_events (GDK_EXPOSURE_MASK
                | GDK_LEAVE_NOTIFY_MASK
                | GDK_BUTTON_PRESS_MASK
                | GDK_POINTER_MOTION_MASK
                | GDK_POINTER_MOTION_HINT_MASK);
  }


/* Create a new backing pixmap of the appropriate size */
int ScribbleDrawingArea::configure_event_impl (GdkEventConfigure * /* event */)
  {
    win = get_window();
    visual = win.get_visual();

    if (pixmap)
      pixmap.release();
      gc = get_style()->gtkobj()->white_gc;
      // Gtk::Style has no access to its data members, so use gtk objekt.
      // Some access functions would be nice like GtkStyle::get_white_gc() etc.
    pixmap.create(get_window(),  width(), height());

    pixmap.draw_rectangle (gc,
                        TRUE,
                        0, 0,
                        width(),
                        height());

    return TRUE;
  }

/* Redraw the screen from the backing pixmap */
int ScribbleDrawingArea::expose_event_impl (GdkEventExpose *event)
  {
    
    gc = get_style()->gtkobj()->fg_gc[GTK_WIDGET_STATE (GTK_WIDGET(gtkobj()))];
    // Same like above, + Gtk::Widget has set_state function but no get_state 
    // function. 
    win.draw_pixmap(gc ,
                    pixmap,
                    event->area.x, event->area.y,
                    event->area.x, event->area.y,
                    event->area.width, event->area.height);

    return FALSE;
  }

/* Draw a rectangle on the screen */
void ScribbleDrawingArea::draw_brush (gdouble x, gdouble y)
  {
    GdkRectangle update_rect;
    update_rect.x = (int)x - 5;
    update_rect.y = (int)y - 5;
    update_rect.width = 10;
    update_rect.height = 10;
    gc = get_style()->gtkobj()->black_gc;
    pixmap.draw_rectangle(
                        gc,
                        TRUE,
                        update_rect.x, update_rect.y,
                        update_rect.width, update_rect.height);
    draw(&update_rect);
    //draw (&update_rect);
  }

gint ScribbleDrawingArea::button_press_event_impl (GdkEventButton *event)
  {
    if (event->button == 1 && pixmap)
      draw_brush (event->x, event->y);

    return TRUE;
  }

gint ScribbleDrawingArea::motion_notify_event_impl (GdkEventMotion *event)
  {
    int x, y;
    GdkModifierType state;
    if (event->is_hint)
      gdk_window_get_pointer (event->window, &x, &y, &state);
    else
      {
        x = (int)event->x;
        y = (int)event->y;
        state = (GdkModifierType) event->state;
      }
    
    if (state & GDK_BUTTON1_MASK && pixmap)
      draw_brush (x, y);
  
    return TRUE;
  }


class ScribbleWindow : public Gtk::Window
{

  Gtk::VBox vbox;
  ScribbleDrawingArea drawing_area;
  Gtk::Button button;
  void quit ();
public:  
  ScribbleWindow ();
}; 

void ScribbleWindow::quit ()
  {
    Gtk::Main::quit();
  }

ScribbleWindow::ScribbleWindow ()
    :  Gtk::Window(GTK_WINDOW_TOPLEVEL),
       vbox (FALSE, 0),
       button ("quit")
  {
    add (vbox);

    /* Create the drawing area */
    drawing_area.size (400, 400);
    vbox.pack_start (drawing_area, TRUE, TRUE, 0);


    /* Add the button */
    vbox.pack_start (button, FALSE, FALSE, 0);

    button.clicked.connect(slot(*this, &ScribbleWindow::quit));
    destroy.connect(slot(*this, &ScribbleWindow::quit));

    drawing_area.show();
    button.show();
    vbox.show();
  }

int
main (int argc, char *argv[])
{
  ScribbleWindow *window;
  Gtk::Main myapp(argc, argv);

  window = new ScribbleWindow;
  window->show();

  myapp.run();

  return 0;
}
