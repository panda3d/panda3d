// Filename: geomBinNormal.h
// Created by:  drose (13Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GEOMBINNORMAL_H
#define GEOMBINNORMAL_H

#include <pandabase.h>

#include "geomBinGroup.h"

#include <pointerTo.h>

#include <set>

////////////////////////////////////////////////////////////////////
//       Class : GeomBinNormal
// Description : This is the most typical kind of GeomBin: it renders
//               nontransparent geometry in state-sorted order,
//               followed by transparent geometry sorted
//               back-to-front.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomBinNormal : public GeomBinGroup {
PUBLISHED:
  GeomBinNormal(const string &name);

public:
  virtual int choose_bin(CullState *cs) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomBinGroup::init_type();
    register_type(_type_handle, "GeomBinNormal",
                  GeomBinGroup::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#endif
