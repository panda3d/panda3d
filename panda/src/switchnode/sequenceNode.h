// Filename: sequenceNode.h
// Created by:  jason (18Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SEQUENCENODE_H
#define SEQUENCENODE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <namedNode.h>
#include <timedCycle.h>
#include <pointerToArray.h>

////////////////////////////////////////////////////////////////////
//       Class : SequenceNode
// Description : A node that implements the ability to switch out 
//               what node is rendered based on time
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA SequenceNode : public NamedNode, public TimedCycle
{
PUBLISHED:
  SequenceNode(const string &initial_name = "");
  SequenceNode(float switch_time, const string &initial_name = "");

  void set_switch_time(float switch_time);

public:
  virtual bool sub_render(const AllAttributesWrapper &attrib,
			  AllTransitionsWrapper &trans,
			  GraphicsStateGuardianBase *gsgbase);
  virtual bool has_sub_render() const;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);  

  static TypedWriteable *make_SequenceNode(const FactoryParams &params);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type( void ) {
      return _type_handle;
  }
  static void init_type( void ) {
    NamedNode::init_type();
    register_type( _type_handle, "SequenceNode",
                NamedNode::get_class_type() );
  }
  virtual TypeHandle get_type( void ) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle		_type_handle;
};

#endif
