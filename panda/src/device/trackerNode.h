// Filename: trackerNode.h
// Created by:  jason (07Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef _TRACKER_NODE
#define _TRACKER_NODE

#include <pandabase.h>

#include <dataNode.h>
#include <nodeAttributes.h>
#include <doubleDataAttribute.h>
#include <doubleDataTransition.h>
#include <vec3DataAttribute.h>
#include <vec3DataTransition.h>
#include <vec3DataAttribute.h>
#include <vec4DataTransition.h>
#include <vec4DataAttribute.h>

#include <pointerTo.h>
#include "clientBase.h"

////////////////////////////////////////////////////////////////////
//       Class : TrackerNode
// Description : Reads the position, velocity and acceleration
//               information from one sensor on a tracker and sends it
//               down the DataGraph
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TrackerNode : public DataNode {
public:
  TrackerNode(PT(ClientBase) client, const string &tracker, 
	      int sensor);

  virtual void transmit_data(NodeAttributes &data);

public:
  NodeAttributes _tracker_attrib;

  PT(DoubleDataAttribute) _ptime;
  PT(Vec3DataAttribute) _position;
  PT(Vec4DataAttribute) _pquat;
  
  PT(DoubleDataAttribute) _vtime;
  PT(Vec3DataAttribute) _velocity;
  PT(Vec4DataAttribute) _vquat;
  PT(DoubleDataAttribute) _vquat_dt;

  PT(DoubleDataAttribute) _atime;
  PT(Vec3DataAttribute) _acceleration;
  PT(Vec4DataAttribute) _aquat;
  PT(DoubleDataAttribute) _aquat_dt;

  static TypeHandle _ptime_type;
  static TypeHandle _position_type;
  static TypeHandle _pquat_type;

  static TypeHandle _vtime_type;
  static TypeHandle _velocity_type;
  static TypeHandle _vquat_type;
  static TypeHandle _vquat_dt_type;

  static TypeHandle _atime_type;
  static TypeHandle _acceleration_type;
  static TypeHandle _aquat_type;
  static TypeHandle _aquat_dt_type;
  
protected:
  PT(ClientBase) _client;
  string _tracker;
  int _sensor;

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

#endif
