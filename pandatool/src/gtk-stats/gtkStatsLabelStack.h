/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsLabelStack.h
 * @author drose
 * @date 2006-01-16
 */

#ifndef GTKSTATSLABELSTACK_H
#define GTKSTATSLABELSTACK_H

#include "pandatoolbase.h"
#include "pvector.h"

#include <gtk/gtk.h>

class GtkStatsLabel;
class GtkStatsMonitor;
class GtkStatsGraph;

/**
 * A widget that contains a stack of labels from bottom to top.
 */
class GtkStatsLabelStack {
public:
  GtkStatsLabelStack();
  ~GtkStatsLabelStack();

  GtkWidget *get_widget() const;

  int get_label_y(int label_index, GtkWidget *target_widget) const;
  int get_label_height(int label_index) const;
  int get_label_collector_index(int label_index) const;

  void clear_labels(bool delete_widgets = true);
  int add_label(GtkStatsMonitor *monitor, GtkStatsGraph *graph,
                int thread_index, int collector_index, bool use_fullname);
  int get_num_labels() const;

  void highlight_label(int collector_index);
  void update_label_color(int collector_index);

private:
  GtkWidget *_widget;
  int _highlight_label;

  typedef pvector<GtkStatsLabel *> Labels;
  Labels _labels;
};

#endif
