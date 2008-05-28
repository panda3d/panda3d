// Filename: gtkStatsLabelStack.cxx
// Created by:  drose (16Jan06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "gtkStatsLabelStack.h"
#include "gtkStatsLabel.h"
#include "pnotify.h"

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabelStack::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkStatsLabelStack::
GtkStatsLabelStack() {
  _widget = gtk_vbox_new(FALSE, 0);
  _highlight_label = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabelStack::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkStatsLabelStack::
~GtkStatsLabelStack() {
  clear_labels();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabelStack::get_widget
//       Access: Public
//  Description: Returns the widget for this stack.
////////////////////////////////////////////////////////////////////
GtkWidget *GtkStatsLabelStack::
get_widget() const {
  return _widget;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabelStack::get_label_y
//       Access: Public
//  Description: Returns the y position of the indicated label's bottom
//               edge, relative to the indicated target widget.
////////////////////////////////////////////////////////////////////
int GtkStatsLabelStack::
get_label_y(int label_index, GtkWidget *target_widget) const {
  nassertr(label_index >= 0 && label_index < (int)_labels.size(), 0);

  // Assume all labels have the same height.
  int height = _labels[0]->get_height();
  int start_y = _widget->allocation.height - height * label_index;

  int x, y;
  gtk_widget_translate_coordinates(_widget, target_widget,
				   0, start_y, &x, &y);
  return y;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabelStack::get_label_height
//       Access: Public
//  Description: Returns the height of the indicated label.
////////////////////////////////////////////////////////////////////
int GtkStatsLabelStack::
get_label_height(int label_index) const {
  nassertr(label_index >= 0 && label_index < (int)_labels.size(), 0);
  return _labels[label_index]->get_height();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabelStack::get_label_collector_index
//       Access: Public
//  Description: Returns the collector index associated with the
//               indicated label.
////////////////////////////////////////////////////////////////////
int GtkStatsLabelStack::
get_label_collector_index(int label_index) const {
  nassertr(label_index >= 0 && label_index < (int)_labels.size(), -1);
  return _labels[label_index]->get_collector_index();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabelStack::clear_labels
//       Access: Public
//  Description: Removes the set of labels and starts a new set.
////////////////////////////////////////////////////////////////////
void GtkStatsLabelStack::
clear_labels(bool delete_widgets) {
  Labels::iterator li;
  for (li = _labels.begin(); li != _labels.end(); ++li) {
    GtkStatsLabel *label = (*li);
    if (delete_widgets) {
      gtk_container_remove(GTK_CONTAINER(_widget), label->get_widget());
    }
    delete label;
  }
  _labels.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabelStack::add_label
//       Access: Public
//  Description: Adds a new label to the top of the stack; returns the
//               new label index.
////////////////////////////////////////////////////////////////////
int GtkStatsLabelStack::
add_label(GtkStatsMonitor *monitor, GtkStatsGraph *graph,
          int thread_index, int collector_index, bool use_fullname) {
  GtkStatsLabel *label = 
    new GtkStatsLabel(monitor, graph, thread_index, collector_index, use_fullname);

  gtk_box_pack_end(GTK_BOX(_widget), label->get_widget(),
		   FALSE, FALSE, 0);

  int label_index = (int)_labels.size();
  _labels.push_back(label);

  return label_index;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabelStack::get_num_labels
//       Access: Public
//  Description: Returns the number of labels in the stack.
////////////////////////////////////////////////////////////////////
int GtkStatsLabelStack::
get_num_labels() const {
  return _labels.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabelStack::highlight_label
//       Access: Public
//  Description: Draws a highlight around the label representing the
//               indicated collector, and removes the highlight from
//               any other label.  Specify -1 to remove the highlight
//               from all labels.
////////////////////////////////////////////////////////////////////
void GtkStatsLabelStack::
highlight_label(int collector_index) {
  if (_highlight_label != collector_index) {
    _highlight_label = collector_index;
    Labels::iterator li;
    for (li = _labels.begin(); li != _labels.end(); ++li) {
      GtkStatsLabel *label = (*li);
      label->set_highlight(label->get_collector_index() == _highlight_label);
    }
  }
}
