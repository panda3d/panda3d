/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelMatrixXfmTable.h
 * @author drose
 * @date 1999-02-20
 */

#ifndef ANIMCHANNELMATRIXXFMTABLE_H
#define ANIMCHANNELMATRIXXFMTABLE_H

#include "pandabase.h"

#include "animChannel.h"

#include "pointerToArray.h"
#include "pta_stdfloat.h"
#include "compose_matrix.h"

/**
 * An animation channel that issues a matrix each frame, read from a table
 * such as might have been read from an egg file.  The table actually consists
 * of nine sub-tables, each representing one component of the transform:
 * scale, rotate, translate.
 */
class EXPCL_PANDA_CHAN AnimChannelMatrixXfmTable : public AnimChannelMatrix {
protected:
  AnimChannelMatrixXfmTable();
  AnimChannelMatrixXfmTable(AnimGroup *parent, const AnimChannelMatrixXfmTable &copy);

PUBLISHED:
  explicit AnimChannelMatrixXfmTable(AnimGroup *parent, const std::string &name);
  virtual ~AnimChannelMatrixXfmTable();

public:
  virtual bool has_changed(int last_frame, double last_frac,
                           int this_frame, double this_frac);
  virtual void get_value(int frame, LMatrix4 &mat);

  virtual void get_value_no_scale_shear(int frame, LMatrix4 &value);
  virtual void get_scale(int frame, LVecBase3 &scale);
  virtual void get_hpr(int frame, LVecBase3 &hpr);
  virtual void get_quat(int frame, LQuaternion &quat);
  virtual void get_pos(int frame, LVecBase3 &pos);
  virtual void get_shear(int frame, LVecBase3 &shear);

PUBLISHED:
  static INLINE bool is_valid_id(char table_id);

  void set_table(char table_id, const CPTA_stdfloat &table);
  INLINE CPTA_stdfloat get_table(char table_id) const;

  void clear_all_tables();
  INLINE bool has_table(char table_id) const;
  INLINE void clear_table(char table_id);

  MAKE_MAP_PROPERTY(tables, has_table, get_table, set_table, clear_table);

public:
  virtual void write(std::ostream &out, int indent_level) const;

protected:
  virtual AnimGroup *make_copy(AnimGroup *parent) const;

  INLINE static char get_table_id(int table_index);
  static int get_table_index(char table_id);
  INLINE static PN_stdfloat get_default_value(int table_index);

  CPTA_stdfloat _tables[num_matrix_components];

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter* manager, Datagram &me);

  static TypedWritable *make_AnimChannelMatrixXfmTable(const FactoryParams &params);

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
