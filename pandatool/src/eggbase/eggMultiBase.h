// Filename: eggMultiBase.h
// Created by:  drose (02Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGMULTIBASE_H
#define EGGMULTIBASE_H

#include <pandatoolbase.h>

#include <programBase.h>
#include <coordinateSystem.h>
#include <eggData.h>
#include <pointerTo.h>

class Filename;

////////////////////////////////////////////////////////////////////
// 	 Class : EggMultiBase
// Description : This specialization of ProgramBase is intended for
//               programs that read and/or write multiple egg files.
//
//               See also EggMultiFilter, for a class that also knows
//               how to read a bunch of egg files in and write them
//               out again.
////////////////////////////////////////////////////////////////////
class EggMultiBase : public ProgramBase {
public:
  EggMultiBase();

protected:
  void append_command_comment(EggData &_data);

  virtual PT(EggData) read_egg(const Filename &filename);

protected:
  bool _got_coordinate_system;
  CoordinateSystem _coordinate_system;

  typedef vector<PT(EggData)> Eggs;
  Eggs _eggs;

  bool _force_complete;
};

#endif


