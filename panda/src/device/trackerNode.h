// Filename: trackerNode.h
// Created by:  jason (07Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TRACKERNODE_H
#define TRACKERNODE_H

#include <pandabase.h>

#include "clientBase.h"
#include "trackerData.h"
#include "clientTrackerDevice.h"

#include <dataNode.h>
#include <matrixDataTransition.h>
#include <matrixDataAttribute.h>
#include <nodeAttributes.h>
#include <luse.h>
#include <lmatrix.h>
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
//       Class : TrackerNode
// Description : This is the primary interface to a Tracker object
//               associated with a ClientBase.  It reads the position
//               and orientation information from the tracker and
//               makes it available as a transformation on the data
//               graph.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TrackerNode : public DataNode {
PUBLISHED:
  TrackerNode(ClientBase *client, const string &device_name);
  virtual ~TrackerNode();

  INLINE bool is_valid() const;

  INLINE const LPoint3f &get_pos() const;
  INLINE const LOrientationf &get_orient() const;
  INLINE const LMatrix4f &get_transform() const;


////////////////////////////////////////////////////////////////////
// From parent class DataNode
////////////////////////////////////////////////////////////////////
public:
  virtual void
  transmit_data(NodeAttributes &data);

  NodeAttributes _attrib;
  PT(MatrixDataAttribute) _transform_attrib;

  // outputs
  static TypeHandle _transform_type;

private:
  PT(ClientTrackerDevice) _tracker;
  TrackerData _data;
  LMatrix4f _transform;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#include "trackerNode.I"

#endif
