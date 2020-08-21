/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTable.h
 * @author drose
 * @date 1999-02-19
 */

#ifndef EGGTABLE_H
#define EGGTABLE_H

#include "pandabase.h"

#include "eggGroupNode.h"

/**
 * This corresponds to a <Table> or a <Bundle> entry.  As such, it doesn't
 * actually contain a table of numbers, but it may be a parent to an
 * EggSAnimData or an EggXfmAnimData, which do.  It may also be a parent to
 * another <Table> or <Bundle>, establishing a hierarchy of tables.
 */
class EXPCL_PANDA_EGG EggTable : public EggGroupNode {
PUBLISHED:
  enum TableType {
    TT_invalid,
    TT_table,
    TT_bundle,
  };

  INLINE explicit EggTable(const std::string &name = "");
  INLINE EggTable(const EggTable &copy);
  INLINE EggTable &operator = (const EggTable &copy);

  INLINE void set_table_type(TableType type);
  INLINE TableType get_table_type() const;

  bool has_transform() const;
  virtual void write(std::ostream &out, int indent_level) const;

  static TableType string_table_type(const std::string &string);

protected:
  virtual void r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
                           CoordinateSystem to_cs);

private:
  TableType _type;


public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggGroupNode::init_type();
    register_type(_type_handle, "EggTable",
                  EggGroupNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

std::ostream &operator << (std::ostream &out, EggTable::TableType t);

#include "eggTable.I"

#endif
