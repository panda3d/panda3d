// Filename: eggLine.h
// Created by:  drose (14Oct03)
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

#ifndef EGGLINE_H
#define EGGLINE_H

#include "pandabase.h"

#include "eggCompositePrimitive.h"

////////////////////////////////////////////////////////////////////
//       Class : EggLine
// Description : A line segment, or a series of connected line
//               segments, defined by a <Line> entry.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggLine : public EggCompositePrimitive {
PUBLISHED:
  INLINE EggLine(const string &name = "");
  INLINE EggLine(const EggLine &copy);
  INLINE EggLine &operator = (const EggLine &copy);
  virtual ~EggLine();

  virtual void write(ostream &out, int indent_level) const;

  INLINE bool has_thick() const;
  INLINE double get_thick() const;
  INLINE void set_thick(const double thick);
  INLINE void clear_thick();

protected:
  virtual int get_num_lead_vertices() const;

private:
  double _thick;
  bool _has_thick;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggCompositePrimitive::init_type();
    register_type(_type_handle, "EggLine",
                  EggCompositePrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "eggLine.I"

#endif
