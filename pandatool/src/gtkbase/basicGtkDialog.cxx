// Filename: basicGtkDialog.cxx
// Created by:  drose (14Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "basicGtkDialog.h"


////////////////////////////////////////////////////////////////////
//     Function: BasicGtkDialog::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
BasicGtkDialog::
BasicGtkDialog(bool free_store) : BasicGtkWindow(free_store) {
  _vbox = manage(new Gtk::VBox);
  _action_area = manage(new Gtk::HBox);

  Gtk::VBox *box0 = manage(new Gtk::VBox);
  Gtk::HSeparator *hsep = manage(new Gtk::HSeparator);

  add(*box0);
  box0->show();
  box0->pack_start(*_vbox);
  _vbox->show();

  box0->pack_start(*hsep);
  hsep->show();

  _action_area->set_border_width(10);
  box0->pack_start(*_action_area, false);
  _action_area->show();
}

////////////////////////////////////////////////////////////////////
//     Function: BasicGtkDialog::get_vbox
//       Access: Public
//  Description: Returns a pointer to the main part of the dialog
//               window.
////////////////////////////////////////////////////////////////////
Gtk::VBox *BasicGtkDialog::
get_vbox() const {
  return _vbox;
}

////////////////////////////////////////////////////////////////////
//     Function: BasicGtkDialog::get_action_area
//       Access: Public
//  Description: Returns a pointer to part of the dialog reserved for
//               action buttons.
////////////////////////////////////////////////////////////////////
Gtk::HBox *BasicGtkDialog::
get_action_area() const {
  return _action_area;
}

