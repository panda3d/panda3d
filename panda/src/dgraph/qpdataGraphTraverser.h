// Filename: qpdataGraphTraverser.h
// Created by:  drose (11Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef qpDATAGRAPHTRAVERSER_H
#define qpDATAGRAPHTRAVERSER_H

#include "pandabase.h"

#include "dataNodeTransmit.h"
#include "pvector.h"
#include "pmap.h"

class qpDataNode;
class PandaNode;

////////////////////////////////////////////////////////////////////
//       Class : DataGraphTraverser
// Description : This object supervises the traversal of the data
//               graph and the moving of data from one DataNode to its
//               children.  The data graph is used to manage data from
//               input devices, etc.  See the overview of the data
//               graph in dataNode.h.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpDataGraphTraverser {
PUBLISHED:
  qpDataGraphTraverser();
  ~qpDataGraphTraverser();

  void traverse(PandaNode *node);

private:
  void r_transmit(qpDataNode *data_node, const DataNodeTransmit inputs[]);
  void r_traverse_children(PandaNode *node, const DataNodeTransmit &output);

  typedef pvector<DataNodeTransmit> DataVector;

  class CollectedData {
  public:
    INLINE CollectedData();
    void set_data(int parent_index, const DataNodeTransmit &data);

    int _num_parents;
    DataVector _data;
  };
  typedef pmap<qpDataNode *, CollectedData> MultipassData;
  MultipassData _multipass_data;
};

#include "qpdataGraphTraverser.I"

#endif
