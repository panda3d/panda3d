/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dataGraphTraverser.h
 * @author drose
 * @date 2002-03-11
 */

#ifndef DATAGRAPHTRAVERSER_H
#define DATAGRAPHTRAVERSER_H

#include "pandabase.h"

#include "dataNodeTransmit.h"
#include "pvector.h"
#include "pmap.h"

class DataNode;
class PandaNode;

/**
 * This object supervises the traversal of the data graph and the moving of
 * data from one DataNode to its children.  The data graph is used to manage
 * data from input devices, etc.  See the overview of the data graph in
 * dataNode.h.
 */
class EXPCL_PANDA_DGRAPH DataGraphTraverser {
PUBLISHED:
  explicit DataGraphTraverser(Thread *current_thread = Thread::get_current_thread());
  ~DataGraphTraverser();

  INLINE Thread *get_current_thread() const;

  void traverse(PandaNode *node);
  void traverse_below(PandaNode *node, const DataNodeTransmit &output);
  void collect_leftovers();

private:
  void r_transmit(DataNode *data_node, const DataNodeTransmit inputs[]);

  typedef pvector<DataNodeTransmit> DataVector;

  Thread *_current_thread;

  class CollectedData {
  public:
    INLINE CollectedData();
    void set_data(int parent_index, const DataNodeTransmit &data);

    int _num_parents;
    DataVector _data;
  };
  typedef pmap<DataNode *, CollectedData> MultipassData;
  MultipassData _multipass_data;
};

#include "dataGraphTraverser.I"

#endif
