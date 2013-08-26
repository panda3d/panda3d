// Filename: trackerNode.h
// Created by:  drose (12Mar02)
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

#ifndef TRACKERNODE_H
#define TRACKERNODE_H

#include "pandabase.h"

#include "clientBase.h"
#include "trackerData.h"
#include "clientTrackerDevice.h"
#include "dataNode.h"
#include "luse.h"
#include "linmath_events.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : TrackerNode
// Description : This is the primary interface to a Tracker object
//               associated with a ClientBase.  It reads the position
//               and orientation information from the tracker and
//               makes it available as a transformation on the data
//               graph.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DEVICE TrackerNode : public DataNode {
PUBLISHED:
  TrackerNode(ClientBase *client, const string &device_name);
  TrackerNode(ClientTrackerDevice *device);
  virtual ~TrackerNode();

  INLINE bool is_valid() const;

  INLINE const LPoint3 &get_pos() const;
  INLINE const LOrientation &get_orient() const;
  INLINE const LMatrix4 &get_transform() const;
  INLINE double get_time() const;
  INLINE bool has_time() const;

  INLINE void set_tracker_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_tracker_coordinate_system() const;

  INLINE void set_graph_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_graph_coordinate_system() const;

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  // outputs
  int _transform_output;

  CPT(TransformState) _transform;

private:
  PT(ClientTrackerDevice) _tracker;
  TrackerData _data;
  LMatrix4 _mat;
  CoordinateSystem _tracker_cs, _graph_cs;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "TrackerNode",
                  DataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "trackerNode.I"

#endif
