// Filename: somethingToEggConverter.h
// Created by:  drose (17Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef SOMETHINGTOEGGCONVERTER_H
#define SOMETHINGTOEGGCONVERTER_H

#include <pandatoolbase.h>

#include <filename.h>

class EggData;

////////////////////////////////////////////////////////////////////
// 	 Class : SomethingToEggConverter
// Description : This is a base class for a family of converter
//               classes that manage a conversion from some file type
//               to egg format.
//
//               Classes of this type can be used to implement xxx2egg
//               converter programs, as well as LoaderFileTypeXXX
//               run-time loaders.
////////////////////////////////////////////////////////////////////
class SomethingToEggConverter {
public:
  SomethingToEggConverter();
  virtual ~SomethingToEggConverter();

  enum PathConvert {
    PC_relative,
    PC_absolute,
    PC_rel_abs,
    PC_strip,
    PC_unchanged
  };
  INLINE void set_texture_path_convert(PathConvert tpc,
				       const Filename &tpc_directory = Filename());
  INLINE void set_model_path_convert(PathConvert mpc,
				     const Filename &mpc_directory = Filename());

  void set_egg_data(EggData *egg_data, bool owns_egg_data);
  INLINE void clear_egg_data();
  INLINE EggData &get_egg_data();

  virtual string get_name() const=0;
  virtual string get_extension() const=0;

  virtual bool convert_file(const Filename &filename)=0;

protected:
  PathConvert _tpc;
  Filename _tpc_directory;
  PathConvert _mpc;
  Filename _mpc_directory;

  EggData *_egg_data;
  bool _owns_egg_data;

  bool _error;
};

#include "somethingToEggConverter.I"

#endif


