/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsLabelStack.mm
 * @author rdb
 * @date 2023-08-17
 */

#include "macStatsLabelStack.h"
#include "macStatsLabel.h"
#include "macStatsMonitor.h"

static const NSEdgeInsets insets = {8, 8, 8, 8};

/**
 *
 */
MacStatsLabelStack::
MacStatsLabelStack() {
  _stack_view = [[NSStackView alloc] init];
  _stack_view.edgeInsets = insets;
  _stack_view.orientation = NSUserInterfaceLayoutOrientationVertical;
  _stack_view.alignment = NSLayoutAttributeRight;//NSLayoutAttributeCenterX;
  _stack_view.distribution = NSStackViewDistributionGravityAreas;
  _stack_view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
  _stack_view.translatesAutoresizingMaskIntoConstraints = NO;
  _stack_view.spacing = 0;
  _highlight_label = -1;

  // May never be wider than the widest label, or 68, whichever is larger
  //FIXME
  _constraint = [_stack_view.widthAnchor constraintLessThanOrEqualToConstant:68];
  _constraint.active = NO;
  [_constraint retain];
}

/**
 *
 */
MacStatsLabelStack::
~MacStatsLabelStack() {
  clear_labels();
  [_constraint release];
  [_stack_view release];
}

/**
 * Returns the view for this stack.
 */
NSView *MacStatsLabelStack::
get_view() const {
  return _stack_view;
}

/**
 * Returns the y position of the indicated label's bottom edge, relative to
 * the indicated target widget.
 */
int MacStatsLabelStack::
get_label_y(int label_index, NSView *target_view) const {
  nassertr(label_index >= 0 && label_index < (int)_labels.size(), 0);

  MacStatsLabel *label = _labels[label_index];
  NSPoint pos = [target_view convertPoint:NSMakePoint(0, label.frame.size.height) fromView:label];
  return pos.y;
}

/**
 * Returns the height of the indicated label.
 */
int MacStatsLabelStack::
get_label_height(int label_index) const {
  nassertr(label_index >= 0 && label_index < (int)_labels.size(), 0);
  return _labels[label_index].frame.size.height;
}

/**
 * Returns the collector index associated with the indicated label.
 */
int MacStatsLabelStack::
get_label_collector_index(int label_index) const {
  nassertr(label_index >= 0 && label_index < (int)_labels.size(), -1);
  return _labels[label_index].collectorIndex;
}

/**
 * Removes the set of labels and starts a new set.
 */
void MacStatsLabelStack::
clear_labels() {
  for (MacStatsLabel *label : _labels) {
    [_stack_view removeView:label];
    [label release];
  }
  _labels.clear();

  _constraint.constant = 68;

  NSRect frame = _stack_view.frame;
  frame.size.width = 0;
  _stack_view.frame = frame;
}

/**
 * Adds a new label to the top of the stack; returns the new label index.
 */
int MacStatsLabelStack::
add_label(MacStatsMonitor *monitor, MacStatsGraph *graph,
          int thread_index, int collector_index, bool use_fullname) {
  const PStatClientData *client_data = monitor->get_client_data();
  std::string text;
  if (use_fullname) {
    text = client_data->get_collector_fullname(collector_index);
  } else {
    text = client_data->get_collector_name(collector_index);
  }

  MacStatsLabel *label = [MacStatsLabel alloc];
  [label initWithText:[NSString stringWithUTF8String:text.c_str()]
                graph:graph
          threadIndex:thread_index
       collectorIndex:collector_index];

  [_stack_view insertView:label atIndex:0 inGravity:NSStackViewGravityBottom];
  [label.leadingAnchor constraintEqualToAnchor:_stack_view.leadingAnchor constant:8].active = YES;
  [label.trailingAnchor constraintLessThanOrEqualToAnchor:_stack_view.trailingAnchor constant:-8].active = YES;

  _constraint.constant = std::max(_constraint.constant, label.intrinsicContentSize.width);

  int label_index = (int)_labels.size();
  _labels.push_back(label);

  return label_index;
}

/**
 * Returns the number of labels in the stack.
 */
int MacStatsLabelStack::
get_num_labels() const {
  return _labels.size();
}

/**
 * Draws a highlight around the label representing the indicated collector,
 * and removes the highlight from any other label.  Specify -1 to remove the
 * highlight from all labels.
 */
void MacStatsLabelStack::
highlight_label(int collector_index) {
  if (_highlight_label != collector_index) {
    _highlight_label = collector_index;
    for (MacStatsLabel *label : _labels) {
      label.highlight = (label.collectorIndex == _highlight_label);
    }
  }
}

/**
 * Refreshes the color of the label with the given index.
 */
void MacStatsLabelStack::
update_label_color(int collector_index) {
  for (MacStatsLabel *label : _labels) {
    if (label.collectorIndex == collector_index) {
      [label updateColor];
    }
  }
}
