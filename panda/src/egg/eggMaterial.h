// Filename: eggMaterial.h
// Created by:  drose (29Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGMATERIAL_H
#define EGGMATERIAL_H

#include <pandabase.h>

#include "eggNode.h"

#include <luse.h>

///////////////////////////////////////////////////////////////////
// 	 Class : EggMaterial
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggMaterial : public EggNode {
public:
  EggMaterial(const string &mref_name);
  EggMaterial(const EggMaterial &copy);

  virtual void write(ostream &out, int indent_level) const;

  INLINE void set_diff(const Colorf &diff);
  INLINE void clear_diff();
  INLINE bool has_diff() const;
  INLINE Colorf get_diff() const;

private:
  bool _has_diff;
  Colorf _diff;
};

#include "eggMaterial.I"

#endif
