// Filename: basicGtkWindow.h
// Created by:  drose (14Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BASICGTKWINDOW_H
#define BASICGTKWINDOW_H

#include <gtk--.h>


////////////////////////////////////////////////////////////////////
// 	 Class : BasicGtkWindow
// Description : This is just a handy wrapper around Gtk::Window that
//               provides some convenient setup functions.
////////////////////////////////////////////////////////////////////
class BasicGtkWindow : public Gtk::Window {
public:
  BasicGtkWindow(bool free_store = true);
  virtual ~BasicGtkWindow();
  void setup();
  virtual bool destruct();

protected:
  void delete_self();
  static gint static_delete(BasicGtkWindow *window);

private:
  void window_destroyed();
  gint idle_event();

  enum State {
    S_virgin,
    S_setup,
    S_ready,
    S_gone,
  };

  bool _destroyed;
  bool _free_store;
  State _state;
  SigC::Connection _destroy_connection;
};


#endif

