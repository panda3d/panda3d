// Filename: eggBase.h
// Created by:  drose (14Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGBASE_H
#define EGGBASE_H

#include <pandatoolbase.h>

#include <programBase.h>
#include <coordinateSystem.h>
#include <eggData.h>

class EggReader;
class EggWriter;

////////////////////////////////////////////////////////////////////
//       Class : EggBase
// Description : This specialization of ProgramBase is intended for
//               programs that read and/or write a single egg file.
//               (See EggMultiBase for programs that operate on
//               multiple egg files at once.)
//
//               This is just a base class; see EggReader, EggWriter,
//               or EggFilter according to your particular I/O needs.
////////////////////////////////////////////////////////////////////
class EggBase : public ProgramBase {
public:
  EggBase();

  virtual EggReader *as_reader();
  virtual EggWriter *as_writer();

protected:
  virtual bool post_command_line();
  void append_command_comment(EggData &_data);

protected:
  bool _got_coordinate_system;
  CoordinateSystem _coordinate_system;
  EggData _data;
};

#endif


