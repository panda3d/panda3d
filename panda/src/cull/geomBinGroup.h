// Filename: geomBinGroup.h
// Created by:  drose (13Apr00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef GEOMBINGROUP_H
#define GEOMBINGROUP_H

#include <pandabase.h>

#include "geomBin.h"

#include <pointerTo.h>

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : GeomBinGroup
// Description : A special kind of abstract GeomBin that assigns its
//               CullState objects to any of a number of sub-bins.
//               This class is abstract because it does not define the
//               function choose_bin(), which should identity the bin
//               a given CullState should be assigned to.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomBinGroup : public GeomBin {
PUBLISHED:
  INLINE GeomBinGroup(const string &name);
  virtual ~GeomBinGroup();

  int add_sub_bin(GeomBin *sub_bin);
  INLINE int get_num_bins() const;
  INLINE GeomBin *get_bin(int n);
  PT(GeomBin) remove_bin(int n);

  virtual void set_active(bool active);

public:
  virtual int choose_bin(CullState *cs) const=0;

  virtual void clear_current_states();
  virtual void record_current_state(GraphicsStateGuardian *gsg,
                                    CullState *cs, int draw_order,
                                    CullTraverser *trav);

  virtual void draw(CullTraverser *trav);

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual void attach();
  virtual PT(GeomBin) detach();

private:
  typedef vector< PT(GeomBin) > SubBins;
  SubBins _sub_bins;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomBin::init_type();
    register_type(_type_handle, "GeomBinGroup",
                  GeomBin::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "geomBinGroup.I"

#endif
