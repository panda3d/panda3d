// Filename: dataGraphTraverser.h
// Created by:  drose (11Mar02)
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

#ifndef DATAGRAPHTRAVERSER_H
#define DATAGRAPHTRAVERSER_H

#include "pandabase.h"

#include "dataNodeTransmit.h"
#include "pvector.h"
#include "pmap.h"

class DataNode;
class PandaNode;

////////////////////////////////////////////////////////////////////
//       Class : DataGraphTraverser
// Description : This object supervises the traversal of the data
//               graph and the moving of data from one DataNode to its
//               children.  The data graph is used to manage data from
//               input devices, etc.  See the overview of the data
//               graph in dataNode.h.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DataGraphTraverser {
PUBLISHED:
  DataGraphTraverser();
  ~DataGraphTraverser();

  void traverse(PandaNode *node);
  void traverse_below(PandaNode *node, const DataNodeTransmit &output);
  void collect_leftovers();

private:
  void r_transmit(DataNode *data_node, const DataNodeTransmit inputs[]);

  typedef pvector<DataNodeTransmit> DataVector;

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
