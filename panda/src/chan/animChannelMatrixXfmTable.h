// Filename: animChannelMatrixXfmTable.h
// Created by:  drose (20Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef ANIMCHANNELMATRIXXFMTABLE_H
#define ANIMCHANNELMATRIXXFMTABLE_H

#include <pandabase.h>

#include "animChannel.h"

#include <pointerToArray.h>
#include <pta_float.h>

////////////////////////////////////////////////////////////////////
//       Class : AnimChannelMatrixXfmTable
// Description : An animation channel that issues a matrix each frame,
//               read from a table such as might have been read from
//               an egg file.  The table actually consists of nine
//               sub-tables, each representing one component of the
//               transform: scale, rotate, translate.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AnimChannelMatrixXfmTable : public AnimChannelMatrix {
public:
  AnimChannelMatrixXfmTable(AnimGroup *parent, const string &name);

  virtual bool has_changed(int last_frame, int this_frame);
  virtual void get_value(int frame, LMatrix4f &mat);
  virtual void get_value_no_scale(int frame, LMatrix4f &value);
  virtual void get_scale(int frame, float scale[3]);

  static INLINE bool is_valid_id(char table_id);

  void clear_all_tables();
  void set_table(char table_id, const CPTA_float &table);
  INLINE bool has_table(char table_id) const;
  INLINE CPTA_float get_table(char table_id) const;
  INLINE void clear_table(char table_id);

  virtual void write(ostream &out, int indent_level) const;

protected:
  AnimChannelMatrixXfmTable(void);
  INLINE static char get_table_id(int table_index);
  static int get_table_index(char table_id);
  INLINE static float get_default_value(int table_index);

  enum { num_tables = 9 };

  CPTA_float _tables[num_tables];

private:
  static const char _table_ids[num_tables];
  static const float _default_values[num_tables];

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  

  static TypedWriteable *make_AnimChannelMatrixXfmTable(const FactoryParams &params);

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
    AnimChannelMatrix::init_type();
    register_type(_type_handle, "AnimChannelMatrixXfmTable",
		  AnimChannelMatrix::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animChannelMatrixXfmTable.I"

#endif
