// Filename: transform2sg.h
// Created by:  drose (27Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef TRANSFORM2SG_H
#define TRANSFORM2SG_H

#include <pandabase.h>

#include <dataNode.h>

class NodeRelation;

////////////////////////////////////////////////////////////////////
//       Class : Transform2SG
// Description : input: Transform (matrix)
//
//               output: none, but applies the matrix as the transform
//               attribute for a given arc of the scene graph.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Transform2SG : public DataNode {
PUBLISHED:
  Transform2SG(const string &name = "");

  void set_arc(NodeRelation *arc);
  NodeRelation *get_arc() const;

private:
  NodeRelation *_arc;
 
////////////////////////////////////////////////////////////////////
// From parent class DataNode
////////////////////////////////////////////////////////////////////
public:

  virtual void
  transmit_data(NodeAttributes &data);

  // inputs
  static TypeHandle _transform_type;


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

