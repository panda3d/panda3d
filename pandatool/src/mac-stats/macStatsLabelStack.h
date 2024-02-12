/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsLabelStack.h
 * @author rdb
 * @date 2023-08-17
 */

#ifndef MACSTATSLABELSTACK_H
#define MACSTATSLABELSTACK_H

#include "pandatoolbase.h"
#include "pvector.h"
#include "macStatsLabel.h"

#include <Cocoa/Cocoa.h>

class MacStatsMonitor;
class MacStatsGraph;

/**
 * A widget that contains a stack of labels from bottom to top.
 */
class MacStatsLabelStack {
public:
  MacStatsLabelStack();
  ~MacStatsLabelStack();

  NSView *get_view() const;

  int get_label_y(int label_index, NSView *target_view) const;
  int get_label_height(int label_index) const;
  int get_label_collector_index(int label_index) const;

  void clear_labels();
  int add_label(MacStatsMonitor *monitor, MacStatsGraph *graph,
                int thread_index, int collector_index, bool use_fullname);
  int get_num_labels() const;

  void highlight_label(int collector_index);
  void update_label_color(int collector_index);

private:
  NSStackView *_stack_view;
  NSLayoutConstraint *_constraint;
  int _highlight_label;

  typedef pvector<MacStatsLabel *> Labels;
  Labels _labels;
};

#endif
