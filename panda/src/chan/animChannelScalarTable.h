// Filename: animChannelScalarTable.h
// Created by:  drose (22Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef ANIMCHANNELSCALARTABLE_H
#define ANIMCHANNELSCALARTABLE_H

#include <pandabase.h>

#include "animChannel.h"

#include <pointerToArray.h>
#include <pta_float.h>

////////////////////////////////////////////////////////////////////
//       Class : AnimChannelScalarTable
// Description : An animation channel that issues a scalar each frame,
//               read from a table such as might have been read from
//               an egg file.  The table actually consists of nine
//               sub-tables, each representing one component of the
//               transform: scale, rotate, translate.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AnimChannelScalarTable : public AnimChannelScalar {
public:
  AnimChannelScalarTable(AnimGroup *parent, const string &name);

  virtual bool has_changed(int last_frame, int this_frame);
  virtual void get_value(int frame, float &value);

  void set_table(const CPTA_float &table);
  INLINE bool has_table() const;
  INLINE void clear_table();

  virtual void write(ostream &out, int indent_level) const;

protected:
  AnimChannelScalarTable(void);
  CPTA_float _table;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  

  static TypedWriteable *make_AnimChannelScalarTable(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AnimChannelScalar::init_type();
    register_type(_type_handle, "AnimChannelScalarTable",
		  AnimChannelScalar::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animChannelScalarTable.I"

#endif
