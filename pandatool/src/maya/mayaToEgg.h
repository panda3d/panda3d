// Filename: mayaToEgg.h
// Created by:  drose (15Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef MAYATOEGG_H
#define MAYATOEGG_H

#include <pandatoolbase.h>

#include "mayaFile.h"

#include <somethingToEgg.h>

////////////////////////////////////////////////////////////////////
//       Class : MayaToEgg
// Description : 
////////////////////////////////////////////////////////////////////
class MayaToEgg : public SomethingToEgg {
public:
  MayaToEgg();

  void run();

  MayaFile _maya;
};

#endif
