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
// 	 Class : FltToEgg
// Description : A program to read a flt file and generate an egg
//               file.
////////////////////////////////////////////////////////////////////
class FltToEgg : public SomethingToEgg {
public:
  FltToEgg();

  void run();

protected:
  static bool dispatch_path_convert_relative(const string &opt, const string &arg, void *var);
  static bool dispatch_path_convert_absolute(const string &opt, const string &arg, void *var);
  static bool dispatch_path_convert_rel_abs(const string &opt, const string &arg, void *var);
  static bool dispatch_path_convert_unchanged(const string &opt, const string &arg, void *var);


  DSearchPath _search_path;
  FltToEggConverter::PathConvert _texture_path_convert;
  FltToEggConverter::PathConvert _model_path_convert;
  Filename _make_rel_dir;
  bool _got_make_rel_dir;
};

#endif

