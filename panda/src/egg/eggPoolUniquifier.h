// Filename: eggPoolUniquifier.h
// Created by:  drose (09Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGPOOLUNIQUIFIER_H
#define EGGPOOLUNIQUIFIER_H

#include <pandabase.h>

#include "eggNameUniquifier.h"

////////////////////////////////////////////////////////////////////
//       Class : EggPoolUniquifier
// Description : This is a specialization of EggNameUniquifier to
//               generate unique names for textures, materials, and
//               vertex pools prior to writing out an egg file.  It's
//               automatically called by EggData prior to writing out
//               an egg file.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggPoolUniquifier : public EggNameUniquifier {
public:
  EggPoolUniquifier();

  virtual string get_category(EggNode *node);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNameUniquifier::init_type();
    register_type(_type_handle, "EggPoolUniquifier",
                  EggNameUniquifier::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#endif
 
 
