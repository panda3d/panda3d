// Filename: gtkStatsMainWindow.h
// Created by:  drose (14Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GTKSTATSMAINWINDOW_H
#define GTKSTATSMAINWINDOW_H

#include <pandatoolbase.h>

#include <basicGtkWindow.h>

class GtkStatsServer;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsMainWindow
// Description : This is the main window that's opened up and stays up
//               all the time when you run gtk-stats.  It just shows
//               that it's running.
////////////////////////////////////////////////////////////////////
class GtkStatsMainWindow : public BasicGtkWindow {
public:
  GtkStatsMainWindow(int port);
  virtual ~GtkStatsMainWindow();
  virtual bool destruct();

private:
  void layout_window();
  void close_clicked();
  gint idle_callback();

  int _port;
  GtkStatsServer *_server;
};


#endif

