// Filename: cerealboxNode.h
// Created by:  jason (08Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef _ADINPUT_NODE
#define _ADINPUT_NODE

#include <pandabase.h>

#include <dataNode.h>
#include <nodeAttributes.h>
#include <doubleDataAttribute.h>
#include <doubleDataTransition.h>
#include <intDataAttribute.h>
#include <intDataTransition.h>
#include <doublePtrDataAttribute.h>
#include <doublePtrDataTransition.h>

#include <pointerTo.h>
#include "clientBase.h"

////////////////////////////////////////////////////////////////////
//       Class : ADInputNode
// Description : Reads the analog, buttons and dials from a adinput
//               and sends it down the DataGraph
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ADInputNode : public DataNode {
public:
  ADInputNode(PT(ClientBase) client, const string &adinput);

  virtual void transmit_data(NodeAttributes &data);

public:
  NodeAttributes _adinput_attrib;

  PT(DoubleDataAttribute) _btime;
  PT(IntDataAttribute) _button_id;
  PT(IntDataAttribute) _state;
  
  PT(DoubleDataAttribute) _dtime;
  PT(IntDataAttribute) _dial_id;
  PT(DoubleDataAttribute) _change;
  
  PT(DoubleDataAttribute) _atime;
  PT(DoublePtrDataAttribute) _channels;

  static TypeHandle _btime_type;
  static TypeHandle _button_id_type;
  static TypeHandle _state_type;

  static TypeHandle _dtime_type;
  static TypeHandle _dial_id_type;
  static TypeHandle _change_type;

  static TypeHandle _atime_type;
  static TypeHandle _channels_type;
  
protected:
  PT(ClientBase) _client;
  string _adinput;

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
