// Filename: fltToEgg.h
// Created by:  drose (17Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef FLTTOEGG_H
#define FLTTOEGG_H

#include <pandatoolbase.h>

#include <somethingToEgg.h>
#include <fltToEggConverter.h>

#include <dSearchPath.h>

////////////////////////////////////////////////////////////////////
//       Class : FltToEgg
// Description : A program to read a flt file and generate an egg
//               file.
////////////////////////////////////////////////////////////////////
class FltToEgg : public SomethingToEgg {
public:
  FltToEgg();

  void run();
};

#endif

