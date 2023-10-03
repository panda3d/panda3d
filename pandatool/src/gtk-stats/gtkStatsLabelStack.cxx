/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsLabelStack.cxx
 * @author drose
 * @date 2006-01-16
 */

#include "gtkStatsLabelStack.h"
#include "gtkStatsLabel.h"
#include "pnotify.h"

/**
 *
 */
GtkStatsLabelStack::
GtkStatsLabelStack() {
  _widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  _highlight_label = -1;
}

/**
 *
 */
GtkStatsLabelStack::
~GtkStatsLabelStack() {
  clear_labels();
}

/**
 * Returns the widget for this stack.
 */
GtkWidget *GtkStatsLabelStack::
get_widget() const {
  return _widget;
}

/**
 * Returns the y position of the indicated label's bottom edge, relative to
 * the indicated target widget.
 */
int GtkStatsLabelStack::
get_label_y(int label_index, GtkWidget *target_widget) const {
  nassertr(label_index >= 0 && label_index < (int)_labels.size(), 0);

  GtkStatsLabel *label = _labels[label_index];

  int x, y;
  gtk_widget_translate_coordinates(label->get_widget(), target_widget,
                                   0, 0, &x, &y);
  y += label->get_height();
  return y;
}

/**
 * Returns the height of the indicated label.
 */
int GtkStatsLabelStack::
get_label_height(int label_index) const {
  nassertr(label_index >= 0 && label_index < (int)_labels.size(), 0);
  return _labels[label_index]->get_height();
}

/**
 * Returns the collector index associated with the indicated label.
 */
int GtkStatsLabelStack::
get_label_collector_index(int label_index) const {
  nassertr(label_index >= 0 && label_index < (int)_labels.size(), -1);
  return _labels[label_index]->get_collector_index();
}

/**
 * Removes the set of labels and starts a new set.
 */
void GtkStatsLabelStack::
clear_labels(bool delete_widgets) {
  for (GtkStatsLabel *label : _labels) {
    if (delete_widgets) {
      gtk_container_remove(GTK_CONTAINER(_widget), label->get_widget());
    }
    delete label;
  }
  _labels.clear();
}

/**
 * Adds a new label to the top of the stack; returns the new label index.
 */
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

/**
 * Returns the number of labels in the stack.
 */
int GtkStatsLabelStack::
get_num_labels() const {
  return _labels.size();
}

/**
 * Draws a highlight around the label representing the indicated collector,
 * and removes the highlight from any other label.  Specify -1 to remove the
 * highlight from all labels.
 */
void GtkStatsLabelStack::
highlight_label(int collector_index) {
  if (_highlight_label != collector_index) {
    _highlight_label = collector_index;
    for (GtkStatsLabel *label : _labels) {
      label->set_highlight(label->get_collector_index() == _highlight_label);
    }
  }
}

/**
 * Refreshes the color of the label with the given index.
 */
void GtkStatsLabelStack::
update_label_color(int collector_index) {
  for (GtkStatsLabel *label : _labels) {
    if (label->get_collector_index() == collector_index) {
      label->update_color();
    }
  }
}
