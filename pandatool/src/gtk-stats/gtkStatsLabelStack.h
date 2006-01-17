// Filename: gtkStatsLabelStack.h
// Created by:  drose (16Jan06)
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

#ifndef GTKSTATSLABELSTACK_H
#define GTKSTATSLABELSTACK_H

#include "pandatoolbase.h"
#include "pvector.h"

#include <gtk/gtk.h>

class GtkStatsLabel;
class GtkStatsMonitor;
class GtkStatsGraph;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsLabelStack
// Description : A widget that contains a stack of labels from bottom
//               to top.
////////////////////////////////////////////////////////////////////
class GtkStatsLabelStack {
public:
  GtkStatsLabelStack();
  ~GtkStatsLabelStack();

  GtkWidget *get_widget() const;

  int get_label_collector_index(int label_index) const;

  void clear_labels();
  int add_label(GtkStatsMonitor *monitor, GtkStatsGraph *graph,
                int thread_index, int collector_index, bool use_fullname);
  int get_num_labels() const;

  void highlight_label(int collector_index);

private:
  GtkWidget *_widget;
  int _highlight_label;

  typedef pvector<GtkStatsLabel *> Labels;
  Labels _labels;
};

#endif

