// Filename: lwoToEgg.h
// Created by:  drose (17Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOTOEGG_H
#define LWOTOEGG_H

#include <pandatoolbase.h>

#include <somethingToEgg.h>
#include <lwoToEggConverter.h>

#include <dSearchPath.h>

////////////////////////////////////////////////////////////////////
//       Class : LwoToEgg
// Description : A program to read a Lightwave file and generate an egg
//               file.
////////////////////////////////////////////////////////////////////
class LwoToEgg : public SomethingToEgg {
public:
  LwoToEgg();

  void run();

protected:
};

#endif

