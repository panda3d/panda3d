// Filename: basicGtkDialog.h
// Created by:  drose (14Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BASICGTKDIALOG_H
#define BASICGTKDIALOG_H

#include "basicGtkWindow.h"

#include <gtk--.h>


////////////////////////////////////////////////////////////////////
//       Class : BasicGtkDialog
// Description : This looks like a wrapper around Gtk::Dialog.
//               Actually, it doesn't inherit from Gtk::Dialog at all,
//               but instead (indirectly) from Gtk::Window; it just
//               duplicates the default functionality of Gtk::Dialog
//               by defining get_vbox() and a get_action_area().
////////////////////////////////////////////////////////////////////
class BasicGtkDialog : public BasicGtkWindow {
public:
  BasicGtkDialog(bool free_store = true);

  Gtk::VBox *get_vbox() const;
  Gtk::HBox *get_action_area() const;

private:
  Gtk::VBox *_vbox;
  Gtk::HBox *_action_area;
};


#endif
