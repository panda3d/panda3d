// Filename: gtkStatsBadVersionWindow.h
// Created by:  drose (18May01)
// 
////////////////////////////////////////////////////////////////////

#ifndef GTKSTATSBADVERSIONWINDOW_H
#define GTKSTATSBADVERSIONWINDOW_H

#include <pandatoolbase.h>

#include <basicGtkWindow.h>

class GtkStatsMonitor;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsBadVersionWindow
// Description : This window is popped up to indicated an attempted
//               connection from an invalid client.
////////////////////////////////////////////////////////////////////
class GtkStatsBadVersionWindow : public BasicGtkWindow {
public:
  GtkStatsBadVersionWindow(GtkStatsMonitor *monitor,
                           int client_major, int client_minor,
                           int server_major, int server_minor);

private:
  void close_clicked();
};


#endif

