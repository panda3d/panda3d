// Filename: eggSliderData.h
// Created by:  drose (26Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGSLIDERDATA_H
#define EGGSLIDERDATA_H

#include <pandatoolbase.h>

#include "eggComponentData.h"


////////////////////////////////////////////////////////////////////
// 	 Class : EggSliderData
// Description : This corresponds to a single morph slider control.
//               It contains back pointers to all the vertices and
//               primitives that reference this slider across all
//               models, as well as all the tables in which it appears
//               in all animation files.
////////////////////////////////////////////////////////////////////
class EggSliderData : public EggComponentData {
public:
  EggSliderData(EggCharacterCollection *collection,
		EggCharacterData *char_data);

  virtual void add_back_pointer(int model_index, EggObject *egg_object);
  virtual void write(ostream &out, int indent_level = 0) const;
};

#include "eggSliderData.I"

#endif


