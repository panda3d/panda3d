// Filename: animChannelMatrixXfmTable.h
// Created by:  drose (20Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef ANIMCHANNELMATRIXXFMTABLE_H
#define ANIMCHANNELMATRIXXFMTABLE_H

#include "pandabase.h"

#include "animChannel.h"

#include "pointerToArray.h"
#include "pta_float.h"
#include "compose_matrix.h"

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
protected:
  AnimChannelMatrixXfmTable();

public:
  ~AnimChannelMatrixXfmTable();
  

  virtual bool has_changed(int last_frame, int this_frame);
  virtual void get_value(int frame, LMatrix4f &mat);
  virtual void get_value_no_scale(int frame, LMatrix4f &value);
  virtual void get_scale(int frame, float scale[3]);

  static INLINE bool is_valid_id(char table_id);

  void set_table(char table_id, const CPTA_float &table);
  INLINE CPTA_float get_table(char table_id) const;

PUBLISHED:
  void clear_all_tables();
  INLINE bool has_table(char table_id) const;
  INLINE void clear_table(char table_id);

public:
  virtual void write(ostream &out, int indent_level) const;

protected:
  INLINE static char get_table_id(int table_index);
  static int get_table_index(char table_id);
  INLINE static float get_default_value(int table_index);

  CPTA_float _tables[num_matrix_components];

public:
  static void register_with_read_factory(void);
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
